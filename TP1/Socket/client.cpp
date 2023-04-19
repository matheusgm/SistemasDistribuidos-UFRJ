#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define SERVER_PORT 5000

/**
 * @brief returns old number plus random number between 1 and 100
 * 
 * @param old_N number written on pipe in previous iteration
 */
unsigned int new_N(unsigned int old_N) {
    return old_N + (1 + rand()%100);
}

/**
 * @brief connects client to server, sends msgs and prints if is prime
 * 
 * @param argc = 2
 * @param argv = [the command with which the program is invoked, number of msg to be written on pipe]
 */
int main(int argc, char const *argv[]) {

    int sock_host;
    struct sockaddr_in adrServer;
    srand(time(NULL));

    unsigned int max_generated_numbers = atoi(argv[1]);

    // Client socket creation
    sock_host = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_host < 0) {
      cout << "Error on opening socket." << endl;
      return 1;
    }

    // Server address definition
    adrServer.sin_family = AF_INET;
    adrServer.sin_port = htons(SERVER_PORT);
    
    if (connect(sock_host,(struct sockaddr *) &adrServer,sizeof(adrServer)) < 0) {
      cout << "Error connection to server." << endl;
      return 1;
    }

    cout << "Connected to the Server." << endl;

    unsigned int number_N = 1;
    char string_N[20];
    char response[1];

    for (int i = 0; i < max_generated_numbers; i++) {
        number_N = new_N(number_N);
        sprintf(string_N, "%u", number_N);
        
        cout << "Sending number: " << number_N << endl;
        send(sock_host, string_N, sizeof(string_N), 0);
        
        cout << "Waiting response from the server." << endl;

        recv(sock_host,response,sizeof(response),0);
        cout << "Response: " << (int) response[0] << endl;
        
        if (response[0] == 0x1) {
                cout << number_N << " is prime." << endl;
        } else if (response[0] == 0x0) {
            cout << number_N << " is not prime." << endl;
        } else {
            cout << "Error on prime response." << endl;
        }
    }

    // Sending 0 the server to finishe the process
    sprintf(string_N, "%u", 0);
    send(sock_host, string_N, sizeof(string_N), 0);
    
    cout << "Disconnected from the Server." << endl;
    close(sock_host);
    return 0;
}
