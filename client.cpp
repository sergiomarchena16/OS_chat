# include <iostream>
# include <cstdlib>
# include <pthread.h>
# include <unistd.h>
# include <string>
# include <bits/stdc++.h> 
# include <sys/socket.h> 
# include <arpa/inet.h> 
# include "mensaje.pb.h"
# include <chrono>
# include <stdlib.h>

# define RESET   "\033[0m"
# define BLACK   "\033[30m"      // Black 
# define RED     "\033[31m"      // Red 
# define GREEN   "\033[32m"      // Green 
# define YELLOW  "\033[33m"      // Yellow 
# define BLUE    "\033[34m"      // Blue 
# define MAGENTA "\033[35m"      // Magenta 
# define CYAN    "\033[36m"      // Cyan 
# define WHITE   "\033[37m"      // White 
# define BOLDBLACK   "\033[1m\033[30m"      // Bold Black 
# define BOLDRED     "\033[1m\033[31m"      // Bold Red 
# define BOLDGREEN   "\033[1m\033[32m"      // Bold Green 
# define BOLDYELLOW  "\033[1m\033[33m"      // Bold Yellow 
# define BOLDBLUE    "\033[1m\033[34m"      // Bold Blue 
# define BOLDMAGENTA "\033[1m\033[35m"      // Bold Magenta 
# define BOLDCYAN    "\033[1m\033[36m"      // Bold Cyan 
# define BOLDWHITE   "\033[1m\033[37m"      // Bold White 

#define inactivoT 20

// Declaracion de namespace.
using namespace chat;
using namespace std;
using namespace chrono;

// Definicion de funciones para su uso.
string getFirstWord (string phrase);
string getMessageFromPhrase (string phrase, string toErase);
void *showInfo ();
void *broadcastMessage (string message);
void *cambiarEstado (string nuevoEstado);
bool ifUsername (string word);
void *getUserInfo (string username);
void *sendMessageToUser (string username, string message);
void *exit();
void *sendBySocket(string msgToServer);
void *getAllUsers();

// Declaracion de variables globales.
string input;
int seg;
string estadoActual;
int sock = 0;
bool isAlive = true;
bool askChangeStatus = false;
bool hasConnected = false;
int userId;
string myUsername;

