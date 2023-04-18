#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define SERVER_PORT 5000

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

int main(int argc, char const *argv[]) {
    
    int sock_server, sock_client;
    struct sockaddr_in adrServer, adrClient;
    char buffer[20];
    unsigned char msg[1] = {};  // Byte buffer

    // Server socket creation
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 0) {
      cout << "Error on opening socket." << endl;
      return 1;
    }

    // Server address definition
    adrServer.sin_family = AF_INET;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    if (bind(sock_server,(struct sockaddr *) &adrServer,sizeof(adrServer)) < 0) {
      cout << "Error when trying to bind socket." << endl;
      return 1;
    }

    listen(sock_server,2);

    cout << "Server Listening..." << endl;

    socklen_t adrClientSize = sizeof(adrClient);
    sock_client = accept(sock_server, (struct sockaddr *) &adrClient, &adrClientSize);
    if (sock_server < 0) {
      cout << "Error accepting client." << endl;
      return 1;
    }

    cout << "Client: " << sock_client << " accepted." << endl;

    while(true) {
      recv(sock_client,buffer,sizeof(buffer),0);

      cout << buffer << endl;
      int int_number = atoi(buffer);

      if (int_number == 0) {
        cout << "Number 0 received." << endl;
        break;
      }

      if (is_prime(int_number)) {
        msg[0] = 0x1;
      } else {
        msg[0] = 0x0;
      }

      // Sending a byte with the result byte
      send(sock_client, msg, sizeof(msg), 0);

    }

    close(sock_client);
    close(sock_server);
    
    
    return 0;
}
