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

#define MAX_CLIENT 4

long long ACCOUNT1 = 0;
long long ACCOUNT2 = 0;

struct client_data {
    int newc;
    string ipandport;
    string user;
};

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

void Show(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    if (recVecTCP.size()!=1) {
        sendBack = "Usage: show-accounts";
    }
    else {
        sendBack = "ACCOUNT1: " + to_string(ACCOUNT1) + "\nACCOUNT2: " + to_string(ACCOUNT2);
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
}

void Deposit(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    if (recVecTCP.size() != 3) {
        sendBack = "Usage: deposit <account> <money>";
    }
    else {
        long long money = stoi(recVecTCP[2]);
        if (money <= 0) {
            sendBack = "Deposit a non-positive number into accounts.";
        }
        else {
            if (recVecTCP[1] == "ACCOUNT1") ACCOUNT1 += money;
            else if (recVecTCP[1] == "ACCOUNT2") ACCOUNT2 += money;
            sendBack = "Successfully deposits " + recVecTCP[2] + " into " + recVecTCP[1] + ".";
        }
    }
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }
}

void Withdraw(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    if (recVecTCP.size() != 3) {
        sendBack = "Usage: withdraw <account> <money>";
    }
    else {
        long long money = stoi(recVecTCP[2]);
        long long ACCOUNT;
        if (recVecTCP[1] == "ACCOUNT1") ACCOUNT = ACCOUNT1;
        else if (recVecTCP[1] == "ACCOUNT2") ACCOUNT = ACCOUNT2;

        if (money <= 0) {
            sendBack = "Withdraw a non-positive number into accounts.";
        }
        else if (money > ACCOUNT) {
            sendBack = "Withdraw excess money from accounts.";
        }
        else {
            if (recVecTCP[1] == "ACCOUNT1") ACCOUNT1 -= money;
            else if (recVecTCP[1] == "ACCOUNT2") ACCOUNT2 -= money;
            sendBack = "Successfully withdraws " + recVecTCP[2] + " from " + recVecTCP[1] + ".";
        }
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
    close(newClient);
    pthread_exit(0);
}

void* Connection(void* data_inp) {
    client_data inp = *((client_data*) data_inp);
    int newClient = inp.newc;
    string ipandport = inp.ipandport;
    string USER = inp.user;
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

        if (recVecTCP[0] == "show-accounts") {
            Show(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "deposit") {
            Deposit(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "withdraw") {
            Withdraw(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "exit") {
            cout << "user" << USER << " " << ipandport << " disconnected" << endl;
            Exit(newClient);
        }
        else {
            string sendBack = "Usage: \n1. show-accounts\n2. deposit <account> <money>\n3. withdraw <account> <money>\n4. exit";
            int len = sendBack.length();
            char sendMessage[len] = {};
            sendBack.copy(sendMessage, len);
            int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
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
    while(1) {
        newClient = accept(TCP_socket, (struct sockaddr*) &clientAddr, &clientAddrLen);
        stringstream ss;
        ss << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port);
        string tcpIpandPort = ss.str();
        string USER = "";
        switch (ind) {
            case 0:
                USER = "A";
                break;
            case 1:
                USER = "B";
                break;
            case 2:
                USER = "C";
                break;
            case 3:
                USER = "D";
                break;
            default:
                USER = "X";
                break;
        }
        cout << "New connection from " << tcpIpandPort << " user" << USER << endl;
        connect[ind] = newClient;
        client_data data;
        data.newc = connect[ind];
        data.ipandport = tcpIpandPort;
        data.user = USER;
        pthread_create(&pid[ind], NULL, Connection, (void* )&data);
        ind++;
    }
    close(TCP_socket);
    return 0;
}  