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

void Welcome(int TCP_socket) {
    char receiveMessage[512] = {};
    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "[Error] Fail to receive message from the server." << endl;
    }
    else {
        cout << receiveMessage << endl;
    }
}

void Exit(int TCP_socket, string commandInput, int UDP_socket) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};

    int errS = send(TCP_socket,sendMessage,sizeof(sendMessage),0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the server." << endl;
    }
    //logout first
    close(TCP_socket);
    close(UDP_socket);
}

void Register(int UDP_socket, string commandInput, struct sockaddr_in &serverAddr) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};
    socklen_t serverAddrLen = sizeof(serverAddr); 

    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errS == -1) {
        cout << "[Error] Fail to send message to the server." << endl;
    }

    int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage), 0, (struct sockaddr*) &serverAddr, &serverAddrLen);
    if (errR == -1) {
        cout << "[Error] Fail to receive message from the server." << endl;
    }
    else {
        cout << receiveMessage << endl;
    }
}

void Login(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
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

void Logout(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
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

void Rule(int UDP_socket, string commandInput, struct sockaddr_in &serverAddr) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};
    socklen_t serverAddrLen = sizeof(serverAddr); 

    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errS == -1) {
        cout << "[Error] Fail to send message to the server." << endl;
    }

    int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage), 0, (struct sockaddr*) &serverAddr, &serverAddrLen);
    if (errR == -1) {
        cout << "[Error] Fail to receive message from the server." << endl;
    }
    else {
        cout << receiveMessage << endl;
    }
}

void Game(int TCP_socket) {
    string number;
    char sendMessage[512] = {};
    char receiveMessage[512] = {};
	cout << "client game func" << endl;
    while(cin >> number) {
        int len = number.length();
        number.copy(sendMessage, len);
        int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the server." << endl;
        }

        int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the server." << endl;
        }
        else {
            cout << receiveMessage << endl;
            char substr[19];
            strncpy(substr, receiveMessage + 5, 18);
            if (!strcmp(receiveMessage, "You got the answer!") || !strcmp(substr, "You lose the game!")) {
                break;
            }
        }
    }
}

void Start(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[512] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[512] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the server." << endl;
    }

    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "[Error] Fail to receive message from the server." << endl;
    }
    else {
        cout << receiveMessage << endl;
        if (!strcmp(receiveMessage, "Please typing a 4-digit number:")) {
            cout << "call client game func" << endl;
	       	Game(TCP_socket);
        }
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
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }  

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

    // connect to a remote host
    int errC = connect(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errC == -1) {
        cout << "[Error] Fail to connect the server." << endl;
        return 0;
    }
    
    string commandInput = "";
    vector<string> command;

    Welcome(TCP_socket);

    while (getline(cin, commandInput)) {
        command = split(commandInput);
        
        if (command[0] == "exit") {
            Exit(TCP_socket, commandInput, UDP_socket);
            return 0;
        }
        else if (command[0] == "register") {
            Register(UDP_socket, commandInput, serverAddr);
        }
        else if (command[0] == "login") {
            Login(TCP_socket, commandInput);
        }
        else if (command[0] == "logout") {
            Logout(TCP_socket, commandInput);
        }
        else if (command[0] == "game-rule") {
            Rule(UDP_socket,commandInput, serverAddr);
        }
        else if (command[0] == "start-game") {
            Start(TCP_socket, commandInput);
        }
        else {
            cout << "Usage:" << endl;
            cout << "1. register <username> <email> <password>" << endl;
            cout << "2. login <username> <password>" << endl;
            cout << "3. logout" << endl;
            cout << "4. game-rule" << endl;
            cout << "5. start-game <4-digit number>" << endl;
            cout << "6. exit" << endl;
        }
    }
    return 0;
}
