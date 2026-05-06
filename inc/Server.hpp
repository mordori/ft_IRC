#pragma once

#include <iostream>
#include <string>
#include <cstring> // memset
#include <unordered_map>
// #include <vector>
#include <arpa/inet.h> // inet_ntop
#include <fcntl.h>
#include <sys/socket.h>	//for socket
#include <netinet/in.h> //for socket address
#include <unistd.h> //for close()

class Client
{
    public:
        int _clientFd;

        Client(int fd) : _clientFd(fd) {}
        ~Client() { close(fd); }
};

class Channel {};

class Server
{
    private:
		int	_serverSocket;
		int _epollFd;
        int _port;
        std::string _password;

        std::unordered_map<int, Client*> clients;
        std::unordered_map<std::string, Channel*> channels;

        Server() = delete;
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

	public:
        Server(int port, const std::string& password);
        ~Server();

    	bool	setupServer(); // socket, bind, listen, epoll_create
        bool    serverAccept();
		void    startServer();

};
