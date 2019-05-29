#include "ntp_node.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main() {
	ntp_node node;
	node.set_hostname();
	node.set_priority();
	cout << "My priority # is: " << node.get_priority() << endl;
	if (node.start_ntpd()) {
		cout << "ntpd is running!" << endl;
	}
	else cout << "ntpd failed!" << endl;
	return 0;
}
