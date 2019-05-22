#include "command/command.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

void error_dump(string failed_command, vector<string> error_dump);
string purge_newline(string hostname);
int get_priority(vector<string> terminal_feedback);

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
		my_priority = get_priority(terminal_feedback); }
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

int get_priority(vector<string> terminal_feedback) {
	cout << "Loading priority config file..." << endl;
	for (int i = 0; i < terminal_feedback.size(); i++) {
		cout << terminal_feedback[i];
	}
	cout << "[done]" << endl << endl;
	return 0;
}