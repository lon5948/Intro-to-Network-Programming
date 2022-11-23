#include <iostream>  
#include <vector> 
#include <map> 
#include <time.h>
#include <cstring> 
#include <string> 
#include <sstream>
#include <algorithm>
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
map<string,bool> loginMap; // < username , login >
map<string, vector<string>> loginMap; // < roomID , members' name >

vector<string> split(string str) {
    vector<string> result;
    stringstream ss(str);
    string token;

    while (getline(ss,token,' ')) {
        token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
        result.push_back(token);
    }
    return result;
}

void Register(int UDP_socket, vector<string>recVec, struct sockaddr_in &clientAddr) {
    char sendMessage[1024] = {};
    string sendBack = "";

    if (recVec.size() != 4) {
        sendBack = "Usage: register <username> <email> <password>\n";
    }
    else {
        map<string,string>::iterator itName = nameMap.find(recVec[1]);
        map<string,string>::iterator itEmail = emailMap.find(recVec[2]);

        if (itName != nameMap.end() || itEmail != emailMap.end()) {
            sendBack = "Username or Email is already used\n";
        }
        else {
            nameMap[recVec[1]] = recVec[3];
            emailMap[recVec[2]] = recVec[1];
            loginMap[recVec[1]] = false;
            sendBack = "Register Successfully\n";
        }
    }
    
    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
    memset(sendMessage, '\0', sizeof(sendMessage));
}

string Login(int newClient, vector<string> recVecTCP, string user) {
    char sendMessage[1024] = {};
    string sendBack = "";
    string retuser = user;

    if (recVecTCP.size() != 3) {
        sendBack = "Usage: login <username> <password>\n";
    }
    else {
        map<string,string>::iterator it = nameMap.find(recVecTCP[1]);

        if (it == nameMap.end()) {
            sendBack = "Username does not exist\n";
        }
        else if (user != "") {
            sendBack = "You already logged in as " + user + "\n";
        }
        else if (loginMap[recVecTCP[1]]) {
            sendBack = "Someone already logged in as " + recVecTCP[1] + "\n";
        }
        else if (nameMap[recVecTCP[1]] != recVecTCP[2]) {
            sendBack = "Wrong password\n";
        }
        else {
            loginMap[recVecTCP[1]] = true;
            sendBack = "Welcome, " + recVecTCP[1] + "\n";
            retuser = recVecTCP[1];
        }
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
    memset(sendMessage, '\0', sizeof(sendMessage));
    return retuser;
}

string Logout(int newClient, vector<string>recVecTCP, string user) {
    char sendMessage[512] = {};
    string sendBack;
    string loginUser = user;

    if (recVecTCP.size() != 1) {
        sendBack = "Usage: logout\n";
    }
    else if (user == "") {
        sendBack = "You are not logged in\n";
    }
    else {
        loginMap[loginUser] = false;
        loginUser = "";
        sendBack = "Goodbye, " + user + "\n";
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage ,sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message from the client." << endl;
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

void StartGame(int newClient, vector<string> recVecTCP, string user) {
    string sendBack;
    string ans = "";
    char sendMessage[1024] = {};
    char receiveMessage[1024] = {};

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
            ans = "";
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
        string input;
        int chance = 5;
        
        while (chance > 0) {
            int Anum=0, Bnum=0;
            memset(&sendMessage, '\0', sizeof(sendMessage));
            memset(&receiveMessage, '\0', sizeof(receiveMessage));
            vector<int> vecInput(10,0);
            vector<int> vecAns(10,0);

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
                    if (input[i] == ans[i]) {
                        Anum++;
                    }
                    else {
                        vecInput[int(input[i])-int('0')] += 1;
                        vecAns[int(ans[i])-int('0')] += 1;
                    }
                }
                for (int i = 0; i < 10; i++) { 
                    if (vecInput[i] > 0 && vecAns[i] > 0) {
                        Bnum += min(vecInput[i], vecAns[i]);
                    }
                }
                sendBack = to_string(Anum) + "A" + to_string(Bnum) + "B";
                if ((--chance) == 0) {
                    sendBack += "\nYou lose the game!";
                } 
            }

            int len = sendBack.length();
            sendBack.copy(sendMessage, len);
            int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
}

void Exit(int newClient, vector<string>recVecTCP, string user) {
    string sendBack;
    char sendMessage[1024] = {};

    if (recVecTCP.size() != 1) {
        sendBack = "Usage: exit";
    }
    else {
        if (user != "") {
            loginMap[user] = false;
        }
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

void* Connection(void* data) {
    int newClient = *((int*) data);
    char receiveMessage[1024] = {};
    vector<string> recVecTCP;
    string loginUser = "";

    while (1) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage),0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }

        recVecTCP = split(receiveMessage);

        if (recVecTCP[0] == "login") {
            loginUser = Login(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "logout") {
            loginUser = Logout(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "start-game") {
            StartGame(newClient, recVecTCP, loginUser);
        }
        else if (recVecTCP[0] == "exit") {
            Exit(newClient, recVecTCP, loginUser);
        }
        else {
            cout << "[Error] receive command not found." << endl;
        }
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recVecTCP.clear();
    }
}

int main(int argc, char* argv[]) {  
    int serverPort = 8888;
    int TCP_socket,UDP_socket;  
    struct sockaddr_in serverAddr;  
    
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

    // set the serverAddr  
    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = INADDR_ANY; 
    
    // set socket opt
    int flag = 1; 
    int errSOT = setsockopt(TCP_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (errSOT == -1) {
        cout << "[Error] Fail to set socket opt (TCP)." << endl;
        return 0;
    }
    int errSOU = setsockopt(UDP_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (errSOU == -1) {
        cout << "[Error] Fail to set socket opt. (UDP)" << endl;
        return 0;
    }

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
    char receiveMessage[1024] = {};
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
            //cout << "New Connection." << endl;
            pthread_create(&pid, NULL, Connection, (void* )&newClient);
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