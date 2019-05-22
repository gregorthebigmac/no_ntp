#include "command/command.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

void error_dump(string failed_command, vector<string> error_dump);
string purge_newline(string hostname);
int get_priority(vector<string> terminal_feedback, string hostname);

int main() {
	command cmd;
	vector<string>terminal_feedback, error_list;
	string hostname;		// obtained with BASH command "hostname"
	int my_priority = -1;	// giving it an invalid value by default to ensure no two machines accidentally wind up with identical *valid* values
	
	cmd.exec("hostname", terminal_feedback, error_list);
	if (terminal_feedback.size() == 1) {
		hostname = purge_newline(terminal_feedback[0]);
		cout << "Hostname: " << hostname << endl << endl;
	}
	else error_dump("hostname", terminal_feedback);
	
	string cmd_get_priority = "cat config/priority.cfg";
	terminal_feedback.clear();
	cmd.exec(cmd_get_priority.c_str(), terminal_feedback, error_list);
	if (error_list.size() < 1) {
		my_priority = get_priority(terminal_feedback, hostname); }
	else error_dump(cmd_get_priority, error_list);

	return 0;
}

void error_dump(string failed_command, vector<string> error_dump) {
	cout << "Errors encountered when trying to execute command:" << endl;
	cout << failed_command << endl << endl;
	cout << "Error dump:" << endl;
	for (int i = 0; i < error_dump.size(); i++) {
		cout << error_dump[i];
	}
	cout << endl << endl;
}

string purge_newline(string hostname) {
	string temp = hostname;
	for (int i = 0; i < temp.size(); i++) {
		if (temp[i] == '\n')
			temp.erase(i, 1);
	}
	return temp;
}

int get_priority(vector<string> terminal_feedback, string hostname) {
	cout << "Loading priority config file..." << endl;
	string str_priority = "";
	for (int i = 0; i < terminal_feedback.size(); i++) {
		if (terminal_feedback[i][0] != '#') {
			string temp = terminal_feedback[i];
			std::size_t found_host = temp.find(hostname);
			if (found_host != std::string::npos) {
				std::size_t found_priority = temp.find("=");
				if (found_priority != std::string::npos) {
					str_priority = temp.substr(0,found_priority);
					break;
				}
			}
		}
	}
	cout << "[done]" << endl << endl;
	cout << "Found " << hostname << " priority: " << str_priority << endl << endl;
	return 0;
}