#include <iostream>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

using namespace std;

#define MAX_CLIENT 10

int main(int argc, char* argv[]) {  

    int serverPort;
    int TCP_socket,UDP_socket;  
    struct sockaddr_in serverAddr;  
    struct sockaddr_in clientAddr; 
    int client_socket;  

    // check command format 
    const char* s = "./server";
    try {
        if (argc!=2 || strcmp(argv[0],s)) {
            cout << "Usage: ./server <server port>" << endl;
            return 0;
        }
        serverPort = atoi(argv[1]);
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        cout << "Usage: ./server <server port>" << endl;
    }
    
    // create TCP and UDP socket  
    TCP_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (TCP_socket == -1) {
        cout << "Error: Fail to create a TCP socket." << endl;
        return 0;
    }  

    UDP_socket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (UDP_socket == -1) {
        cout << "Error: Fail to create a UDP socket." << endl;
        return 0;
    } 

    // set the socket  
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = INADDR_ANY; 

    // bind a socket to a local IP address and a port number
    int errBT = bind(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBT == -1) {
        cout << "Error: Fail to bind a TCP socket." << endl;
        return 0;
    }
    int errBU = bind(UDP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBU == -1) {
        cout << "Error: Fail to bind a UDP socket." << endl;
        return 0;
    }
    
    // wait for someone to connect to my port(used by servers)
    int errL = listen(TCP_socket, MAX_CLIENT);  
    if (errL == -1) {
        cout << "Error: Fail to listen." << endl;
        return 0;
    }
    
    while(1) {
        // accept a new connection and get a new socket for subsequent data transmissions   
        client_socket = accept(TCP_socket, (struct sockaddr*) &clientAddr, &sizeof(clientAddr));  
        cout << "New connection." << endl;
        
        // close the socket   
        close(client_socket);   
        close(UDP_socket);  
        return 0; 
    }
}  