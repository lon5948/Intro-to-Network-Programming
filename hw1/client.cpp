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
    int len = commandInput.length();
    char sendMessage[len] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[128] = {};

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

void Login(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[len] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[128] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "Success") {
        cout << "Welcome," << username/receiveMessage << "." << endl;
    }
    else if (receiveMessage == "Fail1") {
        cout << "Please logout first." << endl;
    }
    else if (receiveMessage == "Fail2") {
        cout << "Username not found." << endl;
    }
    else if (receiveMessage == "Fail3") {
        cout << "Password not correct." << endl;
    }
}

void Logout(int TCP_socket) {
    char sendMessage[] = { "logout" };
    char receiveMessage[128] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "Success") {
        cout << "Bye," << username/receiveMessage << "." << endl;
    }
    else if (receiveMessage == "Fail") {
        cout << "Please login first." << endl;
    }
}

void Rule(int UDP_socket, struct sockaddr_in &serverAddr) {
    char sendMessage[] = { "rule" };
    char receiveMessage[1024] = {};

    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage), 0, (const struct sockaddr*) &serverAddr, sizeof(serverAddr));
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "game-rule") {
        cout << "1. Each question is a 4-digit secret number." << endl;
        cout << "2. After each guess, you will get a hint with the following information:" << endl;
        cout << "2.1 The number of \"A\", which are digits in the guess that are in the correct position." << endl;
        cout << "2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position." << endl;
        cout << "The hint will be formatted as \"xAyB\"." << endl;
        cout << "3. 5 chances for each question." << endl;
    }
}

void Start(int TCP_socket, string commandInput, struct sockaddr_in &serverAddr) {
    int len = commandInput.length();
    char sendMessage[len] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[128] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recv(UDP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "Fail1") {
        cout << "Please login first." << endl;
    }
    else if (receiveMessage == "Fail2") {
        cout << "Usage: start-game <4-digit number>" << endl;
    }
    else if (receiveMessage == "Start") {
        cout << "Please typing a 4-digit number:" << endl;
    }
}

void Start(int TCP_socket, string commandInput) {
    int len = commandInput.length();
    char sendMessage[len] = {};
    commandInput.copy(sendMessage, len);
    char receiveMessage[128] = {};

    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "Error: Fail to send message to the server." << endl;
    }

    int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
    if (errR == -1) {
        cout << "Error: Fail to receive message from the server." << endl;
    }
    else if (receiveMessage == "Fail1") {
        cout << "Please login first." << endl;
    }
    else if (receiveMessage == "Fail2") {
        cout << "Usage: start-game <4-digit number>" << endl;
    }
    else if (receiveMessage == "Start") {
        cout << "Please typing a 4-digit number:" << endl;
        Game(TCP_socket);
    }
}

void Game(int TCP_socket) {
    string number;
    char sendMessage[4] = {};
    char receiveMessage[128] = {};

    while(cin >> number) {
        number.copy(sendMessage, 4);
        int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "Error: Fail to send message to the server." << endl;
        }

        int errR = recv(TCP_socket, receiveMessage, sizeof(receiveMessage), 0);
        if (errR == -1) {
            cout << "Error: Fail to receive message from the server." << endl;
        }
        else if (receiveMessage == "Correct") {
            cout << "You got the answer!" << endl;
            break;
        }
        else if (receiveMessage == "Guess more than five times") {
            cout << "You lose the game!" << endl;
            break;
        }
        else {
            cout << receiveMessage << endl;
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
            if (command.size() != 1){
                cout << "Usage: exit" << endl;
            }
            else {
                Exit(TCP_socket, UDP_socket);
                return 0;
            }
        }
        else if (command[0] == "register") {
            if (command.size() != 4){
                cout << "Usage: register <username> <email> <password>" << endl;
            }
            else {
                Register(UDP_socket, commandInput, serverAddr);
            }
        }
        else if (command[0] == "login") {
            if (command.size() != 3){
                cout << "Usage: login <username> <password>" << endl;
            }
            else {
                Login(TCP_socket, commandInput);
            }
        }
        else if (command[0] == "logout") {
            if (command.size() != 1){
                cout << "Usage: logout" << endl;
            }
            else {
                Logout(TCP_socket);
            }
        }
        else if (command[0] == "game-rule") {
            if (command.size() != 1){
                cout << "Usage: game-rule" << endl;
            }
            else {
                Rule(UDP_socket, serverAddr);
        }
        else if (command[0] == "start-game") {
            Start(TCP_socket, commandInput);
        }
    }
    return 0;
}