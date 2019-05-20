# no_ntp

The idea behind this is that a bunch of machines are on a LAN, but may not have WAN access, and the machines have no RTC (Real-Time Clock) *or* CMOS battery, and are therefore unable to keep time once they lose power, and reset to factory default date/time, but the machines on the LAN need to be able to talk to one another, and will be unable to SSH into each other because of date/time differences, which invalidates certs, and it's a big headache.

## The Solution
My solution to this problem is to have what I'm calling a "pseudo-p2p NTP topology." The idea is no single machine is **the** NTP server, but rather the first machine to come up and have fully functional networking capabilities will ping all other known hosts on the LAN and ask each one "Are you the NTP server?" If a machine responds with "Yes, I am the NTP server! Here's the current date/time/zone!" Then the machine goes into "client mode" and gets its time from the machine which reported itself as NTP server. If no machine responds "Yes, I am the NTP server," then the first machine says, "Fine, *I'll* be the NTP server!" and proceeds to run the NTP service and accept NTP requests from other known hosts on the LAN. Each machine will run through this routine upon startup (via cronjobs), which can lead to other conflicts, and that will be addressed next.

### Handling Conflicts

If all the machines on the LAN are turned on at the same time and have (even approximately) the same boot time, there is a very high chance of seeing the following scenario play out:

Let's say we have 4 machines which are all identical, and therefore have the same boot time and are all started up at the same time. These machines will be labeled "A" through "D" for this scenario:

A boots up and asks B: "Are you the NTP server?"
B has just booted up, and has only begun this process, as well, so of course not. B replies "No, I'm not the NTP server." A then asks the same of C, who *also* just booted up, and therefore replies "Nope. I'm not the NTP server, either." A finally asks D if it's the NTP server, and D (in the same boat as the others) replies "No." If A, B, C, and D are all doing this at the same time, following our rules from above, we would wind up with **ALL four** machines acting as NTP server, and we're back to square one.

To combat this, each machine will be in one of three "states" at any given time, and will respond with its state when prompted by other machines on the LAN. The three states will be:

0. None
1. Client
2. Server

In addition to this, each machine will be given a numeric priority, which it will pull from a plaintext config file, which will start at 0 (highest priority) and the upper limit will only be constricted by the type of integer used to track it. This will allow us to intelligently determine who *should* be the NTP server, while still allowing for others to fill the role, should the machines above it fail. So let's have a new scenario, where machines "A" and "C" fail to initiate the ntpd service, for whatever reason, but "B" and "D" both succeed. The machines priority is defined as follows:
A = 0
B = 1
C = 2
D = 3

