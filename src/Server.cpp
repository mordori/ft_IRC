
/* Port # handling:
Each port is numbered from 0 to 65535 and is divided into different groups.
Well-known Ports (0–1023): Reserved for standard services like HTTP (80), HTTPS (443), FTP (21), SSH (22).
Registered Ports (1024–49151): Used by specific applications like databases (MySQL 3306, SQL Server 1433).
Dynamic/Private Ports (49152–65535): Temporarily assigned for client connections, like when your browser opens a port to connect to a website.*/

#include "Server.hpp"

Server::Server(int port, const std::string& password)
	: _serverSocket(-1), _epollFd(-1), _port(port), _password(password) {}

Server::~Server() {} // need to delete client here

// setupServer + serverListen
bool Server::setupServer()
{
	_serverSocker = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		return false;

	// setup Non-blocking
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	int status = fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK);
	if (status == -1)
		return false;

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	int checkbind = bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (checkbind == -1)
		return false;

	int checklisten = listen(_serverSocket, 5);
	if (checklisten == -1)
		return false;

	// replace clientSocket with epoll fd for multiple clients
	_epollFd = epoll_creat1(0);
	if (_epollFd == -1)
		return false;

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _serverSocket;
	int checkEpoll = epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSocket, &ev);
	if (checkEpoll == -1)
		return false;

	return true;	
}

bool	Server::serverAccept() {
	sockaddr_in clientAddr{};
	socklen_t len = sizeof(clientAddr);

	int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &len);
   	if (clientFd == -1) 
		return false;

	// non-blocking

	// epoll
}







/*
bool	Server::setupServer(int portNo, std::string password) {
	(void)portNo, (void)password;
	
	// Setting up server socket. 
	// - Domain defines communication range and format. AF_INET for using IPv4 protocols (another viable option is IPv6)
	// - Type defines communication semantics (how data is sent&received). SOCK_STREAM for using TCP
	// "Please give me a standard TCP connection over IPv4"
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		return false;

	// Defining server address 
	sockaddr_in	serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080); //portNo
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket and address 
	int checkbind = bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (checkbind == -1)
		return false;

	return true;
}

bool	Server::serverListen() {
	// Listen for incoming connections, 5 connections can queue up 
	int checklisten = listen(_serverSocket, 5);
	if (checklisten == -1)
		return false;
	return true;
}

bool	Server::serverAccept() {
	// Accept 1st connection in listen. 
	_clientSocket = accept(_serverSocket, nullptr, nullptr);
	if (_clientSocket == -1)
		return false;
	return true;
}

bool	Server::serverPrint() {
	// Receive data and print out msg
	std::string	clientMsgBuf;
	const size_t bufSize = 10;
	ssize_t	receivedBytes;
	while (1)
	{
		clientMsgBuf.resize(bufSize);
		receivedBytes = recv(_clientSocket, &clientMsgBuf[0], clientMsgBuf.size(), 0);
		if (receivedBytes <= 0)
			break ;
		else
		{
			clientMsgBuf.resize(receivedBytes);
			std::cout << "MSG fr CLIENT: " << clientMsgBuf << std::endl;
		}
	}
	close(_serverSocket);
	if (receivedBytes == -1)
		return false;
	return true;
}




//https://stackoverflow.com/questions/66916835/c-confused-by-epoll-and-socket-fd-on-linux-systems-and-async-threads
// from stackoverflow; 
int epollfd = epoll_create1(0);
if (epollfd == -1) {
  exit(1);
}

struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = listen_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
  exit(2);
}

for (;;) {
  #define MAX_EVENTS 64 
  struct epoll_event events[MAX_EVENTS];
  int events_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);
  if (events_count == -1) {
    exit(3);
  }

  for (int n = 0; n < events_count; ++ n) {
    if (events[n].data.fd == listen_sock) {
      struct sockaddr_un addr;
      socklen_t addrlen;
      int socket = accept(listen_sock, (struct sockaddr *) &addr, &addrlen);
    }
  }
}
*/