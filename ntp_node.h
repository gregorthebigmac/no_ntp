#ifndef NTP_NODE
#define NTP_NODE

#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include "command/command.hpp"

class ntp_node {
public:
	ntp_node();		// ctor
	~ntp_node(){}	// dtor

private:
	std::string get_hostname_from_nix();
	int get_priority();
	void error_dump(std::string failed_command, std::vector<std::string> terminal_feedback, std::vector<std::string> error_list);
	bool start_ntpd();

	std::string m_hostname;	// actual hostname of the machine
	int m_priority;			// set by config file
	bool m_ntpd_is_up;		// if TRUE, ntpd has successfully spun up and is ready to accept clients
	command cmd;			// object for sending BASH commands to a terminal and getting the terminal's feedback
};

///////////////////////////// Public Methods /////////////////////////////
ntp_node::ntp_node() {
	using std::cout;
	using std::endl;

	// upon construction of the object, we get to work getting the hostname and trying to spin up the ntpd service
	m_hostname = get_hostname_from_nix();
	
	if (m_hostname == "error") {
		cout << "ERROR: Failed to get hostname from host Operating System! Quitting..." << endl;
		exit(EXIT_FAILURE);
	}
	
	m_priority = get_priority();
	
	if (m_priority < 0) {
		cout << "ERROR! Invalid Priority: [" << m_priority << "]" << endl;
		cout << "Quitting..." << endl;
		exit(EXIT_FAILURE);
	}
	else cout << "Priority is: " << m_priority << endl << endl;

	m_ntpd_is_up = start_ntpd();
	if (m_ntpd_is_up) {
		cout << "ntpd is running!" << endl;
	}
	else cout << "ntpd failed to start!" << endl;
}

////////////////////////////// Private Methods //////////////////////////////
std::string ntp_node::get_hostname_from_nix() {
	std::vector<std::string> terminal_feedback, error_list;
	std::string hostname = "";
	cmd.exec("hostname", terminal_feedback, error_list);
	if (terminal_feedback.size() == 1) {
		for (int i = 0; i < hostname.size(); i++) {
			if (hostname[i] == '\n')
				hostname.erase(i, 1);
		}
		std::cout << "Hostname: " << hostname << std::endl << std::endl;
		return hostname;
	}
	else error_dump("hostname", terminal_feedback, error_list);
	return "error";
}

int ntp_node::get_priority() {
	using std::cout;
	using std::endl;
	using std::string;
	using std::vector;

	string s_priority = "";
	int i_priority = -1;
	string cmd_get_priority = "cat config/priority.cfg";
	vector<string> terminal_feedback, error_list;
	cmd.exec(cmd_get_priority.c_str(), terminal_feedback, error_list);
	if (error_list.size() < 1) {
		cout << "Loading priority config file...";
	for (int i = 0; i < terminal_feedback.size(); i++) {
		if (terminal_feedback[i][0] != '#') {
			string temp = terminal_feedback[i];
			std::size_t found_host = temp.find(m_hostname);
			if (found_host != std::string::npos) {
				std::size_t found_priority = temp.find("=");
				if (found_priority != std::string::npos) {
					s_priority = temp.substr(0,found_priority);
					break;
				}
			}
		}
	}
	cout << "[done]" << endl << endl;
	if (s_priority != "") {
		try { i_priority = atoi(s_priority.c_str()); }
		catch (const std::invalid_argument& ia) {
			std::cerr << "Invalid Argument: " << ia.what() << endl;
			i_priority = -1;
			error_dump("convert [std::string s_priority] to [int i_priority]", terminal_feedback, error_list);
		}
	}
	else cout << m_hostname << " priority not found! Check config file!" << endl << endl;
	return i_priority;
	}
}

void ntp_node::error_dump(std::string failed_command, std::vector<std::string> terminal_feedback, std::vector<std::string> error_list) {
	using std::cout;
	using std::endl;
	cout << "Errors encountered when trying to execute command:" << endl;
	cout << failed_command << endl << endl;
	cout << "Full terminal feedback:" << endl << endl;
	for (int i = 0; i < terminal_feedback.size(); i++) {
		cout << terminal_feedback[i];
	}
	cout << endl << "Error dump:" << endl << endl;
	for (int i = 0; i < error_list.size(); i++) {
		cout << error_list[i];
	}
	cout << endl << "[DONE]" << endl << endl;
}

bool ntp_node::start_ntpd() {
	// This entire function is just a placeholder for ntpd succeeding or failing.
	int ntpd_rand = -1;	// giving it an invalid value by default.
	srand (time(NULL));
	ntpd_rand = rand() % 6;	// 1 in 6 chance of ntpd working, one for each VM.
	if (ntpd_rand == 1) {
		return true; }
	else {
		return false; }
}

#endif