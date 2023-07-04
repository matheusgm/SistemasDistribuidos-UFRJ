#include <bits/stdc++.h>
#include<unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

#define SERVER_PORT 7000

int main(int argc, char const *argv[]) {

    int id = atoi(argv[1]); //process id
    int r = atoi(argv[2]); // times each process writes in file
    int k = atoi(argv[3]); // sleep time

    int sock_host;
    struct sockaddr_in adrServer;

    // Client socket creation
    sock_host = socket(AF_INET, SOCK_STREAM, 0);
    while (sock_host < 0) {
      cout << "Error on opening socket." << endl;
      sock_host = socket(AF_INET, SOCK_STREAM, 0);
      sleep(5);
    }

    cout << "Client socket opened." << endl;
    // Server address definition
    adrServer.sin_family = AF_INET;
    adrServer.sin_addr.s_addr = INADDR_ANY;
    adrServer.sin_port = htons(SERVER_PORT);

    while (connect(sock_host,(struct sockaddr *) &adrServer,sizeof(adrServer)) < 0) {
      cout << "Error connection to server." << endl;
      sleep(5);
    }

    cout << "Connected to the Server." << endl;

    char F[10];
    char response[10];

    for (int i = 0; i < r; i++) {
      
      // preparando mensagem de REQUEST
      string msg_request = "1|" + to_string(id) + "|";
      int paddingSize = 9 - msg_request.size();
      if (paddingSize > 0) {
          msg_request.append(paddingSize, '0');
      }
      strcpy(F, msg_request.c_str());
      
      // Solicitando acesso ao coordenador
      cout << "Sending mensage: " << F << endl;
      send(sock_host, F, sizeof(F), 0);
      
      cout << "Waiting response from the server." << endl;

      // Aguardando liberação do coordenador
      recv(sock_host,response,sizeof(response),0);
      // Coordenador liberou
      // Recebeu mensagem de GRANT
      cout << "Response: " << response << endl;

      // Abrir o arquivo 'resultado.txt'


      // Obter hora atual do sistema


      // Escrever identificador e hora atual no arquivo


      // Fechar o arquivo


      // Aguardar k segundos
      sleep(k);

      // Preparando mensagem de RELEASE
      string msg_release = "3|" + to_string(id) + "|";
      paddingSize = 9 - msg_release.size();
      if (paddingSize > 0) {
          msg_release.append(paddingSize, '0');
      }
      strcpy(F, msg_release.c_str());
      
      // Enviando termino ao coordenador
      cout << id << " - Sending mensage: " << F << endl;
      send(sock_host, F, sizeof(F), 0);
    
    }
    
    cout << "Disconnected from the Server." << endl;
    close(sock_host);



    return 0;
}
