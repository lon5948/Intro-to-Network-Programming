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

int main(int argc, char* argv[]) {
    char* serverIP;
    int serverPort;
    int TCP_socket,UDP_socket;
    struct sockaddr_in serverAddr;  

    // check command format  
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

    // create TCP socket  
    TCP_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (TCP_socket == -1) {
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }  

    // set the socket  
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); 

    // connect to a remote host
    int errC = connect(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errC == -1) {
        cout << "[Error] Fail to connect the server." << endl;
        return 0;
    }
    
    string commandInput = "";
    vector<string> command;
    char sendMessage[512] = {};
    char receiveMessage[512] = {};

    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
      cout << "[Error] Fail to receive message from the server." << endl;
    }
    else {
        cout << receiveMessage << endl;
    }

    cout << "% ";
    while (getline(cin, commandInput)) {
        
        command = split(commandInput);
        
        if (command[0] == "list-users" || command[0] == "get-ip") {
            if (command.size() != 1 && command[0] == "list-users") {
                cout << "Usage: list-users" << endl;
            }
            else if (command.size() != 1 && command[0] == "get-ip") {
                cout << "Usage: get-ip" << endl;
            }
            else {
                int len = commandInput.length();
                commandInput.copy(sendMessage, len);
                int errS = send(TCP_socket,sendMessage,sizeof(sendMessage),0);
                if (errS == -1) {
                    cout << "[Error] Fail to send message to the server." << endl;
                }
                int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
                if (errR == -1) {
                    cout << "[Error] Fail to receive message from the server." << endl;
                }
                else {
                    cout << receiveMessage << endl;
                }
            }
        }
        else if (command[0] == "exit") {
            if (command.size() != 1) {
                cout << "Usage: exit" << endl;
            }
            else {
                int len = commandInput.length();
                commandInput.copy(sendMessage, len);
                int errS = send(TCP_socket,sendMessage,sizeof(sendMessage),0);
                if (errS == -1) {
                    cout << "[Error] Fail to send message to the server." << endl;
                }
                int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
                if (errR == -1) {
                    cout << "[Error] Fail to receive message from the server." << endl;
                }
                else {
                    cout << receiveMessage << endl;
                }
                close(TCP_socket);
                return 0;
            } 
        }
        else {
            cout << "Usage:" << endl;
            cout << "1. list-users" << endl;
            cout << "2. get-ip" << endl;
            cout << "3. exit" << endl;
        }
        
        memset(sendMessage, '\0', sizeof(sendMessage));
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        commandInput.clear();

        cout << "% " ;
    }
    return 0;
}
