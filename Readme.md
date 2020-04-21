# Chat

The chat was made for the course Operating System. The idea was to establish a connection between a client and a server within sockets, and using threads. 

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

You must have installed the protocol buffer compiler of C++ in order to compile the protobuf, although, you can use the one on this repo.

```
https://github.com/protocolbuffers/protobuf
```

### Using

You can fork or clone this repo, to get all the files. 

To compile the server file:

```
g++ server.cpp mensaje.pb.cc -lprotobuf -lpthread -o server
```

To compile the client file:

```
g++ client.cpp mensaje.pb.cc -lprotobuf -lpthread -o client
```

To run server file, where PORT is the port where you want your clients to connect:

```
./server <PORT>
```

To run client file, where username is any username you want, IP is de ip of your server and PORT is the port where your server is running:

```
./client <username> <IP> <PORT>
```

## Built With

* C++
* Google Protobuf

## Authors

* **Javier Carpio** - [javiercarpio57](https://github.com/javiercarpio57)
* **Paul Belches** - [paulbelches](https://github.com/paulbelches)
* **Guillermo Sandoval** - [GuilleLink](https://github.com/GuilleLink)
