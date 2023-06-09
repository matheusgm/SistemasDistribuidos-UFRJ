#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/wait.h>

using namespace std;

/**
 * @brief returns old number plus random number between 1 and 100
 * 
 * @param old_N number written on pipe in previous iteration
 */
unsigned int new_N(unsigned int old_N) {
    return old_N + (1 + rand()%100);
}

/**
 * @brief checks if n in prime in O(n)
 * 
 * @param n number to be checked if prime
 * @return true if n is prime
 * @return false if n is not prime
 */
bool is_prime(unsigned int n) {
  if (n == 0 || n == 1) {
    return false;
  }

  for (int i = 2; i <= n / 2; ++i) {
    if (n % i == 0) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Creates pipe, fork process; Child reads and checks if prime; parent generates num and writes
 * 
 * @param argc = 2
 * @param argv = [the command with which the program is invoked, number of msg to be written on pipe]
 */
int main(int argc, char const *argv[]) {
    int pid;
    int pipefd[2];
    srand((unsigned) time(NULL));    

    unsigned int max_generated_numbers = atoi(argv[1]);

    if (pipe(pipefd) < 0) { //checks if pipe was successfully created
		cout << "Error on creating a pipe." << endl;
		return 1;
	}

	pid = fork();
	if (pid < 0) { //checks if fork was successful
		cout << "Error on forking." << endl;
		return 1;
	} 
  else if (pid == 0) { // Child Process, return 0
        close(pipefd[1]); // Closing write-end pipe

        char receive_value[20];
        read(pipefd[0],&receive_value,sizeof(receive_value));
        cout << "Receiving number: " << receive_value << endl;
        unsigned int value_int = atoi(receive_value);

        while (value_int != 0) {
            if (is_prime(value_int)) {
                cout << value_int << " is prime." << endl;
            } else {
                cout << value_int << " is not prime." << endl;
            }
            read(pipefd[0],&receive_value,sizeof(receive_value));
            cout << "Receiving number: " << receive_value << endl;
            value_int = atoi(receive_value);
        }

        cout << "killing process." << endl;
        close(pipefd[0]); // Closing read-end pipe
        return 0;

    } else { // Parent Process, return the child PID
        close(pipefd[0]); // Closing read-end pipe

        unsigned int number_N = 1;
        char string_N[20];

        for (int i = 0; i < max_generated_numbers; i++) {
            number_N = new_N(number_N);
            sprintf(string_N, "%u", number_N);
            cout << "Sending number: " << number_N << endl;
            write(pipefd[1], &string_N, sizeof(string_N));
        }

        sprintf(string_N, "%d", 0);
        write(pipefd[1], &string_N, sizeof(string_N));
        cout << "Sending number: 0" << endl;

        close(pipefd[1]); // Closing write-end pipe
        wait(NULL);
        return 0;
    }
    
    return 0;
}
