#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

void signal_handler(int signal) {
    if (signal == 2) { // SIGINT
        cout << "O sinal 2 (SIGINT) foi recebido com sucesso." << endl;
    } else if (signal == 10) { // SIGUSR1
        cout << "O sinal 10 (SIGUSR1) foi recebido com sucesso." << endl;
    } else if (signal == 12) { // SIGUSR2
        cout << "O sinal 12 (SIGUSR2) foi recebido com sucesso." << endl;
        cout << "Finalizando o processo." << endl;
        raise(SIGKILL);
    } else {
        cout << "Handler não implementado." << endl;
    }
}

int main(int argc, char const *argv[]) {
    signal(2, signal_handler);
	signal(10, signal_handler);
	signal(12, signal_handler);

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
