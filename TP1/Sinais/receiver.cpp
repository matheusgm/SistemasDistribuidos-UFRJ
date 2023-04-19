#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include<unistd.h>  

using namespace std;

/**
 * @brief Defines how incoming signals are handled per process
 * 
 * @param signal incoming int from the message sent by sender.cpp
 */
void signal_handler(int signal) {
    if (signal == 2) { // SIGINT
        cout << "Signal 2 (SIGINT) was successfully received." << endl;
    } else if (signal == 10) { // SIGUSR1
        cout << "Signal 10 (SIGUSR1) has been successfully received." << endl;
    } else if (signal == 12) { // SIGUSR2
        cout << "Signal 12 (SIGUSR2) has been successfully received." << endl;
        cout << "Finishing the process." << endl;
        raise(SIGKILL);
    } else {
        cout << "Handler not implemented." << endl;
    }
}

/**
 * @brief The program waits until it receives a message from sender.cpp and then handles the message.
 * 
 * @param argc = 2
 * @param argv = [the command with which the program is invoked, 0 or 1 defining wait]
 */
int main(int argc, char const *argv[]) {
    //print receiver pid to send message
    pid_t pid = getpid();
    cout << "Receiver's pid is " << pid << endl;
    //defines that all signals from the system should be handled by our function above
    for (int i=1; i<_NSIG; i++) {
        signal(i, signal_handler);
    }

    int wait_form = atoi(argv[1]); // 0 = busy | 1 = blocking

    if (wait_form == 1) {
        while(1) {
            cout << "While of blocking wait form." << endl;
            pause();
	    }
    } else {
        while(1) {
            cout << "While of busy wait form." << endl;
            sleep(1); 
        }
    }
    
    return 0;
}
