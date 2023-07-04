#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>

using namespace std;

#define SERVER_PORT 7000

void handleClient(int sock_client, bool *isRunning) {
    char F[10]; //size of message

    while (*isRunning) {
        
        recv(sock_client, F, sizeof(F), 0);

        if (F[0] == '1') {
            // Recebe REQUEST do cliente


            // Prepara mensagem de GRANT
            string msg_grant = "2|" + to_string(0) + "|";
            int paddingSize = 9 - msg_grant.size();
            if (paddingSize > 0) {
                msg_grant.append(paddingSize, '0');
            }
            strcpy(F, msg_grant.c_str());

            // Envia mensagem de GRANT para o cliente
            send(sock_client, F, sizeof(F), 0);
        } else if(F[0] == '3') {
            // Recebe RELEASE do cliente

        }        
    }

    close(sock_client);
}

void interfaceThread(bool *isRunning) {
    while (*isRunning) {
        string command;
        cout << "Enter a command: ";
        getline(cin, command);

        if (command == "1") {
            // Print the current order queue
            cout << "Current order queue: \n";
           
        } else if (command == "2") {
            // Print how many times each process has been serviced
            cout << "Times each process has been serviced.\n";
            
        } else if (command == "3") {
            // Terminate the coordinator's execution
            cout<< "Terminating...\n";
            *isRunning = false;
        } else {
            cout << "Invalid command. Try again, available commands" << endl;
            cout << "1: Print the current order queue.\n";
            cout << "2: Print how many times each process has been serviced.\n'";
            cout << "3: Terminate the coordinator's execution.\n";     
        }
    }
}

void server(bool *isRunning) {
    int sock_server, sock_client;
    struct sockaddr_in adrServer, adrClient;

    // Server socket creation
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 0) {
        cout << "Error on opening socket." << endl;
        *isRunning = false;
        return;
    }

    // Server address definition
    adrServer.sin_family = AF_INET;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    if (bind(sock_server, (struct sockaddr *)&adrServer, sizeof(adrServer)) < 0) {
        cout << "Error when trying to bind socket." << endl;
        *isRunning = false;
        return;
    }

    listen(sock_server, 10);

    cout << "Server Listening..." << endl;

    socklen_t adrClientSize = sizeof(adrClient);
    while (*isRunning) {
        sock_client = accept(sock_server, (struct sockaddr *)&adrClient, &adrClientSize);
        if (sock_client < 0) {
            cout << "Error accepting client." << endl;
            continue;
        }

        cout << "Client: " << sock_client << " accepted." << endl;

        thread clientThread(handleClient, sock_client, isRunning);
        clientThread.detach();
    }

    close(sock_server);
}

int main(int argc, char const *argv[]) {
    cout << "Coordinator ON.\n";
    bool isRunning = true;
    thread interface(interfaceThread, &isRunning);
    server(&isRunning);
    interface.join();
    return 0;
}
