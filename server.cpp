//g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -o server
//g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -o client
#include <string>
#include <iostream>
#include <thread> 
#include <queue> 
#include <list>
#include <vector>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
# include "mensaje.pb.h"

#include "mensaje.pb.h"

using namespace std;
using namespace chat;

// #define PORT 8080 

struct user {
   string username ;
   string ip;
   int userId;
   string status;
   int socket;
};


//int sock = 0;
list <thread> threadList;
vector <int> userIdList;
vector <user> userList;
vector <string> names;

int threadCount = 0;
int PORT;
//Creates socket
int createSocket () {
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    // char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    return new_socket;
}
//Function to send message
void sendBySocket (string msg, int sock) {
    char buffer[msg.size() + 1] = {0};  
    strcpy(buffer, msg.c_str());
    int bytesSen = send (sock, buffer, msg.size() + 1, 0);
    cout << bytesSen << " bytes were send \n";
}
//Optain user by id
user getUser(int id){
    user tempUser = userList[0];
    for (int i = 0; i < userIdList.size(); i++){
        if (userIdList[i] == id) {
            tempUser = userList[i];
        }
    }
    return tempUser;
}
//Get user by username
user getIdUsername(string username){
    user tempUser = userList[0];
    int cont = 0;
    for (int i = 0; i < userList.size(); i++){
        string temp = userList[i].username;
        if (username.compare(temp) == 0) {
            tempUser = userList[i];
        }
    }
    return tempUser;
}
//Get position by ud
int getUserPos(int id){
    int cont = 0;
    for (int i = 0; i < userIdList.size(); i++){
        if (userIdList[i] == id) {
            cont = i;
        }
    }
    return cont;
}
//Check if user name exist 
int checkUserName(string username){
    int cont = -1;
    for (int i = 0; i < userList.size(); i++){
        string temp = userList[i].username;
        if (username.compare(temp) == 0) {
            cont = 1;
        }
    }
    return cont;
}
//Check id username is repeated
int checkUserRepeated(string username){
    int cont = -1;
    for (int i = 0; i < userList.size(); i++){
        string temp = userList[i].username;
        if (username.compare(temp) == 0) {
            cont = cont + 1;
        }
    }
    return cont;
}
//Change status in list
void changeStatusInList(int id, string status){
    user tempUser = userList[0];
     for (int i = 0; i < userIdList.size(); i++){
        if (userIdList[i] == id) {
            tempUser = userList[i];
            tempUser.status = status;
            userList[i] = tempUser;
        }
    }
}
//Optain conected users
void getConnectedUsers(connectedUserRequest cur, int socket){
    ConnectedUserResponse * response(new ConnectedUserResponse);
    int seguir = 1;
    if (cur.userid() == 0){
        //All users
        for (int i = 0; i < userList.size(); i++){
            ConnectedUser * tempConectedUser;
            tempConectedUser = response->add_connectedusers();
            user temporalUser = userList[i]; 

            tempConectedUser->set_userid(temporalUser.userId);
            tempConectedUser->set_username(temporalUser.username);
            tempConectedUser->set_status(temporalUser.status);
            tempConectedUser->set_ip(temporalUser.ip);
        }
    } else {
        
        if (checkUserName(cur.username()) == -1){
            seguir = -1;
        }
        //Single user
        ConnectedUser * tempConectedUser;
        tempConectedUser = response->add_connectedusers();

        user temporalUser = getIdUsername(cur.username()); 

        tempConectedUser->set_userid(temporalUser.userId);
        tempConectedUser->set_username(temporalUser.username);
        tempConectedUser->set_status(temporalUser.status);
        tempConectedUser->set_ip(temporalUser.ip);
    }
    if (seguir == 1) {
        ServerMessage * m(new ServerMessage);
        m->set_option(5); 
        m->set_allocated_connecteduserresponse(response);
        string binary;
        m->SerializeToString(&binary);
        sendBySocket(binary, socket);
        ServerMessage temp;
        temp.ParseFromString(binary);

        cout << "Sending : "<<temp.connecteduserresponse().connectedusers_size() << " users " << "\n";
        for (int i = 0; i < temp.connecteduserresponse().connectedusers_size(); i++) {
            ConnectedUser tmpUser = temp.connecteduserresponse().connectedusers(i);
            cout << "USERNAME: " << tmpUser.username() << endl;
            cout << "STATUS: " << tmpUser.status() << endl;
            cout << "USER ID: " << tmpUser.userid() << endl;
            cout << "IP: " << tmpUser.ip() << endl;
            cout << "----------------------------------" << endl;
        }
    } else {
        //Error
        printf("Non existing user requested\n");
        ErrorResponse * response(new ErrorResponse);

        response->set_errormessage("Non existing user requested");
        ServerMessage * m(new ServerMessage);
        m->set_option(3); 
        m->set_allocated_error(response);
        string binary;
        m->SerializeToString(&binary);
        sendBySocket(binary, socket);
    }
}
//Send broadcast to all users
void sendBroadcast(int id, string message, int socket){ ///FIx broadcast
    //Server response to sender 
    BroadcastResponse * response(new BroadcastResponse);
    response->set_messagestatus("Send");
    ServerMessage * m(new ServerMessage);
    m->set_option(7); 
    m->set_allocated_broadcastresponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, socket);
    //server response to everybody
    BroadcastMessage * globalResponse(new BroadcastMessage);
    globalResponse->set_message(message);
    globalResponse->set_userid(id);
    globalResponse->set_username(getUser(id).username);
    ServerMessage * gM(new ServerMessage);
    gM->set_option(1); 
    gM->set_allocated_broadcast(globalResponse);
    binary;
    gM->SerializeToString(&binary);
    for (int i = 0; i < userList.size(); i++){
        cout <<" Broadcast vuelta no. "<< i;
        user temporalUser = userList[i];
        cout << "Enviando broadcast a: " << temporalUser.username << endl;
        sendBySocket(binary, temporalUser.socket);
    }
    printf("Broadcast was send\n");
}
//Send private message
void sendMessage(string username, int myid , string message, int socket){ 
    //Server response to sender
    if (checkUserName(username) == -1){
        //Error
        printf("Message sended to non existing user \n");
        ErrorResponse * response(new ErrorResponse);

        response->set_errormessage("Message sended to non existing user ");
        ServerMessage * m(new ServerMessage);
        m->set_option(3); 
        m->set_allocated_error(response);
        string binary;
        m->SerializeToString(&binary);
        sendBySocket(binary, socket);
    } else {
        DirectMessageResponse * response(new DirectMessageResponse);
        response->set_messagestatus("Send");
        ServerMessage * m(new ServerMessage);
        m->set_option(8); 
        m->set_allocated_directmessageresponse(response);
        string binary;
        m->SerializeToString(&binary);
        sendBySocket(binary, socket);
        //server response to person
        DirectMessage * directMessage(new DirectMessage);
        directMessage->set_message(message);
        directMessage->set_userid(myid); //fix proto should be int
        directMessage->set_username(getUser(myid).username);
        ServerMessage * pm (new ServerMessage);
        pm->set_option(2); 
        pm->set_allocated_message(directMessage);
        binary = "";
        pm->SerializeToString(&binary);
        user temporalUser = getIdUsername(username);
        sendBySocket(binary, temporalUser.socket);
        printf("Dm was send to %d \n", temporalUser.userId);
    }
}
//Change status of a user
void changeStatus(int id, string status, int socket){ 
    //Server response to sender 
    changeStatusInList(id, status);
    //server response to everybody
    ChangeStatusResponse * changeStatusResponse(new ChangeStatusResponse);
    changeStatusResponse->set_status(status);
    changeStatusResponse->set_userid(id);
    
    ServerMessage * pm (new ServerMessage);
    pm->set_option(6); 
    pm->set_allocated_changestatusresponse(changeStatusResponse);
    string binary;
    pm->SerializeToString(&binary);
    sendBySocket(binary, socket);
    printf("User status was changed \n");
}
//OPtain postition of user
int getPositionOfUser (user usuario) {
    for (int i = 0; i < userList.size(); i++)
    {
        if (userList[i].userId == usuario.userId) return i;
    }
}

