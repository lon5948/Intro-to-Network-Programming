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
    int serverPort,UDP_socket;
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

    // create UDP socket
    UDP_socket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (UDP_socket == -1) {
        cout << "[Error] Fail to create a UDP socket." << endl;
        return 0;
    } 

    // set the socket  
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = inet_addr(serverIP); 

    string commandInput = "";
    vector<string> command;
    vector<string> ans;
    socklen_t serverAddrLen = sizeof(serverAddr);
    int len;
    char sendMessage[512] = {};
    char receiveMessage[512] = {};

    cout << "% " ;
    while (getline(cin, commandInput)) {
        
        command = split(commandInput);
        len = commandInput.length();
        
        commandInput.copy(sendMessage, len);
        int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
        if (errS == -1) {
            cout << "[Error] Fail to send message to the server." << endl;
        }
        
        if (command[0] == "get-file-list") {
            int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage), 0, (struct sockaddr*) &serverAddr, &serverAddrLen);
            if (errR == -1) {
                cout << "[Error] Fail to receive message from the server." << endl;
            }
            else {
                ans = split(receiveMessage);
                for (int i = 0; i < ans.size(); i++) {
                    cout << ans[i] << " ";
                }
                cout << endl;
            }
        }
        else if (command[0] == "get-file") {
            
        }
        else if (command[0] == "exit") {
            close(UDP_socket);
            break;
        }
        else {
            cout << "Usage:" << endl;
            cout << "1. get-file-list" << endl;
            cout << "2. Files: {file-name1} {file-name2} {file-name3} ..." << endl;
            cout << "3. get-file {file-name1} {file-name2} {file-name3}... " << endl;
            cout << "4. exit" << endl;
        }

        memset(sendMessage, '\0', sizeof(sendMessage));
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        commandInput.clear();

        cout << "% " ;
    }
    return 0;
}