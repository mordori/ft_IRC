#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Commands/ICommand.hpp"

class Channel
{};

class Client;

class Server
{
private:
	int _serverSocket{};
	int _epollFd{};
	std::uint16_t _port{};
	std::string _password;

	std::unordered_map<int, std::unique_ptr<Client>> _clients;
	std::unordered_map<std::string, Channel*> _channels;
	std::unordered_map<std::string_view, std::unique_ptr<ICommand>> _commands;

public:
	Server(std::uint16_t port, std::string password);
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	~Server();

	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	bool setupServer();	 // socket, bind, listen, epoll_create
	bool serverAccept();
	void startServer();
	void initCommands();
	bool addEvents(int fd, uint32_t events) const;
	bool modEvents(int fd, uint32_t events) const;
	void handleRequest(Client& client, std::string_view message);
	void removeClient(int socket);
};
