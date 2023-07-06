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
#include <map>

using namespace std;

#define SERVER_PORT 9505
#define LOG_NAME "[COORDINATOR] "

int n;
int r;
pid_t coordinatorId = getpid();

mutex mtx;
ofstream logFile;
mutex log_mtx;
mutex statistic_mtx;
mutex queue_mtx;
condition_variable cv;
queue<std::tuple<std::string, int>> processQueue;
map<string, int> statisticMap;
atomic<bool> terminateInterfaceFlag(false);

void write_on_log(string header, string client)
{
    log_mtx.lock();
    // Open the file for writing
    logFile.open("log.txt", ios_base::app); // append mode
    if (logFile.is_open())
    {
        // Write the formatted time to the file
        logFile << header << " - " << client << endl;
        // Close the file
        logFile.close();
    }
    else
    {
        cout << "Unable to open the log file for writing." << endl;
    }
    log_mtx.unlock();
}

int interfaceThread()
{

    while (!terminateInterfaceFlag)
    {
        string command;
        cout << LOG_NAME << "Enter a command: \n";
        getline(cin, command);

        if (command == "1")
        {
            // Print the current order queue
            cout << LOG_NAME << "Current order queue: \n";
            // Create a copy of the queue
            queue_mtx.lock();
            queue<tuple<string, int>> myQueueCopy = processQueue;
            queue_mtx.unlock();

            // Print the copied queue
            while (!myQueueCopy.empty())
            {
                tuple<string, int> frontTuple = myQueueCopy.front();
                string id = get<0>(frontTuple); // Get the first element of the tuple
                cout << id << ", ";
                myQueueCopy.pop();
            }
            cout << endl;
        }
        else if (command == "2")
        {
            // Print how many times each process has been serviced
            cout << LOG_NAME << "Times each process has been serviced.\n";

            statistic_mtx.lock();
            map<string, int>::iterator it;

            for (it = statisticMap.begin(); it != statisticMap.end(); it++)
            {
                cout << LOG_NAME << it->first << ':' << it->second << endl;
            }
            statistic_mtx.unlock();
        }
        else if (command == "3")
        {
            // Terminate the coordinator's execution
            cout << LOG_NAME << "Terminating...\n";
            terminateInterfaceFlag = true;
        }
        else
        {
            cout << LOG_NAME << "Invalid command. Try again, available commands" << endl;
            cout << LOG_NAME << "1: Print the current order queue.\n";
            cout << LOG_NAME << "2: Print how many times each process has been serviced.\n'";
            cout << LOG_NAME << "3: Terminate the coordinator's execution.\n";
        }
    }

    cout << LOG_NAME << "Finishing Interface.\n";
    return 0;
}

int granter()
{
    char F[10]; // size of message

    while (!terminateInterfaceFlag)
    {
        // If queue is empty, waits
        while (processQueue.empty() && !terminateInterfaceFlag)
        {
            sleep(1);
        }

        if (terminateInterfaceFlag)
            break;

        // Removing First Element
        queue_mtx.lock();
        tuple<string, int> first = processQueue.front();
        processQueue.pop();
        queue_mtx.unlock();
        int sock_client = get<1>(first);
        string idString = get<0>(first);

        // Send grant message
        string msg_grant = "2|" + to_string(coordinatorId) + "|";
        int paddingSize = 9 - msg_grant.size();

        if (paddingSize > 0)
        {
            msg_grant.append(paddingSize, '0');
        }
        strcpy(F, msg_grant.c_str());
        send(sock_client, F, sizeof(F), 0);
        write_on_log("[S] Grant", idString);

        statistic_mtx.lock();
        statisticMap[idString] = statisticMap[idString] + 1;
        statistic_mtx.unlock();

        // Wait until release
        unique_lock<mutex> lck(mtx);
        cv.wait(lck);
    }

    cout << LOG_NAME << "Finishing Granter.\n";

    return 0;
}

int handleClient(int sock_client)
{
    char F[10]; // size of message
    cout << LOG_NAME << "Handle Client " << sock_client << endl;

    while (!terminateInterfaceFlag)
    {
        cout << LOG_NAME << "Waiting message from client " << sock_client << endl;
        int by = recv(sock_client, F, sizeof(F), 0);
        if (by == 0)
            break; // Cliente Disconnected
        cout << LOG_NAME << "Message received: " << F << endl;

        char msg[10];
        char id[10];

        // Parsing Message
        char *firstDelim = strchr(F, '|'); // Find the first occurrence of "|"
        if (firstDelim != nullptr)
        {
            // Calculate the index of the first "|"
            size_t msgSize = firstDelim - F;

            // Copy the first part to the firstPart array
            strncpy(msg, F, msgSize);
            msg[msgSize] = '\0';

            // Find the second occurrence of "|"
            char *secondDelim = strchr(firstDelim + 1, '|');
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

        if (msgString == "1")
        {
            queue_mtx.lock();
            processQueue.push(make_tuple(idString, sock_client));
            queue_mtx.unlock();
            // Add to log
            write_on_log("[R] Request", idString);
            statistic_mtx.lock();
            map<string, int>::iterator it = statisticMap.find(idString);
            if ( it == statisticMap.end())
                statisticMap[idString] = 0;
            statistic_mtx.unlock();
        }
        else if (msgString == "3")
        {
            // Add to log
            write_on_log("[R] Release", idString);
            cv.notify_one(); // Notify a waiting thread
        }
        else
        {
            cout << LOG_NAME << "Message not recognized, please use '1' for request and '3' for release.\n";
        }
    }

    close(sock_client);
    cout << LOG_NAME << "Closing Client Socket.\n";
    return 0;
}

int server()
{
    int sock_server, sock_client;
    struct sockaddr_in adrServer, adrClient;

    // Server socket creation
    sock_server = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        cout << LOG_NAME << "Error on opening socket." << endl;
        return 0;
    }

    // Server address definition
    adrServer.sin_family = AF_LOCAL;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    if (bind(sock_server, (struct sockaddr *)&adrServer, sizeof(adrServer)) < 0)
    {
        cout << LOG_NAME << "Error when trying to bind socket." << endl;
        return 0;
    }

    listen(sock_server, n);

    cout << LOG_NAME << "Server Listening..." << endl;

    socklen_t adrClientSize = sizeof(adrClient);

    thread granterThread(granter);
    vector<thread> clientThreads(n);

    // We will have n clients
    // 1 thread for each client, so n threads
    for (int i = 0; i < n; i++)
    {
        sock_client = accept(sock_server, (struct sockaddr *)&adrClient, &adrClientSize);
        if (sock_client < 0)
        {
            cout << LOG_NAME << "Error accepting client." << endl;
            continue;
        }

        cout << LOG_NAME << "client " << sock_client << " accepted." << endl;

        clientThreads[i] = thread(handleClient, sock_client);
    }

    // Wait for threads to finish
    for (auto &thread : clientThreads)
    {
        thread.join();
    }
    granterThread.join();
    close(sock_server);
    cout << LOG_NAME << "Closing Server Socket.\n";
    return 0;
}

int main(int argc, char const *argv[])
{
    // Parsing arguments
    n = atoi(argv[1]);     // number of processes
    r = atoi(argv[2]);     // times each process writes in file
    int k = atoi(argv[3]); // sleep time
    cout << LOG_NAME << "Coordinator ON.\n";

    // Create and open log text file
    ofstream MyLog("log.txt");
    MyLog.close(); // Close file

    thread interface(interfaceThread);
    server();
    interface.join();

    cout << LOG_NAME << "Coordinator OFF.\n";

    return 0;
}
