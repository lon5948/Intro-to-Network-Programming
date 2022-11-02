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
map<int, bool> user;
int usernum;

struct struc{
    int newc;
    string ipandport;
    int num;
};

vector<string> split(string str) {
    vector<string> result;
    stringstream ss(str);
    string token;

    while (getline(ss,token,' ')) {
        result.push_back(token);
    }
    return result;
}

void* Connection(void* data) {
    struc inp = *((struc*) data);
    int newClient = inp.newc;
    string ipandport = inp.ipandport;
    int num = inp.num;
    char sendMessage[512] = {};
    char receiveMessage[512] = {};
    vector<string> recVecTCP;
    string loginUser = "";

    while (1) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage),0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }

        recVecTCP = split(receiveMessage);

        string sendBack;
        if (recVecTCP[0] == "list-users") {
            if (recVecTCP.size() != 1) {
                sendBack = "Usage: list-users";
            }
            else {
                map<int, bool>::iterator it;
                sendBack = "1. Success:\n";
                for (it = user.begin(); it != user.end(); it++) {
                    if (it->second) {
                        string s = "user" + it->first + '\n';
                        sendBack += s;
                    }
                }
            }
        }
        else if (recVecTCP[0] == "get-ip") {
            sendBack = "1. Success:\nIP: ";
            sendBack += ipandport;
        }
        else if (recVecTCP[0] == "exit") {
            sendBack = "1. Success:\nBye, user";
            sendBack += num;
        }

        int len = sendBack.length();
        sendBack.copy(sendMessage, len);
        int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }

        memset(sendMessage, '\0', sizeof(sendMessage));
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recVecTCP.clear();
    }
}

int main(int argc, char* argv[]) {
    int serverPort;
    int TCP_socket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // check command format 
    try {
        if (argc!=2) {
            cout << "Usage: ./server <server port>" << endl;
            return 0;
        }
        serverPort = atoi(argv[1]);
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        cout << "Usage: ./server <server port>" << endl;
    }

    // create TCP socket  
    TCP_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (TCP_socket == -1) {
        cout << "[Error] Fail to create a TCP socket." << endl;
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

    // bind a socket to a local IP address and a port number
    int errBT = bind(TCP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBT == -1) {
        cout << "[Error] Fail to bind a TCP socket." << endl;
        return 0;
    }

    // wait for someone to connect to my port(used by servers)
    int errL = listen(TCP_socket, MAX_CLIENT);  
    if (errL == -1) {
        cout << "[Error] Fail to listen." << endl;
        return 0;
    }

    vector<string> recVec;
    pthread_t pid;
    int newClient;
    char sendMessage[512] = {};
    char receiveMessage[512] = {};

    while(1) {
        newClient = accept(TCP_socket, (struct sockaddr*) &clientAddr, &clientAddrLen);
        string tcpIpandPort = sockaddr_in_to_string((sockaddr_in * ) &clientAddr);
        cout << "New connection from " << tcpIpandPort << " user" << usernum << endl;
        user[usernum] = true;
        struc inp;
        inp.newc = newClient;
        inp.ipandport = tcpIpandPort;
        inp.num = usernum;
        pthread_create(&pid, NULL, Connection, (void* )&inp);
    }
}