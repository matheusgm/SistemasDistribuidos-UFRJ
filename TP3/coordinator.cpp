#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <atomic>

using namespace std;

#define SERVER_PORT 7000

int n;
int r;
pid_t coordinatorId = getpid();

mutex mtx;
condition_variable cv;
queue<std::tuple<std::string, int>> processQueue;
atomic<bool> terminateInterfaceFlag(false);



int interfaceThread() {
    while (!terminateInterfaceFlag) {
        string command;
        cout << "Enter a command: \n";
        getline(cin, command);

        if (command == "1") {
            // Print the current order queue
            cout << "Current order queue: \n";
            // Create a copy of the queue
            queue<tuple<string, int>> myQueueCopy = processQueue;

            // Print the copied queue
            while (!myQueueCopy.empty()) {
                tuple<string, int> frontTuple = myQueueCopy.front();
                string id = get<0>(frontTuple);  // Get the first element of the tuple
                cout << id << ", ";
                myQueueCopy.pop();
            }
            cout<< endl;
           
        } else if (command == "2") {
            // Print how many times each process has been serviced
            cout << "Times each process has been serviced.\n";
            
        } else if (command == "3") {
            // Terminate the coordinator's execution
            cout<< "Terminating...\n";
            break;
        } else {
            cout << "Invalid command. Try again, available commands" << endl;
            cout << "1: Print the current order queue.\n";
            cout << "2: Print how many times each process has been serviced.\n'";
            cout << "3: Terminate the coordinator's execution.\n";     
        }
    }

    cout << "Server: Finishing Interface.\n";
    return 0;
}

int granter(){
    char F[10]; //size of message

    // Each process will write in the file r times
    // We have n processes
    // So n_msgs = n*r, then finishes
    for (int i = 0; i < n*r; i++){
        //If queue is empty, waits
        while (processQueue.empty()) {
            sleep(1);
        }

        //Removing First Element
        tuple<string, int> first = processQueue.front();
        processQueue.pop();
        int sock_client = get<1>(first);

        //Send grant message
        string msg_grant = "2|" + to_string(coordinatorId) + "|";
        int paddingSize = 9 - msg_grant.size();

        if (paddingSize > 0) {
                msg_grant.append(paddingSize, '0');
        }
        strcpy(F, msg_grant.c_str());
        send(sock_client, F, sizeof(F), 0);

        //Wait until release
        unique_lock<mutex> lck(mtx);
        cv.wait(lck);
    }

    cout << "Server: Finishing Granter.\n";

    return 0;
}

int handleClient(int sock_client) {
    char F[10]; //size of message

    // Each process will write in the file r times
    // It sends two messages to write: request and release
    // So n_msgs = 2*r, then finishes
    for (int i = 0; i < (2*r); i++) {
        recv(sock_client, F, sizeof(F), 0);
        cout << "Server: msg received: " <<F;

        char msg[10];
        char id[10];

        // Parsing Message
        char* firstDelim = strchr(F, '|'); // Find the first occurrence of "|"
        if (firstDelim != nullptr)
        {
            // Calculate the index of the first "|"
            size_t msgSize = firstDelim - F;

            // Copy the first part to the firstPart array
            strncpy(msg, F, msgSize);
            msg[msgSize] = '\0';

            // Find the second occurrence of "|"
            char* secondDelim = strchr(firstDelim + 1, '|');
            if (secondDelim != nullptr)
            {
                // Calculate the index of the second "|"
                size_t idSize = secondDelim - firstDelim - 1;

                // Copy the second part to the secondPart array
                strncpy(id, firstDelim + 1, idSize);
                id[idSize] = '\0';
            }
        }
        
        string msgString(msg);
        string idString(id);

        if(msgString == "1"){
            //adicionar ao log
            processQueue.push(make_tuple(idString, sock_client));
        }
        else if(msgString == "3"){
            //adicionar ao log
            cv.notify_one();  // Notify a waiting thread
        }
        else{
            cout<< "Message not recognized, please use '1' for request and '3' for release.\n";
        }
    }

    close(sock_client);
    cout << "Server: Closing Client Socket.\n";;
    return 0;
}


int server() {
    int sock_server, sock_client;
    struct sockaddr_in adrServer, adrClient;

    // Server socket creation
    sock_server = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock_server < 0) {
        cout << "Error on opening socket." << endl;
        return 0;
    }

    // Server address definition
    adrServer.sin_family = AF_LOCAL;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    if (bind(sock_server, (struct sockaddr *)&adrServer, sizeof(adrServer)) < 0) {
        cout << "Error when trying to bind socket." << endl;
        return 0;
    }

    listen(sock_server, 256);

    cout << "Server Listening..." << endl;

    socklen_t adrClientSize = sizeof(adrClient);

    thread granterThread(granter);
    vector<thread> clientThreads(n);

    // We will have n clients
    // 1 thread for each client, so n threads
    for (int i=0; i<n; i++) {
        sock_client = accept(sock_server, (struct sockaddr *)&adrClient, &adrClientSize);
        if (sock_client < 0) {
            cout << "Error accepting client." << endl;
            continue;
        }

        cout << "Server: client: " << sock_client << " accepted." << endl;

        clientThreads[i] = thread(handleClient, sock_client);
    }

    // Wait for threads to finish
    for(auto& thread : clientThreads){
        thread.join();
    }
    granterThread.join();
    close(sock_server);
    cout << "Server: Closing Server Socket.\n";
    return 0;
}

int main(int argc, char const *argv[]) {
    // Parsing arguments
    n = atoi(argv[1]); //number of processes
    r = atoi(argv[2]); // times each process writes in file
    int k = atoi(argv[3]); // sleep time
    cout << "Coordinator ON.\n";

    // Create and open log text file
    ofstream MyLog("log.txt");
    MyLog.close(); // Close file

    thread interface(interfaceThread);
    server();
    // Set the termination flag to request thread termination
    terminateInterfaceFlag = true;
    interface.join();

    cout << "Coordinator OFF.\n";

    return 0;
}
