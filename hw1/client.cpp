#include <iostream>  
#include <vector>
#include <cstring>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

using namespace std;

vector<string> split(str) {

    vector<string> result;
    stringstream ss(str);
    string token;

    while (getline(ss,token,' ')) {
        result.push_back(token);
    }
    return result;
}

void Exit(int TCP_socket,int UDP_socket) {
    char message[] = { "exit" };

    int err = send(TCP_socket,message,sizeof(message),0);
    if (err == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }
    //logout first
    close(TCP_socket);
    close(UDP_socket);
}

int main(int argc, char* argv[]) {

    char* serverIP;
    int serverPort;
    int TCP_socket,UDP_socket;
    struct sockaddr_in serverAddr;  

    // check command format  
    const char* s = "./client";
    try {
        if (argc!=3 || strcmp(argv[0],s)) {
            cout << "Usage: ./client <server IP> <server port>" << endl;
            return 0;
        }
        serverIP = argv[1];
        serverPort = atoi(argv[2]);
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        cout << "Usage: ./client <server IP> <server port>" << endl;
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
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); 

    // connect to a remote host
    int err = connect(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (err == -1) {
        cout << "Error: Fail to connect server." << endl;
        return 0;
    }
    else if (err == 0) {
        cout << "*****Welcome to Game 1A2B*****" << endl;
    }
    
    string commandInput = "";
    string
    vector<string> command;

    while (getline(cin, commandInput)) {
        command = split(commandInput);
        
        if (command[0] == "exit") {
            Exit(TCP_socket,UDP_socket);
            return 0;
        }
    }

    return 0;
}