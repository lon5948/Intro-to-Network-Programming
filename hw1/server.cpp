#include <iostream>  
#include <vector> 
#include <map> 
#include <time.h>
#include <cstring> 
#include <sstream>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <pthread.h> 

using namespace std;

#define MAX_CLIENT 10

map<string,string> nameMap; // < username , password >
map<string,string> emailMap; // < email , username >

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
    char sendMessage[512] = { "*****Welcome to Game 1A2B*****" };
    int errS = send(TCP_socket, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
}

void Register(int UDP_socket, vector<string>recVec, struct sockaddr_in &clientAddr) {
    string sendBack;
    char sendMessage[512] = {};

    if (recVec.size() != 4) {
        sendBack = "Usage: register <username> <email> <password>";
    }
    else {
        map<string,string>::iterator itName = nameMap.find(recVec[1]);
        map<string,string>::iterator itEmail = emailMap.find(recVec[2]);

        if (itName != nameMap.end()) {
            sendBack = "Username is already used.";
        }
        else if (itEmail != emailMap.end()) {
            sendBack = "Email is already used.";
        }
        else {
            nameMap[recVec[1]] = recVec[3];
            emailMap[recVec[2]] = recVec[1];
            sendBack = "Register successfully.";
        }
    }
    
    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
}

void Rule(int UDP_socket, vector<string>recVec, struct sockaddr_in &clientAddr) {
    string sendBack;
    char sendMessage[512] = {};
    
    if (recVec.size() != 1) {
        sendBack = "Usage: game-rule";
    }
    else {
        sendBack = "1. Each question is a 4-digit secret number.\
        \n2. After each guess, you will get a hint with the following information :\
        \n2.1 The number of \"A\", which are digits in the guess that are in the correct position.\
        \n2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\
        \nThe hint will be formatted as \"xAyB\".\
        \n3. 5 chances for each question.";
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);

    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to send message from the client." << endl;
    }
}

void Exit(int newClient, vector<string>recVecTCP, string user) {
    string sendBack;
    char sendMessage[512] = {};

    if (recVecTCP.size() != 1) {
        sendBack = "Usage: exit";
    }
    else if (user != "") {
        sendBack = { "Please logout first." };
    }
    else {
        close(newClient);
        pthread_exit(0);
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);

    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message from the client." << endl;
    }
}

string Logout(int newClient, vector<string>recVecTCP, string user) {
    char sendMessage[512] = {};
    string sendBack;
    string loginUser = user;

    if (recVecTCP.size() != 1) {
        sendBack = "Usage: logout";
    }
    else if (user == "") {
        sendBack = "Please login first.";
    }
    else {
        loginUser = "";
        sendBack = "Bye, " + user + ".";
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage ,sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message from the client." << endl;
    }

    return loginUser;
}

string Login(int newClient, vector<string> recVecTCP, string user) {
    char sendMessage[512] = {};
    string loginUser = user;
    string sendBack;

    if (recVecTCP.size() != 3) {
        sendBack = "Usage: login <username> <password>";
    }
    else {
        map<string,string>::iterator it = nameMap.find(recVecTCP[1]);

        if (user != "") {
            sendBack = "Please logout first.";
        }
        else if (it == nameMap.end()) {
            sendBack = "Username not found.";
        }
        else if (nameMap[recVecTCP[1]] != recVecTCP[2]) {
            sendBack = "Password not correct.";
        }
        else {
            loginUser = recVecTCP[1];
            sendBack = "Welcome, "+ recVecTCP[1] + ".";
        }
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    return loginUser;
}

bool check4digits(string num) { 
    if (num.length() != 4) return false;

    const char* ch = num.c_str();
    for (int i=0; i<4; i++) {
        if (ch[i]<48 || ch[i]>57) {
            return false;
        }
    }
    return true;
}

void Game(int newClient, string ans) {
    char receiveMessage[512] = {};
    string input, sendBack;
    int chance = 5, A = 0, B = 0;

    while (chance > 0) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage), 0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
        }

        input = receiveMessage;
        bool check = check4digits(input);
        if (!check) {
            sendBack = "Your guess should be a 4-digit number.";
        }
        else if (input == ans) {
            sendBack = "You got the answer!";
            chance = 0;
        }
        else {
            for (int i = 0; i < 4; i++) { 
                for (int j = 0; j < 4; j++) { 
                    if (input[i] == input[j]) {
                        if (i == j) A++;
                        else B++;
                    }
                }
            }
            sendBack = to_string(A) + "A" + to_string(B) +"B";
            if ((--chance) == 0) {
               sendBack += "\nYou lose the game!";
            } 
        }

        int errS = send(newClient, receiveMessage, sizeof(receiveMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
    }
}