// Funcion que ejecuta un thread para escuchar todo aquello
// que el server mande al usuario.
void *listen (void *args) {
    int valread;

    while (isAlive) {
        char buffer[1024] = {0}; 
        valread = read(sock, buffer, 8096);
        if (buffer[0] != '\0') {
            if (valread != 0) {
                ServerMessage serverMessage;
                serverMessage.ParseFromString (buffer);

                switch (serverMessage.option()) {
                case 1: {
                    string message = serverMessage.broadcast().message();
                    string u = serverMessage.broadcast().username();
                    int id = serverMessage.broadcast().userid();

                    if (u != myUsername) cout << BOLDCYAN << "([" << id << "]) " << u << ": " << RESET << BOLDGREEN << message << RESET << endl;
                    break;
                }
                case 2: {
                    string message = serverMessage.message().message();
                    string u = serverMessage.message().username();
                    int id = serverMessage.message().userid();

                    if (u != myUsername) cout << BOLDBLUE << "([" << id << "] " << u << " en privado): " << RESET << BOLDGREEN << message << RESET << endl;
                    break;
                }
                case 3: {
                    string error = serverMessage.error().errormessage();

                    cout << BOLDRED << "ERROR: " << error << RESET << endl;
                    break;
                }
                case 4: {
                    userId = serverMessage.myinforesponse().userid();

                    MyInfoAcknowledge *myInfoAcknowledge = new MyInfoAcknowledge;
                    myInfoAcknowledge -> set_userid(userId);

                    ClientMessage clientMessage;
                    clientMessage.set_option (6);
                    clientMessage.set_allocated_acknowledge (myInfoAcknowledge);

                    string msgToServer;
                    clientMessage.SerializeToString (&msgToServer);
                    sendBySocket (msgToServer);

                    cout << BOLDGREEN << "Conectado :) \n" << RESET << endl;

                    hasConnected = true;
                    break;
                }
                case 5: {
                    cout << BOLDBLUE << "Los usuarios conectados son: " << RESET << endl;
                    for (int i = 0; i < serverMessage.connecteduserresponse().connectedusers_size(); i++) {
                        ConnectedUser tmpUser = serverMessage.connecteduserresponse().connectedusers(i);
                        cout << BOLDBLUE << "----------------------------------" << RESET << endl;
                        cout << BOLDBLUE << "\tUSERNAME: " << tmpUser.username() << RESET << endl;
                        cout << BOLDBLUE << "\tSTATUS: " << tmpUser.status() << RESET << endl;
                        cout << BOLDBLUE << "\tUSER ID: " << tmpUser.userid() << RESET << endl;
                        cout << BOLDBLUE << "\tIP: " << tmpUser.ip() << RESET << endl;
                        cout << BOLDBLUE << "----------------------------------" << RESET << endl;
                    }
                    break;
                }
                case 6: {
                    int id = serverMessage.changestatusresponse().userid();
                    string status = serverMessage.changestatusresponse().status();

                    if (status == "ACTIVO") {
                        seg = 0;
                        askChangeStatus = false;
                    }

                    cout << BOLDYELLOW << "Estado nuevo: " << status << RESET << endl;
                    estadoActual = status;            
                    break;
                }
                case 7: {
                    string messageStatus = serverMessage.broadcastresponse().messagestatus();

                    cout << BOLDYELLOW << "Broadcast message: " << messageStatus << RESET << endl;
                    askChangeStatus = false;
                    break;
                }
                case 8: {
                    string messageStatus = serverMessage.directmessageresponse().messagestatus();

                    cout << BOLDYELLOW << "Direct message: " << messageStatus << RESET << endl;
                    break;
                }
                default:
                    break;
                }
            } else {
                cout << BOLDRED << "\nDesconexion con el server :(" << endl;

                exit();
            }
        }
    }

    cout << "\nTermine de escuchar.\n" << endl;
}

// Funcion que ejecuta thread para interactuar con el usuario
// y mandar al server las operaciones que desee el usuario.
void *user (void *args) {
    cout << BOLDMAGENTA << "\nSi desea terminar el chat, escribe: 'salir'" << RESET << endl;
    cout << BOLDMAGENTA << "Para obtener mas informacion sobre el uso del chat, escribe: 'info'\n" << RESET << endl;

    while (isAlive) {
        getline (cin, input);
        seg = 0;

        string word = getFirstWord (input);

        string message = getMessageFromPhrase (input, word);

        if (word == "info")
            showInfo ();
        else if (word == "broadcast")
            broadcastMessage (message);
        else if (word == "estado")
            cambiarEstado (message);
        else if (word == "salir")
            exit();
        else if (word == "usuarios")
            getAllUsers ();
        else
            if (ifUsername) {
                if (message == "")
                    getUserInfo (word);
                else
                    sendMessageToUser(word, message);
            } else {
                cout << endl;
            }
    }
}

// Funcion que ejecuta thread para verificar el tiempo de inactividad
// de un usuario y cambiarlo automaticamente.
void *checkState(void *args) {
    while (isAlive) {
        if (hasConnected) {
            if (seg < inactivoT) {
                sleep (1);
                seg++;
            } else {
                if ((estadoActual != "INACTIVO") && (!askChangeStatus)) {
                    cambiarEstado("INACTIVO");
                    askChangeStatus = true;
                }
            }
        }
    }
}

// Funcion que manda informacion al server mediante un socket.
void *sendBySocket (string msg) {
    char buffer[msg.size() + 1] = {0};
    strcpy(buffer, msg.c_str());

    int bytesSen = send (sock, buffer, msg.size() + 1, 0);
}

