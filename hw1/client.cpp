#include <iostream>  
#include <vector>
#include <cstring>
#include <string>
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

void Exit(int TCP_socket, int UDP_socket) {
    char sendMessage[] = { "exit" };

    int errS = send(TCP_socket,sendMessage,sizeof(sendMessage),0);
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }
    //logout first
    close(TCP_socket);
    close(UDP_socket);
}

void Register(int UDP_socket, string commandInput, struct sockaddr_in &serverAddr) {
    len = commandInput.length();
    char sendMessage[len] = {};
    char receiveMessage[128] = {};
    commandInput.copy(sendMessage,len);

    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "Success") {
        cout << "Register successfully." << endl;
    }
    else if (receiveMessage == "Fail1") {
        cout << "Username is already used." << endl;
    }
    else if (receiveMessage == "Fail2") {
        cout << "Email is already used." << endl;
    }
}

int main(int argc, char* argv[]) {

    char* serverIP;
    int serverPort;
    int TCP_socket,UDP_socket;
    struct sockaddr_in serverAddr;  

    // check command format  
    const char* s = "./client";
    try {
        if (argc!=3 || strcmp(argv[0], s)) {
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
    bzero(&serverAddr, izeof(serverAddr));
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
            Exit(TCP_socket, UDP_socket);
            return 0;
        }
        else if (command[0] == "register") {
            if (command.size() != 4){
                cout<<"Usage: register <username> <email> <password>"<<endl;
            }
            else {
                Register();
            }
        }
    }

    return 0;
}