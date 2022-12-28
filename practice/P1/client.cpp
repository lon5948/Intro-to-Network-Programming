#include <iostream>  
#include <vector>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

using namespace std;

vector<string> split(string str) {
    vector<string> result;
    stringstream ss(str);
    string token;

    while (getline(ss,token,' ')) {
        result.push_back(token);
    }
    return result;
}

void Exit(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[1024] = {};
    commandInput.copy(sendMessage, len);
    int errS = send(TCP_socket,sendMessage,sizeof(sendMessage),0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the server." << endl;
    }
    close(TCP_socket);
}

void* Receive_Message(void* data) {
    int TCP_socket = *((int*) data);
    char receiveMessage[1024] = {};
    int length;
    while (length = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0)) {
        receiveMessage[length] = '\0';
        cout << receiveMessage << endl;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    char* serverIP;
    int serverPort;
    int TCP_socket;
    struct sockaddr_in serverAddr;  

    try {
        if (argc!=3) {
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

    TCP_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (TCP_socket == -1) {
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }  

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); 

    int errC = connect(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errC == -1) {
        cout << "[Error] Fail to connect the server." << endl;
        return 0;
    }
    
    string commandInput = "";
    vector<string> command;

    cout << "******************************" << endl;
    cout << "* Welcome to the BBS server. *" << endl;
    cout << "******************************" << endl;

    pthread_t pid;
    pthread_create(&pid, NULL, Receive_Message, (void* )&TCP_socket);

    while (getline(cin, commandInput)) {
        command = split(commandInput);
        if (command[0] == "exit") {
            Exit(TCP_socket, commandInput);
            return 0;
        }
        else {
            int len = commandInput.length();
            char sendMessage[1024] = {};
            commandInput.copy(sendMessage, len);
            int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the server." << endl;
            }
        } 
    }
}
