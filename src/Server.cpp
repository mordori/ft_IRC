#include "../inc/Server.hpp"

#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "../inc/Client.hpp"
#include "../inc/CommandRequest.hpp"
#include "../inc/Commands/Join.hpp"
#include "../inc/Commands/Nick.hpp"
#include "../inc/Utils.hpp"

Server::Server(std::uint16_t port, std::string password)
	: _serverSocket{ -1 }, _epollFd{ -1 }, _port{ port }, _password{ std::move(password) }
{}

Server::~Server()
{
	if (_serverSocket != -1)
		close(_serverSocket);
	if (_epollFd != -1)
		close(_epollFd);
}

// setupServer + serverListen
bool Server::setupServer()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		return false;

	const int opt{ 1 };
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	// setup Non-blocking
	int flags = fcntl(_serverSocket, F_GETFL, 0);
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

	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
		return false;

	initCommands();
	return addEvents(_serverSocket, EPOLLIN);
}

bool Server::serverAccept()
{
	while (true)
	{
		sockaddr_in clientAddr{};
		socklen_t len = sizeof(clientAddr);
	
		int clientFd = accept(_serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &len); // added reinterpret_cast to make casting safer and more explicit
		if (clientFd == -1)
			return errno == EAGAIN || errno == EWOULDBLOCK;

		char host[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &clientAddr.sin_addr, host, INET_ADDRSTRLEN) == nullptr)
			return false;

		int flags = fcntl(clientFd, F_GETFL, 0);
		if (flags == -1)
			return false;
		int status = fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
		if (status == -1)
			return false;

		if (!addEvents(clientFd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP))
			return false;

		_clients[clientFd] = std::make_unique<Client>(*this, clientFd);
		_clients[clientFd]->setHostname(host);
		std::cout << "New client is added\n";
	}
	return true;
}

void Server::startServer()
{
	std::array<struct epoll_event, IRC::EVENT_QUEUE_SIZE> events{};
	while (true)
	{
		int numEvents = epoll_wait(_epollFd, events.data(), events.size(), -1);
		if (numEvents == -1)
			break;
		std::span<struct epoll_event> eventQueue(events.data(), static_cast<std::size_t>(numEvents));
		for (const auto& event : eventQueue)
		{
			int fd = event.data.fd;
			if (event.events & EPOLLIN)
			{
				if (fd == _serverSocket)
					serverAccept();
				else if (_clients.contains(fd))
					_clients[fd]->receiveBytes();
			}
			if (event.events & EPOLLOUT)
			{
				if (_clients.contains(fd))
					_clients[fd]->sendBytes();
			}
			if (event.events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				removeClient(fd);
				continue;
			}
		}
	}
}

bool Server::addEvents(int fd, uint32_t events) const
{
	struct epoll_event ev{};
	ev.data.fd = fd;
	ev.events = events;
	return epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Server::modEvents(int fd, uint32_t events) const
{
	struct epoll_event ev{};
	ev.data.fd = fd;
	ev.events = events;
	return epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

void Server::initCommands()
{
	_commands["JOIN"] = std::make_unique<Join>();
	_commands["NICK"] = std::make_unique<Nick>();
}

void Server::handleRequest(Client& client, std::string_view message)
{
	CommandRequest request{ message };
	if (!request.isValid())
		return;
	if (auto iter = _commands.find(request.name); iter != _commands.end())
		iter->second->execute(client, *this, request.params);
	else
		;  // TODO: no such command
}

void Server::removeClient(int socket)
{
	// Remove the socket from the epoll
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, nullptr);

	// Here inform that the client has left the channel and remove client from the channel and destroy channel if no one left

	if (_clients.contains(socket))
		_clients.erase(socket);
}

std::string_view Server::getHostname()
{
	if (_hostname.empty())
	{
		char tmp[0x100];

		if (gethostname(tmp, sizeof(tmp)) == 0)
			_hostname = tmp;
		else
			_hostname = "localhost";
	}
	return _hostname;	
}