void Start(int newClient, vector<string> recVecTCP, string user) {
    string sendBack;
    string ans = "";
    char sendMessage[512] = {};

    if (recVecTCP.size() > 2) {
        sendBack = "Usage: start-game <4-digit number>";
    }
    else if (user == "" ) {
        sendBack = "Please login first.";
    }
    else if (recVecTCP.size() == 2) {
        ans = recVecTCP[1];
        bool check = check4digits(ans);
        if (!check) {
            sendBack = "Usage: start-game <4-digit number>";
        }
        else {
            sendBack = "Please typing a 4-digit number:";
        }
    }
    else if (recVecTCP.size() == 1) {
        srand(time(NULL));
        int randomAnswer = rand() % 10000;
        stringstream ss;
        ss << randomAnswer;
        string str = ss.str();
        ans = str;
        sendBack = "Please typing a 4-digit number:";
    }
    
    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    if (ans != "") {
        Game(newClient, ans);
    }
}

void* Connection(void* data) {
    int newClient = *((int*) data);
    char receiveMessage[512] = {};
    vector<string> recVecTCP;
    string loginUser = "";
    char ans[4];

    while (1) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage),0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }

        recVecTCP = split(receiveMessage);

        if (recVecTCP[0] == "exit") {
            Exit(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "login") {
            loginUser = Login(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "logout") {
            loginUser = Logout(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "start-game") {
            Start(newClient, recVecTCP, loginUser);
        }
        else {
            cout << "[Error] receive command not found." << endl;
        }
    }
}

int main(int argc, char* argv[]) {  

    int serverPort;
    int TCP_socket,UDP_socket;  
    struct sockaddr_in serverAddr;   

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
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }  

    UDP_socket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (UDP_socket == -1) {
        cout << "[Error] Fail to create a UDP socket." << endl;
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
        cout << "[Error] Fail to bind a TCP socket." << endl;
        return 0;
    }
    int errBU = bind(UDP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBU == -1) {
        cout << "[Error] Fail to bind a UDP socket." << endl;
        return 0;
    }
    
    // wait for someone to connect to my port(used by servers)
    int errL = listen(TCP_socket, MAX_CLIENT);  
    if (errL == -1) {
        cout << "[Error] Fail to listen." << endl;
        return 0;
    }
    
    fd_set set;
    FD_ZERO(&set);
    int nfds = max(TCP_socket, UDP_socket) + 1;
    int newClient;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char receiveMessage[512] = {};
    vector<string> recVec;
    pthread_t pid;

    while (1) {
        FD_SET(TCP_socket, &set);
        FD_SET(UDP_socket, &set);
        int errSEL = select(nfds, &set, NULL, NULL, NULL);
        if (errSEL == -1) {
            cout << "[Error] Fail to select." << endl;
            return 0;
        }

        // message is sent by TCP
        if (FD_ISSET(TCP_socket, &set)) {
            newClient = accept(TCP_socket, (struct sockaddr*) &clientAddr, &clientAddrLen);
            cout << "New Connection." << endl;
            Welcome(newClient);
            pthread_create(&pid, NULL, Connection, &newClient);
        }
        
        // message is sent by UDP
        if (FD_ISSET(UDP_socket, &set)) {
            int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage),0, (struct sockaddr*) &clientAddr, &clientAddrLen);
            if (errR == -1) {
                cout << "[Error] Fail to receive message from the client." << endl;
                return 0;
            }
            
            recVec = split(receiveMessage);

            if (recVec[0] == "register") {
                Register(UDP_socket, recVec, clientAddr);
            }
            else if (recVec[0] == "game-rule") {
                Rule(UDP_socket, recVec, clientAddr);
            }
            else {
                cout << "[Error] receive command not found." << endl;
            }
            
            memset(receiveMessage, '\0', sizeof(receiveMessage));
            recVec.clear();
        }
    }
    
    close(TCP_socket);
    close(UDP_socket);
    return 0;
}  