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

map<string, pair<string, string> > nameMap; // < username , <email,password> >
map<string, string> emailMap; // < email , username >
map<string, int> loginMap; // < username , login or not >
map<string, pair<string, vector< pair<string, int> > > > roomMap; // < roomID, pair<invitation code,vector:members' name> >
map<string, vector< pair<string,string> > > inviteMap; // < username , inviter/roomID >
map<int, pair<string,string> > curStatus;

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
    string sendBack = "";

    if (recVec.size() != 4) {
        sendBack = "Usage: register <username> <email> <password>\n";
    }
    else {
        map<string, pair<string, string> >::iterator itName = nameMap.find(recVec[1]);
        map<string,string>::iterator itEmail = emailMap.find(recVec[2]);

        if (itName != nameMap.end() || itEmail != emailMap.end()) {
            sendBack = "Username or Email is already used\n";
        }
        else {
            nameMap[recVec[1]] = make_pair(recVec[2], recVec[3]);
            emailMap[recVec[2]] = recVec[1];
            loginMap[recVec[1]] = -1;
            sendBack = "Register Successfully\n";
        }
    }
    
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
}

void List_Room(int UDP_socket, struct sockaddr_in &clientAddr) {
    string sendBack = "";

    if (roomMap.size() == 0) {
        sendBack = "No Rooms\n";
    }
    else {
        map<string, pair<string, vector< pair<string,int> > > >::iterator it;
        int index = 1;
        string str1 = "";
        string str2 = "";
        for (it = roomMap.begin(); it != roomMap.end(); it++) {
            if (it->second.second[0].first == "0") str1 = ". (Public) Game Room ";
            else str1 = ". (Private) Game Room ";

            if (it->second.second[0].first == "1") str2 = " is open for players\n";
            else str2 = " has started playing\n";

            sendBack += to_string(index) + str1 + it->first + str2;
            index++;
        }
    }
    
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
}

