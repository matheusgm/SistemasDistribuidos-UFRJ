#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define SERVER_PORT 9503
#define LOG_NAME "[PROCESS] "

ofstream outputFile;
pid_t processId = getpid();

void printTime()
{
    // Get the current system time
    auto currentTime = chrono::system_clock::now();

    // Convert the current time to a time_t object
    time_t time = chrono::system_clock::to_time_t(currentTime);

    // Convert the time_t object to a string representation
    string timeString = ctime(&time);

    // Get the milliseconds
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(
                            currentTime.time_since_epoch())
                            .count() %
                        1000;

    // Format the time string
    tm *timeinfo = localtime(&time);
    stringstream ss;
    ss << setfill('0') << setw(2) << timeinfo->tm_mday << "/"
       << setw(2) << (timeinfo->tm_mon + 1) << "/"
       << (timeinfo->tm_year + 1900) << " "
       << setw(2) << timeinfo->tm_hour << ":"
       << setw(2) << timeinfo->tm_min << ":"
       << setw(2) << timeinfo->tm_sec << ":"
       << setw(3) << milliseconds;

    string formattedTime = ss.str();

    // Open the file for writing
    outputFile.open("resultado.txt", ios_base::app); // append mode
    if (outputFile.is_open())
    {
        // Write the formatted time to the file
        outputFile << processId << " - " << formattedTime << endl;
        // Close the file
        outputFile.close();
    }
    else
    {
        cout << "Unable to open the file for writing." << endl;
    }
}

int main(int argc, char const *argv[])
{

    // Parsing arguments
    int n = atoi(argv[1]); // number of processes
    int r = atoi(argv[2]); // times each process writes in file
    int k = atoi(argv[3]); // sleep time

    int sock_host;
    struct sockaddr_in adrServer;

    // Client socket creation
    sock_host = socket(AF_LOCAL, SOCK_STREAM, 0);
    while (sock_host < 0)
    {
        cout << LOG_NAME << "Error on opening socket." << endl;
        sock_host = socket(AF_LOCAL, SOCK_STREAM, 0);
        sleep(5);
    }

    cout << LOG_NAME << "Client socket opened." << endl;
    // Server address definition
    adrServer.sin_family = AF_LOCAL;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    while (connect(sock_host, (struct sockaddr *)&adrServer, sizeof(adrServer)) < 0)
    {
        cout << LOG_NAME << "Error connection to server." << endl;
        sleep(5);
    }

    cout << LOG_NAME << "Connected to the Server." << endl;
    sleep(5);

    char F[10];
    char response[10];

    for (int i = 0; i < r; i++)
    {
        // Preparing REQUEST message
        string msg_request = "1|" + to_string(processId) + "|";
        int paddingSize = 9 - msg_request.size();
        if (paddingSize > 0)
        {
            msg_request.append(paddingSize, '0');
        }
        strcpy(F, msg_request.c_str());

        // Sending REQUEST message
        cout << LOG_NAME << "Process: Sending mensage " << F << endl;
        send(sock_host, F, sizeof(F), 0);

        cout << LOG_NAME << "Waiting response from the server." << endl;

        // Received GRANT message
        recv(sock_host, response, sizeof(response), 0);
        cout << LOG_NAME << "Process: server response " << response << endl;

        // Entering Critical Region
        printTime();
        sleep(k);
        // Leaving Critical Region

        // Preparing RELEASE message
        string msg_release = "3|" + to_string(processId) + "|";
        paddingSize = 9 - msg_release.size();
        if (paddingSize > 0)
        {
            msg_release.append(paddingSize, '0');
        }
        strcpy(F, msg_release.c_str());

        // Sending RELEASE message
        cout << LOG_NAME << "Process: Sending mensage  " << F << endl;
        send(sock_host, F, sizeof(F), 0);
    }

    cout << LOG_NAME << "Process: Disconnected from the Server." << endl;
    close(sock_host);

    return 0;
}
