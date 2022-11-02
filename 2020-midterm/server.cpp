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

    char sendMessage[512] = {};
    char receiveMessage[512] = {};
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
            struct dirent **nameList;
            int fileNum;
            fileNum = scandir(".", &nameList, 0, alphasort);
            for (int i = 0; i < fileNum ; i++) {
                if (nameList[i]->d_type == DT_REG) {
                    sendBack += nameList[i]->d_name;
                    sendBack += " ";
                }
            }

            len = sendBack.length();
            sendBack.copy(sendMessage, len);
            int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
            if (errS == -1) {
                cout << "[Error] Fail to receive message from the client." << endl;
            }
        }
        else if (recVec[0] == "get-file") {
            for (int i = 1; i < recVec.size(); i++) {
                string filename = recVec[i];
                int filefd = open(filename.c_str(), O_CREAT | O_RDONLY, S_IWUSR | S_IRUSR);
                int read_length;
                char *read_buffer = new char[512];
                while ((read_length = read(filefd, read_buffer, 512))) {
                    char *transfer_data = new char[filename.size() + 1 + read_length];
                    memcpy(transfer_data, filename.c_str(), filename.size());
                    memset(transfer_data + filename.size(), ' ', 1);
                    memcpy(transfer_data + filename.size() + 1, read_buffer, read_length);
                    sendto(UDP_socket, transfer_data, filename.size() + 1 + read_length, 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
                    delete[] transfer_data;
                }
                len = filename.length();
                filename.copy(sendMessage, len);
                int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
                if (errS == -1) {
                    cout << "[Error] Fail to receive message from the client." << endl;
                }
                delete[] read_buffer;
                close(filefd);
            }
        }
        
        memset(sendMessage, '\0', sizeof(sendMessage));
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recVec.clear();
    }
}