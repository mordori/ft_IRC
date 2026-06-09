#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <fstream>
#include <signal.h>

#include "Commands/ICommand.hpp"

class Channel;

class Client;

class Server
{
private:
	int _serverSocket{};
	int _epollFd{};
	std::uint16_t _port{};
	std::string _password;
	std::string _hostname;
	std::string _launchTime;
	std::ofstream _logFile;

	std::unordered_map<int, std::unique_ptr<Client>> _clients;
	std::unordered_map<std::string, std::unique_ptr<Channel>> _channels;
	std::unordered_map<std::string_view, std::unique_ptr<ICommand>> _commands;

	static volatile sig_atomic_t	_running;
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
	void log(int logLvl, const std::string& msg);
	void initCommands();
	bool addEvents(int fd, uint32_t events) const;
	bool modEvents(int fd, uint32_t events) const;
	void handleRequest(Client& client, std::string_view message);
	void removeClient(int socket);

	const std::string& getPassword() const { return _password; }
	std::string_view getHostname();
	const std::string& getLaunchTime() const { return _launchTime; }
	bool isNickInUse(std::string_view nick) const;
	void registerClient(Client& client);
	void broadcastToChannels(Client& client, std::string& msg);
	Channel* createChannel(const std::string& name);
	Channel* findChannel(const std::string& name);
	void removeChannel(const std::string& name);
	const std::unordered_map<std::string, std::unique_ptr<Channel>>& getAllChannels() const { return _channels; }
	Client* findClient(const std::string& name);

	static void signalHandler(int sig);


};
