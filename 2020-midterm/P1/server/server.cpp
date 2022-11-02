#include <iostream>  
#include <vector> 
#include <map> 
#include <time.h>
#include <cstring> 
#include <sstream>
#include <fstream>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <dirent.h> 
#include <fcntl.h>


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
    int UDP_socket, serverPort;
    struct sockaddr_in serverAddr, clientAddr;
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

    // create UDP socket
    UDP_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (UDP_socket == -1) {
        cout << "[Error] Fail to create a TCP socket." << endl;
        return 0;
    }

    // set the serverAddr
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // set socket opt
    int flag = 1;
    int errSOU = setsockopt(UDP_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (errSOU == -1) {
        cout << "[Error] Fail to set socket opt. (UDP)" << endl;
        return 0;
    }

    // bind a socket to a local IP address and a port number
    int errBU = bind(UDP_socket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));  
    if (errBU == -1) {
        cout << "[Error] Fail to bind a UDP socket." << endl;
        return 0;
    }

    char sendMessage[16384] = {};
    char receiveMessage[16384] = {};
    vector<string> recVec;
    int len;

    while(1) {
        int errR = recvfrom(UDP_socket, receiveMessage, sizeof(receiveMessage),0, (struct sockaddr*) &clientAddr, &clientAddrLen);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }
            
        recVec = split(receiveMessage);

        if (recVec[0] == "get-file-list") {
            string sendBack = "Files: ";
            DIR *dp = opendir(".");
            struct dirent *filename;
            while (filename = readdir(dp)) {
                sendBack += filename->d_name;
                sendBack += " ";
            }

            len = sendBack.length();
            sendBack.copy(sendMessage, len);
            int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
            if (errS == -1) {
                cout << "[Error] Fail to receive message from the client." << endl;
            }
        }
        else if (recVec[0] == "get-file") {
            string sendBack = "";
            string line;
            for (int i = 1; i < recVec.size(); i++) {
                ifstream file(recVec[i]);
                sendBack = "";
                memset(sendMessage, '\0', sizeof(sendMessage));
                while (getline(file, line)) {
                    if (line.size() && line[line.size() - 1] == '\r' ) {
                        line = line.substr( 0, line.size() - 1);
                    }
                    sendBack += line;
                    sendBack += "\n";
                }
                len = sendBack.length();
                sendBack.copy(sendMessage, len);
                int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
                if (errS == -1) {
                    cout << "[Error] Fail to send message to the client." << endl;
                }
            }
        }
        
        memset(sendMessage, '\0', sizeof(sendMessage));
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recVec.clear();
    }
}