A: Boot complete. I'm not currently running ntpd, and I'm not currently connected to an NTP server, which means my "state" is 0 (None), and I know my priority is 0. I will first check for other running NTP servers before attempting to run my own NTP server. Each of these will be broken down by "frames" (sorry, I'm a game dev, this terminology makes sense to me, deal with it) where it's assumed that each of the actions happening in one frame are happening (approximately) at the same time.

Frame 1:
A: Hey, B. Are you running ntpd?
B: Hey, C. Are you running ntpd?
C: Hey, D. Are you running ntpd?
D: Hey, A. Are you running ntpd?

Frame 2:
B: No, A. My state is 0, and my priority is 1.
C: No, B. My state is 0, and my priority is 2.
D: No, C. My state is 0, and my priority is 3.
A: No, D. My state is 0, and my priority is 0.

Frame 3:
A: Okay. Thanks, B.
B: Okay. Thanks, C.
C: Okay. Thanks, D.
D: Okay. Thanks, A.

Frame 4:
A: Hey, C. Are you running ntpd?
B: Hey, D. Are you running ntpd?
C: Hey, A. Are you running ntpd?
D: Hey, B. Are you running ntpd?

Frame 5:
C: No, A. My state is 0, and my priority is 2.
D: No, B. My state is 0, and my priority is 3.
A: No, C. My state is 0, and my priority is 0.
B: No, D. My state is 0, and my priority is 1.

Frame 6:
A: Okay. Thanks, C.
B: Okay. Thanks, D.
C: Okay. Thanks, A.
D: Okay. Thanks, B.

Frame 7:
A: Hey, D. Are you running ntpd?
B: Hey, A. Are you running ntpd?
C: Hey, B. Are you running ntpd?
D: Hey, C. Are you running ntpd?

Frame 8:
D: No, A. My state is 0, and my priority is 3.
A: No, B. My state is 0, and my priority is 0.
B: No, C. My state is 0, and my priority is 1.
C: No, D. My state is 0, and my priority is 2.

Frame 9:
A: Okay. Thanks, D.
B: Okay. Thanks, A.
C: Okay. Thanks, B.
D: Okay. Thanks, C.

Frame 10:
A: Since all machines are at state 0 (including myself), and my priority is 0, I'll start ntpd...
B: Since all machines are at state 0 (including myself), and my priority is 1, I'll check with A once per second for 5 seconds. If A responds with a state of 2 before then, I'll configure myself as an NTP client to A, and cease checking with A. If A responds with a 1 (client) I will ask it for who its server is, and try that host for my NTP server. If, after 5 attempts, A continues to respond with 0 (or no response at all), I will attempt to begin ntpd, since I'm next in the order of succession.
C: Since all machines are at state 0 (including myself), and my priority is 2, I'll check with B once per second for 5 seconds. If B responds with a state of 2 before then, I'll configure myself as an NTP client to B, and cease checking with B. If B responds with a 1 (client), I will ask it for who its server is, and try that host for my NTP server. If, after 5 attempts, B continues to respond with 0 (or no response at all), I will attempt to begin ntpd, since I'm next in the order of succession.
D: Since all machines are at state 0 (including myself), and my priority is 3, I'll check with C once per second for 5 seconds. If C responds with a state of 2 before then, I'll configure myself as an NTP client to C, and cease checking with C. If C responds with a 1 (client), I will ask it for who its server is, and try that host for my NTP server. If, after 5 attempts, C continues to respond with 0 (or no response at all), I will attempt to begin ntpd, since I'm next in the order of succession.

Frame 11:
A: I failed to start ntpd, and my state is still 0. I'll continue re-trying to get ntpd up and running 2 more times before I quit and look to see if anyone else has taken over as NTP server.
B: A is still reporting 0. 3 attempts remaining before I start my own NTP server.
C: B is still reporting 0. 3 attempts remaining before I start my own NTP server.
D: C is still reporting 0. 3 attempts remaining before I start my own NTP server.

Frame 12:
A: I failed to start ntpd, and my state is still 0. I'll continue re-trying to get ntpd up and running 1 more time before I quit and look to see if anyone else has taken over as NTP server.
B: A is still reporting 0. 2 attempts remaining before I start my own NTP server.
C: B is still reporting 0. 2 attempts remaining before I start my own NTP server.
D: C is still reporting 0. 2 attempts remaining before I start my own NTP server.

Frame 13:
A: I failed to start ntpd, and my state is still 0. I've failed 3 times in a row, so I'm aborting starting an NTP server, and will check to see if B has been successful.
B: A is still reporting 0. 1 attempts remaining before I start my own NTP server.
C: B is still reporting 0. 1 attempts remaining before I start my own NTP server.
D: C is still reporting 0. 1 attempts remaining before I start my own NTP server.

Frame 14:
A: My state is still 0. Hey, B. Are you the server?
B: A is still reporting 0. This is the last time I check before I start my own NTP server.
C: B is still reporting 0. This is the last time I check before I start my own NTP server.
D: C is still reporting 0. This is the last time I check before I start my own NTP server.

Frame 15:
B: A is still reporting 0. Time to start my own NTP server...
C: B is still reporting 0. Time to start my own NTP server...
D: C is still reporting 0. Time to start my own NTP server...
A: My state is still 0. Hey, B. Are you the server?

Frame 16:
B: ntpd is up and running. My state is now 2, if anyone asks. Next frame, I'll poll the others for their states to see if anyone higher priority than me was successful in getting NTP up and running.
C: ntpd failed to initiate. 2 more tries before I abort and look to see if anyone else managed to get NTP up and running.
D: ntpd is up and running. My state is now 2, if anyone asks. Next frame, I'll poll the others for their states to see if anyone higher priority than me was successful in getting NTP up and running.
A: My state is still 0. Hey, B. Are you the server?

Frame 17:
B: Yes, A. I'm the server. My state is 2. Here's the date/time/zone.
C: ntpd failed to initiate. 1 more try before I abort and look to see if anyone else managed to get NTP up and running.
D: ntpd is up and running, and no one asked me to sync, so I'll go up one on the priority list and see if C got ntpd up and running. Hey C, are you the server?
A: [Waiting for response from B].

Frame 18:
A: Okay, B. You're the server, and my state is now 1. Thanks for the date/time/zone info!
B: [Waiting for response from A]
C: ntpd failed to initiate again. No, D. I'm not the server, my state is 0.
D: [Waiting for response from C]

Frame 19:
A: I'm good. Not doing anything this frame.
B: Okay, A is my client, and I'm the server. No one is higher up the priority list than A, so I'll chill and wait for the requests to cascade up to me.
C: ntpd failed to initiate, and I just responded to D with my failure. Time to go up one on the list and see if B was successful in getting a server up and running. Hey, B. Are you the server?
D: C is at state 0, so I'm going to check one level higher. Hey, B. Are you the server?

Frame 20:
A: I'm good. Not doing anything this frame.
B: Yes, C. I am the server. Here is date/time/zone.
C: [Waiting for response from B]
D: [Waiting for response from B]

Frame 21:
A: I'm good. Not doing anything this frame.
B: Yes, D. I am the server. Here is date/time/zone.
C: Okay, B. You're the server, and I'm your client. My state is now 1.
D: Oh, you're a server, too? What's your priority number? Mine is 3.

Frame 22:
A: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
B: Uh-oh. D is a server, too. His priority is 3. Hey, D. My priority is 1. My priority is higher because I'm closer to 0.
D: [Waiting for response from B]

Frame 23:
A: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
B: [Waiting for response from D]
D: Okay, B's priority is 1. My priority is 3. My priority is lower because B's priority is closer to 0.

Frame 24:
A: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
B: My evaluation of the difference in priority agrees with D's evaluation of the difference in priority. I will continue to be the NTP server, and D will switch to client.
D: My evaluation of the difference in priority agrees with B's evaluation of the difference in priority. I will stop my NTP service, and switch to client.

Frame 25:
A: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
D: Hey, B. My state is 1, and yours is 2. What is date/time/zone?
B: [Receiving message from D]

Frame 26:
A: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
B: Got your message, D. Here's date/time/zone.
D: [Waiting for response from B]

Frame 27:
A: I'm good. Not doing anything this frame.
B: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
D: Got your date/time/zone, B. Thanks!

Frame 28:
A: I'm good. Not doing anything this frame.
B: I'm good. Not doing anything this frame.
C: I'm good. Not doing anything this frame.
D: I'm good. Not doing anything this frame.