//Thread code
void foo(user user, int id ) 
{
    int errorFlag = 0;

    int mySock = user.socket;
    //Get request list 
    int mypos = getUserPos(id);
    
    int acknowledgement = 0;

    int valread; 
    char buffer[8096] = {0};
    
    
    MyInfoResponse * response(new MyInfoResponse);
    response->set_userid(id);
    ServerMessage * m(new ServerMessage);
    m->set_option(4); 
    m->set_allocated_myinforesponse(response);
    string binary;
    m->SerializeToString(&binary);
    sendBySocket(binary, mySock);
    
    printf("%d :Response from server to client send\n", id);
    ClientMessage mr;
    //waiting for acknowledgement
    while(acknowledgement == 0){
        valread = read(mySock, buffer, 8096);
        if ((buffer[0] != '\0') && (valread != 0)) {
            mr.ParseFromString(buffer);
            buffer[8096] = {0}; 
            if (mr.option() == 6) {
                acknowledgement = 1 ;
            } else {
                acknowledgement = -1 ;
            }
        }
        //acknowledgement = 1 ;//Remove later
    }
    if (acknowledgement == -1) {
        errorFlag = 1;
    }
    int answer = checkUserRepeated(user.username);
    
    if (answer != 0 ) {
        errorFlag = 2;
    }
    //Verificar si hay 2 con el mismo nombre
    
    int working = 0;
    //waiting for request from user
    
    if (errorFlag == 0){
        printf("Acknowledgement was recive\n");
        while(working == 0){
            ClientMessage temp;
            valread = read(mySock, buffer, 8096);
            if (buffer[0] != '\0') {
                if (valread != 0) {                
                    temp.ParseFromString(buffer);
                    string prueba = buffer;
                    buffer[8096] = {0}; 
                    switch (temp.option()) {
                        
                        case 2: 
                            getConnectedUsers(temp.connectedusers(), mySock);
                        break;

                        case 3: 
                            changeStatus(user.userId, temp.changestatus().status(), mySock);
                        break;
                        
                        case 4: 
                            sendBroadcast(user.userId, temp.broadcast().message(), mySock);
                        break;
                        
                        case 5:
                            sendMessage(temp.directmessage().username(), id,temp.directmessage().message(), mySock);
                        break;                            
                        default:
                        ;
                    }
                } else {
                    int mypos = getUserPos(id);              
                    close (mySock);
                    cout << "Se desconecto: " << names[mypos] << endl;

                    names[mypos] = names.back();
                    names.pop_back();

                    userIdList[mypos] = userIdList.back();
                    userIdList.pop_back();

                    userList[mypos] = userList.back();
                    userList.pop_back();
                    
                    working = 1; //Sali del while
                    break;
                }
            buffer[8192] = {0}; 
            //Error handling
            }
        }
    } else {
        ErrorResponse * response(new ErrorResponse);
        if (errorFlag == 1){
            printf("Acknowledgement fail\n");

            response->set_errormessage("Acknowledgement fail");
        } else {
            printf("Repeated user\n");

            response->set_errormessage("Repeated user");            
        }
        int mypos = getUserPos(id);
        ServerMessage * m(new ServerMessage);
        m->set_option(3); 
        m->set_allocated_error(response);
        string binary;
        m->SerializeToString(&binary);
        sendBySocket(binary, mySock);

        close (mySock);

        cout << "User disconnected: " << user.username << endl;
        userIdList[mypos] = userIdList.back();
        userIdList.pop_back();
        userList[mypos] = userList.back();
        userList.pop_back();

    }
    //matar todo
}

int main (int argc, char **argv) { 
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //start thread 2
    //thread t2 (thread2);

    PORT = stoi(argv[1]);
    cout << "Running on port: " << PORT << endl;

    while(true){
        //Establish socket 
        int socket = createSocket();
        int flag = 1;
        int valread; 
        char buffer[8096] = {0}; 
        ClientMessage m;

        while(flag){
            valread = read(socket, buffer, 8096);
            if ((buffer[0] != '\0') && (valread != 0)) {
                m.ParseFromString(buffer);
                buffer[8096] = {0}; 
                flag = 0;
            }
        } 

        threadCount ++ ;
        user tempUser ;
        tempUser.username = m.synchronize().username();
        tempUser.ip = m.synchronize().ip();
        tempUser.userId = threadCount;
        tempUser.status = "ACTIVO";
        tempUser.socket = socket;
        
        //threadIdList.push_back(tempUser.userId);
        userIdList.push_back(tempUser.userId);
        userList.push_back(tempUser);
        names.push_back(tempUser.username);

        printf("User created\n");

        threadList.push_back(thread(foo, tempUser, threadCount));
        
        printf("Thread added\n");
        google::protobuf::ShutdownProtobufLibrary();
    }
    //t2.join();
}


