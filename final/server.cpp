#include <iostream>  
#include <vector> 
#include <map> 
#include <time.h>
#include <cstring> 
#include <string> 
#include <sstream>
#include <algorithm>
#include <queue>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <pthread.h> 

using namespace std;

#define MAX_CLIENT 10

vector<bool> mute_vec;
map<int,int> newClient2user;
map<int,int> user2newClient;
vector<int> allusers;

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

void Mute(int newClient) {
    int usernum = newClient2user[newClient];
    string sendBack = "";
    if (mute_vec[usernum] == true) {
        sendBack = "You are already in mute mode.";
    }
    else {
        sendBack = "Mute mode.";
        mute_vec[usernum] = true;
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
}

void UnMute(int newClient) {
    int usernum = newClient2user[newClient];
    string sendBack = "";
    if (mute_vec[usernum] == false) {
        sendBack = "You are already in unmute mode.";
    }
    else {
        sendBack = "Unmute mode.";
        mute_vec[usernum] = false;
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
}

void Yell(int newClient, vector<string> recVecTCP) {
    int usernum = newClient2user[newClient];
    string sendBack = "user" + to_string(usernum) + ":";
    for (int i = 1; i < recVecTCP.size(); i++) {
        sendBack += " ";
        sendBack += recVecTCP[i];
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    for (int i = 0; i < allusers.size(); i++) {
        if ((allusers[i] != newClient) && (mute_vec[newClient2user[allusers[i]]] == false)) {
            int errS = send(allusers[i], sendMessage, sizeof(sendMessage), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
}

void Tell(int newClient, vector<string> recVecTCP) {
    int usernum = newClient2user[newClient];
    string sendBack = "";
    string receiver = recVecTCP[1];
    int receiver_num = -1;
    bool stoi_flag = true;
    if (receiver.size() > 4) {
        for (int i = 4; i < receiver.size(); i++) {
            if (receiver[i] < int('0') || receiver[i] > int('9')) stoi_flag = false;
        }
        if (stoi_flag) receiver_num = stoi(receiver.erase(0,4));
    }
    if (find(allusers.begin(), allusers.end(), user2newClient[receiver_num]) == allusers.end()) {
        sendBack = recVecTCP[1] + " does not exist.";
    }
    else if (usernum != receiver_num){
        sendBack = "user" + to_string(usernum) + " told you:";
        for (int i = 2; i < recVecTCP.size(); i++) {
            sendBack += " ";
            sendBack += recVecTCP[i];
        }
        if (mute_vec[receiver_num] == false) {
            int len = sendBack.length();
            char sendMessage[len] = {};
            sendBack.copy(sendMessage, len);
            int errS = send(user2newClient[receiver_num], sendMessage, sizeof(sendMessage), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
            return;
        }
        else {
            sendBack = "User is in mute mode.";
        }
    }
    else {
        sendBack = "You can't send message to yourself.";
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
}

void Exit(int newClient) {
    vector<int>::iterator it = find(allusers.begin(), allusers.end(), newClient);
    allusers.erase(it);
    close(newClient);
    pthread_exit(0);
}

void* Connection(void* data) {
    int newClient = *((int*) data);
    char receiveMessage[1024] = {};
    vector<string> recVecTCP;

    while (1) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage),0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }
        else if (errR == 0) {
            Exit(newClient);
        }

        recVecTCP = split(receiveMessage);

        if (recVecTCP[0] == "mute") {
            Mute(newClient);
        }
        else if (recVecTCP[0] == "unmute") {
            UnMute(newClient);
        }
        else if (recVecTCP[0] == "yell") {
            Yell(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "tell") {
            Tell(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "exit") {
            Exit(newClient);
        }
        else {
            cout << "[Error] receive command not found." << endl;
        }
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recVecTCP.clear();
    }
}

int main(int argc, char* argv[]) {  
    int serverPort = stoi(argv[1]);
    int TCP_socket;  
    struct sockaddr_in serverAddr;  
      
    TCP_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (TCP_socket == -1) {
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }  

    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(serverPort);  
    serverAddr.sin_addr.s_addr = INADDR_ANY; 
    
    int flag = 1; 
    int errSOT = setsockopt(TCP_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (errSOT == -1) {
        cout << "[Error] Fail to set socket opt (TCP)." << endl;
        return 0;
    }

    int errBT = bind(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBT == -1) {
        cout << "[Error] Fail to bind a TCP socket." << endl;
        return 0;
    }
    
    int errL = listen(TCP_socket, MAX_CLIENT);  
    if (errL == -1) {
        cout << "[Error] Fail to listen." << endl;
        return 0;
    }
    
    int newClient;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char receiveMessage[1024] = {};
    vector<string> recVec;
    pthread_t pid[MAX_CLIENT];
    int connect[MAX_CLIENT];
    int ind = 0;
    int user_count = 0;
    while (1) {
        newClient = accept(TCP_socket, (struct sockaddr*) &clientAddr, &clientAddrLen);
        connect[ind] = newClient;
        pthread_create(&pid[ind], NULL, Connection, (void* )&connect[ind]);
        ind++;
        string sendBack = "Welcome, user" + to_string(user_count) + ".";
        int len = sendBack.length();
        char sendMessage[len] = {};
        sendBack.copy(sendMessage, len);
        int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
        mute_vec.push_back(false);
        newClient2user[newClient] = user_count; 
        user2newClient[user_count] = newClient;
        allusers.push_back(newClient);
        user_count++;
    }
    close(TCP_socket);
    return 0;
}  