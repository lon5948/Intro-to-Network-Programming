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

#define MAX_CLIENT 20

map<string, pair<string, string> > nameMap; // < username , <email,password> >
map<string, string> emailMap; // < email , username >
map<string, int> loginMap; // < username , login or not >
map<string, pair<string, vector< pair<string, int> > > > roomMap; // < roomID, pair<invitation code,vector:members' name> >
map<string, vector< pair<string,string> > > inviteMap; // < username , inviter/roomID >
map<int, pair<string,string> > curStatus;
map<string, string> answerMap;
map<string, pair<int, int> > guessMap; // <roomID, rounds/index>

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
        sendBack = "List Game Rooms\nNo Rooms\n";
    }
    else {
        sendBack = "List Game Rooms\n";
        map<string, pair<string, vector< pair<string,int> > > >::iterator it;
        int index = 1;
        string str1 = "";
        string str2 = "";
        for (it = roomMap.begin(); it != roomMap.end(); it++) {
            if (it->second.first == "") str1 = ". (Public) Game Room ";
            else str1 = ". (Private) Game Room ";

            if (it->second.second[0].first == "0") str2 = " is open for players\n";
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
        sendBack = "List Users\nNo Users\n";
    }
    else {
        sendBack = "List Users\n";
        map<string, pair<string, string> >::iterator it;
        int index = 1;
        string str = "";
        for (it = nameMap.begin(); it != nameMap.end(); it++) {
            string name = it->first;
            string email = "<" + it->second.first + ">";
            if (loginMap[name] != -1) str = " Online\n";
            else str = " Offline\n";
            sendBack += to_string(index) + ". " + name + email + str;
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
    char sendMessage[len] = {};
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
        map<string, pair<string, vector< pair<string,int> > > >::iterator it = roomMap.find(recVecTCP[2]);

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
            curStatus[newClient].second = recVecTCP[2];
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
            if (roomMap[recVecTCP[2]].second[i].first != user) {
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
                vector< pair<string,string> >::iterator vecit = find(inviteMap[invitee_name].begin(), inviteMap[invitee_name].end(), make_pair(user, room));
                if (vecit == inviteMap[invitee_name].end()) {
                    inviteMap[invitee_name].push_back(make_pair(user, room));
                }
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
            sendBack = "List invitations\nNo Invitations\n";
        }
        else {
            sendBack = "List invitations\n";
            priority_queue< pair<string, string> ,vector< pair<string, string> >,greater< pair<string, string> > >pq;
            map<string, pair<string, vector< pair<string, int> > > >::iterator it2;
            int size = inviteMap[user].size();
            for (int i = 0; i < size; i++) {
                string inviter_name = inviteMap[user][i].first;
                string roomID = inviteMap[user][i].second;
                it2 = roomMap.find(roomID);
                
                if (it2 == roomMap.end()) {
                    vector< pair<string,string> >::iterator vecit = find(inviteMap[user].begin(), inviteMap[user].end(), inviteMap[user][i]);
                    inviteMap[user].erase(vecit);
                    size--;
                    i--;
                }
                else {
                    pq.push(make_pair(roomID, inviter_name));
                }
            }

            int index = 1;
            while (!pq.empty()) {
                string inviter_name = pq.top().second;
                string inviter_email = "<" + nameMap[inviter_name].first + ">";
                string roomID = pq.top().first;
                string invitation_code = roomMap[roomID].first;
                sendBack += to_string(index) + ". " + inviter_name + inviter_email + \
                " invite you to join game room " + roomID + ", invitation code is " + \
                invitation_code + "\n";
                index++;
                pq.pop();
            }

            if (sendBack == "List invitations\n") sendBack = "List invitations\nNo Invitations\n";
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
            sendBack = "Your invitation code is incorrect\n";
        }
        else if (roomMap[rID].second[0].first == "1") {
            sendBack = "Game has started, you can't join now\n";
        }
        else {
            sendBack = "You join game room " + rID + "\n";
            roomMap[rID].second.push_back(make_pair(user, newClient));
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
            if (roomMap[rID].second[i].first != user) {
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
}

void Leave_Room(int newClient) {
    string sendBack = "";
    string sb2other = "";
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;
    bool deleteRoom = false;

    if (user == "") {
        sendBack = "You are not logged in\n";
    }
    else if (room == "") {
        sendBack = "You did not join any game room\n";
    }
    else if (user == roomMap[room].second[1].first) {
        for (int i = 1; i < roomMap[room].second.size(); i++) {
            int c = roomMap[room].second[i].second;
            curStatus[c].second = "";
        }
        deleteRoom = true;
        sendBack = "You leave game room " + room + "\n";
        sb2other = "Game room manager leave game room " + room + ", you are forced to leave too\n";
    }
    else if (roomMap[room].second[0].first == "1") {
        roomMap[room].second[0].first == "0";
        vector< pair<string,int> >::iterator it = find(roomMap[room].second.begin(), roomMap[room].second.end(), make_pair(user, newClient));
        roomMap[room].second.erase(it);
        curStatus[newClient].second = "";
        sendBack = "You leave game room " + room + ", game ends\n";
        sb2other = user + " leave game room " + room + ", game ends\n";
    }
    else {
        vector< pair<string,int> >::iterator it = find(roomMap[room].second.begin(), roomMap[room].second.end(), make_pair(user, newClient));
        roomMap[room].second.erase(it);
        curStatus[newClient].second = "";
        sendBack = "You leave game room " + room + "\n";
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
        sb2other.copy(send_to_other_buffer, l);
        for (int i=1; i < roomMap[room].second.size(); i++) {
            if (roomMap[room].second[i].first != user) {
                int errS = send(roomMap[room].second[i].second, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
                if (errS == -1) {
                    cout << "[Error] Fail to send message to the client." << endl;
                }   
            }
        }
    }

    if (deleteRoom) roomMap.erase(room);
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
    string user = curStatus[newClient].first; 
    string room = curStatus[newClient].second;
    bool success = false;
    bool guess = false;

    if (recVecTCP.size() != 3 && recVecTCP.size() != 4) {
        sendBack = "Usage: start game <number of rounds> <guess number>";
    }
    else if (user == "" ) {
        sendBack = "You are not logged in\n";
    }
    else if (room == "") {
        sendBack = "You did not join any game room\n";
    }
    else if (roomMap[room].second[1].first != user) {
        sendBack = "You are not game room manager, you can't start game\n";
    }
    else if (roomMap[room].second[0].first == "1") {
        sendBack = "Game has started, you can't start again\n";
    }
    else if (recVecTCP.size() == 4 && !check4digits(recVecTCP[3])) {
        sendBack = "Please enter 4 digit number with leading zero\n";
    }
    else if (recVecTCP.size() == 3){
        srand(time(NULL));
        int randomAnswer = rand() % 10000;
        stringstream ss;
        ss << randomAnswer;
        string str = ss.str();
        answerMap[room] = str;
        guessMap[room] = make_pair(stoi(recVecTCP[2]), 1);
        roomMap[room].second[0].first = "1";
        success = true;
    }
    else {
        answerMap[room] = recVecTCP[3];
        guessMap[room] = make_pair(stoi(recVecTCP[2]), 1);
        roomMap[room].second[0].first = "1";
        success = true;
    }
    
    if (success) {
        string str = "Game start! Current player is " + user + "\n";
        int l = str.length();
        char send_to_other_buffer[l] = {};
        str.copy(send_to_other_buffer, l);
        for (int i = 1; i < roomMap[room].second.size(); i++) {
            int c = roomMap[room].second[i].second;
            int errS = send(c, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
    else {
        int len = sendBack.length();
        char sendMessage[len] = {};
        sendBack.copy(sendMessage, len);
        int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
    }
}

string GuessResult(string guess, string rID) {
    string ret = "";
    int Anum=0, Bnum=0;
    vector<int> vecInput(10,0);
    vector<int> vecAns(10,0);

    if (guess == answerMap[rID]) {
        ret = "Bingo";
    }
    else {
        for (int i = 0; i < 4; i++) { 
            if (guess[i] == answerMap[rID][i]) {
                Anum++;
            }
            else {
                vecInput[int(guess[i])-int('0')] += 1;
                vecAns[int(answerMap[rID][i])-int('0')] += 1;
            }
        }
        for (int i = 0; i < 10; i++) { 
            if (vecInput[i] > 0 && vecAns[i] > 0) {
                Bnum += min(vecInput[i], vecAns[i]);
            }
        }
        ret = "'" + to_string(Anum) + "A" + to_string(Bnum) + "B'";
    }
    return ret;
}

void Guess(int newClient, vector<string> recVecTCP) {
    string sendBack = "";
    string send2other = "";
    string user = curStatus[newClient].first; 
    string room = curStatus[newClient].second;
    bool success = false;
    bool guess = false;

    if (recVecTCP.size() != 2) {
        sendBack = "Usage: guess <guess number>\n";
    }
    else if (user == "" ) {
        sendBack = "You are not logged in\n";
    }
    else if (room == "") {
        sendBack = "You did not join any game room\n";
    }
    else if (roomMap[room].second[0].first == "0") {
        if (roomMap[room].second[1].first == user) {
            sendBack = "You are game room manager, please start game first\n";
        }
        else {
            sendBack = "Game has not started yet\n";
        }
    }
    else if (user != roomMap[room].second[guessMap[room].second].first) {
        sendBack = "Please wait..., current player is " + \
        roomMap[room].second[guessMap[room].second].first + "\n";
    }
    else if (!check4digits(recVecTCP[1])) {
        sendBack = "Please enter 4 digit number with leading zero\n";
    }
    else {
        string result = GuessResult(recVecTCP[1] ,room);
        guessMap[room].second++;
        if (guessMap[room].second == roomMap[room].second.size()) {
            guessMap[room].second = 1;
            guessMap[room].first--;
        }
        success = true;
        if (result == "Bingo") {
            send2other = user + " guess '" + recVecTCP[1] + "' and got Bingo!!! " + user + \
            " wins the game, game ends\n";
            roomMap[room].second[0].first = "0";
        }
        else if (guessMap[room].first == 0) {
            send2other = user + " guess '" + recVecTCP[1] + "' and got " + result + "\n" \
            + "Game ends, no one wins\n";
            roomMap[room].second[0].first = "0";
        }
        else {
            send2other = user + " guess '" + recVecTCP[1] + "' and got " + result + "\n";
        }
    }
    
    if (success) {
        int l = send2other.length();
        char send_to_other_buffer[l] = {};
        send2other.copy(send_to_other_buffer, l);
        for (int i = 1; i < roomMap[room].second.size(); i++) {
            int c = roomMap[room].second[i].second;
            int errS = send(c, send_to_other_buffer, sizeof(send_to_other_buffer), 0);
            if (errS == -1) {
                cout << "[Error] Fail to send message to the client." << endl;
            }
        }
    }
    else {
        int len = sendBack.length();
        char sendMessage[len] = {};
        sendBack.copy(sendMessage, len);
        int errS = send(newClient, sendMessage, sizeof(sendMessage), 0);
        if (errS == -1) {
            cout << "[Error] Fail to send message to the client." << endl;
        }
    }
}

void Exit(int newClient) {
    string user = curStatus[newClient].first;
    string room = curStatus[newClient].second;

    if (room != "") {
        if (user == roomMap[room].second[1].first) {
            for (int i = 1; i < roomMap[room].second.size(); i++) {
                int c = roomMap[room].second[i].second;
                curStatus[c].second = "";
            }
            roomMap.erase(room);
        }
        else {
            vector< pair<string,int> >::iterator it = find(roomMap[room].second.begin(), roomMap[room].second.end(), make_pair(user, newClient));
            roomMap[room].second.erase(it);
            curStatus[newClient].second = "";
        }
    }
    if (user != "") {
        loginMap[user] = -1;
        curStatus[newClient].first = "";
    }

    close(newClient);
    pthread_exit(0);
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
        else if (errR == 0) {
            Exit(newClient);
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
        else if (recVecTCP[0] == "start") {
            StartGame(newClient, recVecTCP);
        }
        else if (recVecTCP[0] == "guess") {
            Guess(newClient, recVecTCP);
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
            else if (recVec[0] == "list" && recVec[1] == "users") {
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