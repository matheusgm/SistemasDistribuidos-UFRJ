#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

/**
 * @brief Receives by command line pid and message to be sent
 * 
 * @param argc = 3
 * @param argv = [command with which the program is invoked, pid from receiver.cpp, int from message]
 * @return int 
 */
int main(int argc, char const *argv[]) {
    int pid = atoi(argv[1]);
    int signal = atoi(argv[2]);

    cout << " Signal " << argv[2] <<" sent to pid = " << argv[1] << ")"<< endl;
    int res = kill(pid, signal);

    // error generates -1
    if (res == -1)
		cout << "The process does not exist\n";
    
    return 0;
}
