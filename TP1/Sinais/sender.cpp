#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

int main(int argc, char const *argv[]) {
    int pid = atoi(argv[1]);
    int signal = atoi(argv[2]);

    int res = kill(pid, signal);

    if (res == -1)
		cout << "O processo não existe\n";
    
    return 0;
}