void List_User(int UDP_socket, struct sockaddr_in &clientAddr) {
    string sendBack = "";

    if (nameMap.size() == 0) {
        sendBack = "No Users\n";
    }
    else {
        map<string, pair<string, string> >::iterator it;
        int index = 1;
        string str = "";
        for (it = nameMap.begin(); it != nameMap.end(); it++) {
            string name = it->first;
            string email = "<" + it->second.first + ">";
            if (loginMap[name] != -1) str = " Online";
            else str = " Offline";
            sendBack += to_string(index) + ".\n" + email + str;
            index++;
        }
    }
    
    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = sendto(UDP_socket, sendMessage, sizeof(sendMessage), 0, (const struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if (errS == -1) {
        cout << "[Error] Fail to receive message from the client." << endl;
    }
}

void Login(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    if (recVecTCP.size() != 3) {
        sendBack = "Usage: login <username> <password>\n";
    }
    else {
        map<string, pair<string, string> >::iterator it = nameMap.find(recVecTCP[1]);

        if (it == nameMap.end()) {
            sendBack = "Username does not exist\n";
        }
        else if (user != "") {
            sendBack = "You already logged in as " + user + "\n";
        }
        else if (loginMap[recVecTCP[1]] != -1) {
            sendBack = "Someone already logged in as " + recVecTCP[1] + "\n";
        }
        else if (nameMap[recVecTCP[1]].second != recVecTCP[2]) {
            sendBack = "Wrong password\n";
        }
        else {
            loginMap[recVecTCP[1]] = newClient;
            sendBack = "Welcome, " + recVecTCP[1] + "\n";
            curStatus[newClient].first = recVecTCP[1];
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

void Logout(int newClient, vector<string>recVecTCP) {
    char sendMessage[512] = {};
    string sendBack;
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;

    if (recVecTCP.size() != 1) {
        sendBack = "Usage: logout\n";
    }
    else if (user == "") {
        sendBack = "You are not logged in\n";
    }
    else if (room != "") {
        sendBack = "You are already in game room " + room + ", please leave game room\n";
    }
    else {
        loginMap[user] = -1;
        curStatus[newClient].first = "";
        sendBack = "Goodbye, " + user + "\n";
    }

    int len = sendBack.length();
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage ,sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message from the client." << endl;
    }
}

void Create_Public_Room(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;

    if (recVecTCP.size() != 4) {
        sendBack = "Usage: create public room <game room id>\n";
    }
    else {
        map<string, pair<string, vector< pair<string, int> > > >::iterator it = roomMap.find(recVecTCP[3]);

        if (user == "") {
            sendBack = "You are not logged in\n";
        }
        else if (room != "") {
            sendBack = "You are already in game room " + room + ", please leave game room\n";
        }
        else if (it != roomMap.end()) {
            sendBack = "Game room ID is used, choose another one\n";
        }
        else {
            sendBack = "You create public game room " + recVecTCP[3] + "\n";
            vector< pair<string,int> > member;
            member.push_back(make_pair("0", 0));
            member.push_back(make_pair(user, newClient));
            roomMap.insert(make_pair(recVecTCP[3], make_pair("" ,member)));
            curStatus[newClient].second = recVecTCP[3];
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

void Create_Private_Room(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;

    if (recVecTCP.size() != 5) {
        sendBack = "Usage: create private room <game_room_id> <invitation code>\n";
    }
    else {
        map<string, pair<string, vector< pair<string,int> > > >::iterator it = roomMap.find(recVecTCP[3]);

        if (user == "") {
            sendBack = "You are not logged in\n";
        }
        else if (room != "") {
            sendBack = "You are already in game room " + room + ", please leave game room\n";
        }
        else if (it != roomMap.end()) {
            sendBack = "Game room ID is used, choose another one\n";
        }
        else {
            sendBack = "You create private game room " + recVecTCP[3] + "\n";
            vector< pair<string,int> > member;
            member.push_back(make_pair("0", 0));
            member.push_back(make_pair(user, newClient));
            roomMap.insert(make_pair(recVecTCP[3], make_pair(recVecTCP[4] ,member)));
            curStatus[newClient].second = recVecTCP[3];
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

void Join_Room(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;
    bool success = false;

    if (recVecTCP.size() != 3) {
        sendBack = "Usage: join room <game room id>\n";
    }
    else {
        map<string, pair<string, vector< pair<string,int> > > >::iterator it = roomMap.find(recVecTCP[3]);

        if (user == "") {
            sendBack = "You are not logged in\n";
        }
        else if (room != "") {
            sendBack = "You are already in game room " + room + ", please leave game room\n";
        }
        else if (it == roomMap.end()) {
            sendBack = "Game room " + recVecTCP[2] + " is not exist\n";
        }
        else if (it->second.first != "") {
            sendBack = "Game room is private, please join game by invitation code\n";
        }
        else if (it->second.second[0].first == "1") {
            sendBack = "Game has started, you can't join now\n";
        }
        else {
            sendBack = "You join game room " + recVecTCP[2] + "\n";
            roomMap[recVecTCP[2]].second.push_back(make_pair(user, newClient));
            curStatus[newClient].second = recVecTCP[3];
            success = true;
        }
    }

    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    if (success) {
        for (int i = 1; i < roomMap[recVecTCP[2]].second.size(); i++) {
            string str = "Welcome, " + user + " to game!\n";
            int l = str.length();
            int member_socket = roomMap[recVecTCP[2]].second[i].second;
            char send_to_other_buffer[l] = {};
            str.copy(send_to_other_buffer, l);
            int errS = send(member_socket, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
}

void Invite(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;
    bool success = false;
    string invitee_name = "";
    string invitee_email = "";
    int invitee_status = -1;

    if (recVecTCP.size() != 2) {
        sendBack = "Usage: invite <invitee email>\n";
    }
    else {
        map<string, pair<string, vector< pair<string,int> > > >::iterator it = roomMap.find(room);
        invitee_name = emailMap[recVecTCP[1]];
        invitee_status = loginMap[invitee_name];
        if (user == "") {
            sendBack = "You are not logged in\n";
        }
        else if (room == "") {
            sendBack = "You did not join any game room\n";
        }
        else if (user != it->second.second[1].first) {
            sendBack = "You are not private game room manager\n";
        }
        else if (invitee_status == -1) {
            sendBack = "Invitee not logged in\n";
        }
        else {
            map<string, vector< pair<string, string> > >::iterator it = inviteMap.find(invitee_name);
            if (it != inviteMap.end()) {
                inviteMap[invitee_name].push_back(make_pair(user, room));
            }
            else {
                vector< pair<string, string> > vec;
                vec.push_back(make_pair(user, room));
                inviteMap.insert(make_pair(invitee_name, vec));
            }
            invitee_email = "<" + recVecTCP[1] + ">";
            sendBack = "You send invitation to " + invitee_name + invitee_email + "\n";
            success = true;
        }
    }

    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    if (success) {
        string inviter_name = user;
        string inviter_email = "<" + nameMap[user].first + ">";
        string str = "You receive invitation from " + inviter_name + inviter_email + "\n";
        int l = str.length();
        char send_to_other_buffer[l] = {};
        str.copy(send_to_other_buffer, l);
        int errS = send(invitee_status, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
    }
}

void List_Invitation(int newClient) {
    string sendBack = "";
    string user = curStatus[newClient].first;

    if (user == "") {
        sendBack = "You are not logged in\n";
    }
    else {
        map<string, vector< pair<string,string> > >::iterator it = inviteMap.find(user);
        if (it == inviteMap.end()){
            sendBack = "No Invitations\n";
        }
        else {
            for (int i = 0; i < inviteMap[user].size(); i++) {
                string inviter_name = inviteMap[user][i].first;
                string inviter_email = "<" + nameMap[inviter_name].first + ">";
                string roomID = inviteMap[user][i].second;
                string invitation_code = roomMap[roomID].first;
                sendBack += to_string(i+1) + ".\n" + inviter_name + inviter_email + \
                " invite you to join game room " + roomID + ", invitation code is " + \
                invitation_code + "\n";
            }
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

void Accept(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;
    string rID = "";
    bool success = false;
    bool codeERR = false;
    bool find = false;

    if (recVecTCP.size() != 3) {
        sendBack = "Usage: accept <inviter email> <invitation code>\n";
    }
    else {
        map<string, pair<string, vector< pair<string, int> > > >::iterator it;
        string inp_name = emailMap[recVecTCP[1]];
        for (int i = 0; i < inviteMap[user].size(); ++i) {
            string roomID = inviteMap[user][i].second;
            string inviter_name = inviteMap[user][i].first;
            if (inviter_name == inp_name) {
                it = roomMap.find(roomID);
                if (it == roomMap.end()) {
                    continue;
                }
                else if (roomMap[roomID].first != recVecTCP[2]) {
                    codeERR = true;
                    find = false;
                }
                else {
                    rID = roomID;
                    find = true;
                    break;
                } 
            }
        }

        if (user == "") {
            sendBack = "You are not logged in\n";
        }
        else if (room != "") {
            sendBack = "You are already in game room " + room + ", please leave game room\n";
        }
        else if ((!find && !codeERR)) {
            sendBack = "Invitation not exist\n";
        }
        else if (!find && codeERR) {
            sendBack = "Invitation code is incorrect\n";
        }
        else if (roomMap[rID].second[0].first == "1") {
            sendBack = "Game has started, you can't join now\n";
        }
        else {
            sendBack = "You join game room " + rID + "\n";
            roomMap[recVecTCP[2]].second.push_back(make_pair(user, newClient));
            curStatus[newClient].second = rID;
            success = true;
        }
    }

    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    if (success) {
        for (int i = 1; i < roomMap[rID].second.size(); i++) {
            string str = "Welcome, " + user + " to game!\n";
            int l = str.length();
            int member_socket = roomMap[rID].second[i].second;
            char send_to_other_buffer[l] = {};
            str.copy(send_to_other_buffer, l);
            int errS = send(member_socket, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
}

void Leave_Room(int newClient) {
    string sendBack = "";
    string sb2other = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;

    if (user == "") {
        sendBack = "You are not logged in\n";
    }
    else if (room == "") {
        sendBack = "You did not join any game room\n";
    }
    else if (user == roomMap[room].second[1].first) {
        roomMap.erase(room);
        sendBack = "You leave game room " + room + "\n";
        sb2other = "Game room manager leave game room " + room + ", you are forced to leave too\n";
    }
    else if (roomMap[room].second[0].first == "1") {
        roomMap[room].second[0].first == "0";
        vector< pair<string,int> >::iterator it = find(roomMap[room].second.begin(), roomMap[room].second.end(), make_pair(user, newClient));
        roomMap[room].second.erase(it);
        sendBack = "You leave game room " + room + ", game ends\n";
        sb2other = user + " leave game room " + room + ", game ends\n";
    }
    else {
        vector< pair<string,int> >::iterator it = find(roomMap[room].second.begin(), roomMap[room].second.end(), make_pair(user, newClient));
        roomMap[room].second.erase(it);
        sendBack = "You leave game room " + room + "n";
        sb2other = user + " leave game room " + room + "\n";
    }

    int len = sendBack.length();
    char sendMessage[len] = {};
    sendBack.copy(sendMessage, len);
    int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
    if (errS == -1) {
        cout << "[Error] Fail to send message to the client." << endl;
    }

    if (sb2other != "") {
        int l = sb2other.length();
        char send_to_other_buffer[l] = {};
        sb2other.copy(send_to_other_buffer, len);
        int errS = send(newClient, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
    }
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

void StartGame(int newClient, vector<string> recVecTCP) {
    string sendBack;
    string ans = "";
    char sendMessage[1024] = {};
    char receiveMessage[1024] = {};
    string user = curStatus[newClient].first; 

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

void Exit(int newClient) {
    string sendBack;
    char sendMessage[1024] = {};

    close(newClient);
    pthread_exit(0);

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
    curStatus.insert(make_pair(newClient, make_pair("", "")));

    while (1) {
        int errR = recv(newClient, receiveMessage, sizeof(receiveMessage),0);
        if (errR == -1) {
            cout << "[Error] Fail to receive message from the client." << endl;
            return 0;
        }

        recVecTCP = split(receiveMessage);

        if (recVecTCP[0] == "login") {
            Login(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "logout") {
            Logout(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "create" && recVecTCP[1] == "public") {
            Create_Public_Room(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "create" && recVecTCP[1] == "private") {
            Create_Private_Room(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "join") {
            Join_Room(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "invite") {
            Invite(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "list" && recVecTCP[1] == "invitations") {
            List_Invitation(newClient);
        }
        else if (recVecTCP[0] == "accept") {
            Accept(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "leave") {
            Leave_Room(newClient);
        }
        else if (recVecTCP[0] == "start-game") {
            StartGame(newClient, recVecTCP);
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
    pthread_t pid[MAX_CLIENT];
    int connect[MAX_CLIENT];
    int ind = 0;
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
            connect[ind] = newClient;
            pthread_create(&pid[ind], NULL, Connection, (void* )&connect[ind]);
            ind++;
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
            else if (recVec[0] == "list" && recVec[1] == "rooms") {
                List_Room(UDP_socket, clientAddr);
            }
            else if (recVec[0] == "list" && recVec[1] == "rooms") {
                List_User(UDP_socket, clientAddr);
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