// Funcion que sirve para conectar mediate sockets con el server.
int connectToServer (string nombre, string username, string ip, string puerto) {
    cout << BOLDCYAN << "Intentando conectarme..." << RESET << endl;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        cout << BOLDRED << "\n Socket creation error \n" << RESET << endl;
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(stoi(puerto)); 
       
    char newIP[ip.size() + 1];
    strcpy(newIP, ip.c_str());

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, newIP, &serv_addr.sin_addr)<=0)  
    { 
        cout << BOLDRED << "\nInvalid address/ Address not supported \n" << RESET << endl;
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        cout << BOLDRED << "\nConnection Failed\n" << RESET << endl;
        return -1; 
    }
}

// Funcion para sincronizarse con el servidor y empezar el proceso de acknowledge.
int sendInfoToServer(string nombre, string username, string ip, string puerto) {
    connectToServer (nombre, username, ip, puerto);

    // Enviar al servidor miUsuario.
    MyInfoSynchronize * myInfo(new MyInfoSynchronize);
    myInfo->set_username(username);
    myInfo->set_ip(ip);

    ClientMessage clientMessage;
    clientMessage.set_option (1);
    clientMessage.set_allocated_synchronize(myInfo);
    
    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
    return 1;
}

// Funcion que envia al server un mensaje broadcast.
void *broadcastMessage (string message) {
    // Aqui se envia un mensaje a todos los usuarios
    BroadcastRequest *broadcastMessage = new BroadcastRequest();
    broadcastMessage->set_message (message);

    ClientMessage clientMessage;
    clientMessage.set_option(4);
    clientMessage.set_allocated_broadcast (broadcastMessage);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

// Funcion que cambia el estado del usuario en el server.
void *cambiarEstado (string nuevoEstado) {
    // Aqui se cambia a otro estado
    ChangeStatusRequest *changeStatus = new ChangeStatusRequest();
    changeStatus->set_status (nuevoEstado);

    ClientMessage clientMessage;
    clientMessage.set_option (3);
    clientMessage.set_allocated_changestatus (changeStatus);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

// Funcion que verifica si es un usuario.
bool ifUsername (string word) {
    return true;
}

// Funcion que solicita al server la informacion de un usuario especifico.
void *getUserInfo (string username) {
    cout << BOLDBLUE << "Obteniendo info de " << username << "..." << RESET << endl;
    connectedUserRequest *userRequest = new connectedUserRequest();
    userRequest -> set_userid(userId); // Hay que asignarle un valor para el usuario.
    userRequest -> set_username (username);

    ClientMessage clientMessage;
    clientMessage.set_option (2);
    clientMessage.set_allocated_connectedusers (userRequest);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

// Funcion que manda un mensaje directo a un usuario especifico.
void *sendMessageToUser (string username, string message) {
    DirectMessageRequest *directMessage = new DirectMessageRequest();
    directMessage -> set_message (message);
    directMessage -> set_userid (userId);
    directMessage -> set_username (username);

    ClientMessage clientMessage;
    clientMessage.set_option (5);
    clientMessage.set_allocated_directmessage (directMessage);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);
    sendBySocket (msgToServer);
}

// FUncion que solicita al server todos los usuarios conectados al server.
void *getAllUsers () {
    connectedUserRequest *userRequest = new connectedUserRequest();
    userRequest -> set_userid(0);

    ClientMessage clientMessage;
    clientMessage.set_option (2);
    clientMessage.set_allocated_connectedusers (userRequest);

    string msgToServer;
    clientMessage.SerializeToString (&msgToServer);

    sendBySocket (msgToServer);
}

// Funcion que cierra el programa.
void *exit() {
    // Si se hace alguna accion para salir.
    cout << BOLDCYAN << "Desconectandome..." << RESET << endl;
    isAlive = false;
    exit(0);
}

// Obtiene la primera palabra de 'phrase'
string getFirstWord (string phrase) {
    istringstream ss (phrase);
    string word;
    ss >> word;

    return word;
}

// Obtiene el mensaje de 'phrase'
string getMessageFromPhrase (string phrase, string toErase) {
    size_t pos = phrase.find(toErase);

    if (pos != string::npos) 
        phrase.erase (pos, toErase.length() + 1);

    return phrase;
}

// Funcion que muestra un mensaje de ayuda.
void *showInfo () {
    cout << BOLDYELLOW << "---------------------------------- INFO ----------------------------------" << RESET << endl;
    cout << BOLDGREEN << "Este es un chat creado para el curso de sistemas operativos." << RESET << endl;
    cout << BOLDGREEN "Debes escribir un mensaje utilizando palabras clave para enviar los mensajes correctamente." << RESET << endl;
    cout << BOLDGREEN "Las palabras clave estan encerradas en comillas simples ('')." << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "'info'" << BOLDGREEN << ": para solicitar informacion de como usar el chat." << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "'salir'" << BOLDGREEN << ": para desconectarse del chat." << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "'usuarios'" << BOLDGREEN << ": obtener todos los usuarios conectados." << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "<username>" << BOLDGREEN << ": al ingresar el username de un usuario conectado, puedes ver informacion de el. " << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "<username> <mensaje>" << BOLDGREEN << ": para enviar el <mensaje> al usuario <username>. " << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "'broadcast' <mensaje>" << BOLDGREEN << ": para enviar el <mensaje> a todos los usuarios conectados." << RESET << endl;
    cout << BOLDGREEN "\t" << BOLDYELLOW << "'estado' <nuevo estado>" << BOLDGREEN << ": para cambiar tu estado actual a <nuevo estado>" << RESET << endl;

    cout << BOLDYELLOW << "-------------------------------------------------------------------------" << RESET << endl;
}

int main (int argc, char **argv) {
    if (argc > 3) {

        string username = argv[1];
        string nombre = "";
        myUsername = username;
        string ip = argv[2];
        string puerto = argv[3];

        if (username == "broadcast") {
            cout << BOLDRED << "Tu username no puede ser 'broadcast'." << RESET << endl;
            exit(0);
        } else if (username == "info") {
            cout << BOLDRED << "Tu username no puede ser 'info'." << RESET << endl;
            exit(0);
        } else if (username == "usuarios") {
            cout << BOLDRED << "Tu username no puede ser 'usuarios'." << RESET << endl;
            exit(0);
        } else if (username == "estado") {
            cout << BOLDRED << "Tu username no puede ser 'estado'." << RESET << endl;
            exit(0);
        } else if (username == "salir") {
            cout << BOLDRED << "Tu username no puede ser 'salir'" << RESET << endl;
            exit(0);
        } 

        cout << "Username: " << username << endl;
        cout << "IP: " << ip << endl;
        cout << "puerto: " << puerto << endl;
        seg = 0;

        estadoActual = "ACTIVO";

        cout << "---------------------------------" << endl;

        // Se inicializa el usuario con el server si existe.
        if (sendInfoToServer(nombre, username, ip, puerto) != 0) {
            pthread_t threadListen;
            pthread_t threadUser;
            pthread_t threadState;

            if (pthread_create(&threadListen, NULL, listen, NULL) 
            || pthread_create(&threadUser, NULL, user, NULL) 
            || pthread_create(&threadState, NULL, checkState, NULL)) {
                cout << BOLDRED << "Error: unable to create threads." << RESET << endl;
                exit(-1);
            }

            pthread_join (threadListen, NULL);
            pthread_join (threadUser, NULL);
            pthread_join (threadState, NULL);
        }
    } else {
        cout << BOLDRED << "Faltan parametros para ejecutar el programa." << RESET << endl;
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 1;
}


