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

	// getters
	std::string get_hostname() { return m_hostname; }
	int get_priority() { return m_priority; }
	
	// setters
	void set_hostname();	// gets physical machine's hostname from linux
	void set_priority();		// loads priority from config file

	// doers
	bool start_ntpd();		// attempts to start ntpd service and returns success/fail as true/false

private:
	void _error_dump(std::string failed_command, std::vector<std::string> terminal_feedback, std::vector<std::string> error_list);

	std::string m_hostname;	// actual hostname of the machine
	int m_priority;			// set by config file
	bool m_ntpd_is_up;		// if TRUE, ntpd has successfully spun up and is ready to accept clients
	command cmd;			// object for sending BASH commands to a terminal and getting the terminal's feedback
};

///////////////////////////////////////////////// Public Methods /////////////////////////////////////////////////

ntp_node::ntp_node() {
	m_priority = -1;
	m_hostname = "";
}

void ntp_node::set_hostname() {
	std::vector<std::string> terminal_feedback, error_list;
	std::string hostname = "";
	cmd.exec("hostname", terminal_feedback, error_list);
	if (terminal_feedback.size() == 1) {
		hostname = terminal_feedback[0];
		for (int i = 0; i < hostname.size(); i++) {
			if (hostname[i] == '\n')
				hostname.erase(i, 1);
		}
		std::cout << "Hostname: " << hostname << std::endl << std::endl;
		m_hostname = hostname;
	}
	else _error_dump("hostname", terminal_feedback, error_list);
}

void ntp_node::set_priority() {
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
		cout << "Loading priority config file..." << endl;
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
			_error_dump("convert [std::string s_priority] to [int i_priority]", terminal_feedback, error_list);
		}
	}
	else cout << m_hostname << " priority not found! Check config file!" << endl << endl;
	m_priority = i_priority;
	}
}

bool ntp_node::start_ntpd() {
	// This entire function is just a placeholder for ntpd succeeding or failing.
	int ntpd_rand = -1;	// giving it an invalid value by default.
	srand (time(NULL));
	ntpd_rand = rand() % 2;	// 1 in 6 chance of ntpd working, one for each VM.
	if (ntpd_rand == 1) {
		m_ntpd_is_up = true; }
	else {
		m_ntpd_is_up = false; }
	return m_ntpd_is_up;
}

////////////////////////////////////////////////// Private Methods //////////////////////////////////////////////////

void ntp_node::_error_dump(std::string failed_command, std::vector<std::string> terminal_feedback, std::vector<std::string> error_list) {
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

#endif