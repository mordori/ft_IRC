#include "../inc/Server.hpp"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"
#include "../inc/CommandRequest.hpp"
#include "../inc/Commands/Invite.hpp"
#include "../inc/Commands/Join.hpp"
#include "../inc/Commands/Kick.hpp"
#include "../inc/Commands/Mode.hpp"
#include "../inc/Commands/Nick.hpp"
#include "../inc/Commands/Part.hpp"
#include "../inc/Commands/Pass.hpp"
#include "../inc/Commands/Ping.hpp"
#include "../inc/Commands/Privmsg.hpp"
#include "../inc/Commands/Quit.hpp"
#include "../inc/Commands/Topic.hpp"
#include "../inc/Commands/User.hpp"
#include "../inc/Utils.hpp"

Server::Server(std::uint16_t port, std::string password)
	: _serverSocket{ -1 }
	, _epollFd{ -1 }
	, _port{ port }
	, _password{ std::move(password) }
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

	_launchTime = oss.str();
}

Server::~Server()
{
	if (_serverSocket != -1)
		close(_serverSocket);
	if (_epollFd != -1)
		close(_epollFd);
	if (_logFile.is_open())
		_logFile.close();
}

// setupServer + serverListen
bool Server::setupServer()
{
	//Creating/Opening log file that would replace old session content
	_logFile.open("ServerLog.txt", std::ofstream::trunc);
	if (!_logFile.is_open())
	{
		std::cerr << "Cannot open log file\n";
		return false;
	}

	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		return false;

	const int opt{ 1 };
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	// setup Non-blocking
	int status = fcntl(_serverSocket, F_SETFL, O_NONBLOCK);
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
	if (addEvents(_serverSocket, EPOLLIN))
	{
		log(LOG_INFO, "Server started successfully!");
		return true;
	}
	return false;
}

bool Server::serverAccept()
{
	sockaddr_in clientAddr{};
	socklen_t len = sizeof(clientAddr);

	// added reinterpret_cast to make casting safer and more explicit
	int clientFd = accept(_serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &len);
	if (clientFd == -1)
	{
		log(LOG_ERROR, "Failed to accept connection");
		return false;
	}

	char host[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, host, INET_ADDRSTRLEN) == nullptr)
	{
		log(LOG_ERROR, "Failed to convert Internet address");
		return false;
	}

	int status = fcntl(clientFd, F_SETFL, O_NONBLOCK);
	if (status == -1)
	{
		log(LOG_ERROR, "Failed to setup non-blocking connection");
		return false;
	}

	if (!addEvents(clientFd, EPOLLIN | EPOLLRDHUP))
	{
		log(LOG_ERROR, "Failed to add event for new connection");
		return false;
	}

	_clients[clientFd] = std::make_unique<Client>(*this, clientFd);
	_clients[clientFd]->setHostname(host);
	log(LOG_INFO, "New client added");
	return true;
}

void Server::signalHandler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		_running = 0;
}
volatile sig_atomic_t Server::_running = 1;

void Server::startServer()
{
	signal(SIGINT, Server::signalHandler);
	signal(SIGTERM, Server::signalHandler);

	std::array<struct epoll_event, IRC::EVENT_QUEUE_SIZE> events{};
	while (_running)
	{
		int numEvents = epoll_wait(_epollFd, events.data(), events.size(), -1);
		if (numEvents == -1)
		{
			log(LOG_ERROR, "epoll_wait failed");
			break;
		}
		std::span<struct epoll_event> eventQueue(events.data(), static_cast<std::size_t>(numEvents));
		for (const auto& event : eventQueue)
		{
			int fd = event.data.fd;
			if (event.events & EPOLLIN)
			{
				if (fd == _serverSocket)
				{
					if (!serverAccept())
						log(LOG_WARNING, "Adding new connection failed");
				}
				else if (_clients.contains(fd))
				{
					_clients[fd]->receiveBytes();
				}
			}
			if (event.events & EPOLLOUT)
			{
				if (_clients.contains(fd))
					_clients[fd]->sendBytes();
			}
			if (event.events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				if (_clients.contains(fd))
					_clients[fd]->setDisconnect(true);
			}
			if (_clients.contains(fd) && _clients[fd]->isDisconnected())
				removeClient(fd);
		}
	}
}

void Server::log(int logLvl, const std::string& msg)
{
	std::string type;
	switch (logLvl)
	{
		case LOG_INFO: type = "INFO"; break;
		case LOG_WARNING: type = "WARNING"; break;
		case LOG_ERROR: type = "ERROR"; break;
		case LOG_DEBUG: type = "DEBUG"; break;
		default: type = "OTHER";
	}
	auto now = std::time(nullptr);
	auto local = *std::localtime(&now);

	std::cout << std::put_time(&local, "[%H:%M:%S] ") << "[" << type << "]" << " " << msg << '\n';

	if (_logFile.is_open())
	{
		_logFile << std::put_time(&local, "[%H:%M:%S] ") << "[" << type << "]" << " " << msg << '\n';
		if (msg == "Server started successfully!")
		{
			std::cout << "\tPort: " << _port << " | Password: " << _password << '\n';
			_logFile << "\tPort: " << _port << " | Password: " << _password << '\n';
		}
		if (!_logFile)
		{
			std::cout << "> Failed to write to log file\n"
					<< "> Console logging only\n";
			_logFile.close();
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
	_commands["INVITE"] = std::make_unique<Invite>();
	_commands["JOIN"] = std::make_unique<Join>();
	_commands["KICK"] = std::make_unique<Kick>();
	_commands["MODE"] = std::make_unique<Mode>();
	_commands["NICK"] = std::make_unique<Nick>();
	_commands["PART"] = std::make_unique<Part>();
	_commands["PASS"] = std::make_unique<Pass>();
	_commands["PING"] = std::make_unique<Ping>();
	_commands["PRIVMSG"] = std::make_unique<Privmsg>();
	_commands["QUIT"] = std::make_unique<Quit>();
	_commands["TOPIC"] = std::make_unique<Topic>();
	_commands["USER"] = std::make_unique<User>();
}

void Server::handleRequest(Client& client, std::string_view message)
{
	CommandRequest request{ message };
	if (!request.isValid())
		return;
	auto iter = _commands.find(request.name);
	if (iter != _commands.end())
	{
		iter->second->execute(client, *this, request.params);
	}
	else
	{
		log(LOG_ERROR, client.getNickname() + ": Invalid request");
		client.numericReply(IRC::ERR_UNKNOWNCOMMAND, " :Unknown command");
	}
}

void Server::removeClient(int socket)
{
	if (!_clients.contains(socket))
		return;

	// Remove the socket from the epoll
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket, nullptr);

	// Here inform that the client has left the channel and remove client from the channel and destroy channel if no one left

	auto it = _clients.find(socket);
	Client* client = it->second.get();
	std::unordered_set<Channel*> clientChannels = client->getChannels();
	std::string broadcastMsg = client->getUserPrefix() + " QUIT :Quit: Disconnected";
	if (!clientChannels.empty())
	{
		broadcastToChannels(*client, broadcastMsg);
		for (Channel* channel : clientChannels)
		{
			channel->removeMember(client->getSocket());
			if (channel->getMemberSize() == 0)
				removeChannel(channel->getChannelName());
		}
		client->clearChannels();
	}

	_clients.erase(socket);

	log(LOG_INFO, "Client removed");
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

bool Server::isNickInUse(std::string_view nick) const
{
	for (const auto& pair : _clients)
		if (pair.second->getNickname() == nick)
			return true;
	return false;
}

void Server::registerClient(Client& client) const
{
	if (client.isRegistered())
		return;
	if (!client.getNickname().empty() && !client.getUsername().empty())
	{
		client.setRegistered(true);
		client.numericReply(IRC::RPL_WELCOME, ":Welcome to the IRC Network, " + client.getUserPrefix());
		client.numericReply(
			IRC::RPL_YOURHOST, ":Your host is " + std::string(IRC::SERVER_NAME) + ", running version " + std::string(IRC::SERVER_VERSION));
		client.numericReply(IRC::RPL_CREATED, ":This server was created " + getLaunchTime());
		client.numericReply(IRC::RPL_MYINFO,
			std::string(IRC::SERVER_NAME) + " " + std::string(IRC::SERVER_VERSION) + " " + std::string(IRC::AVAILABLE_USER_MODES) + " " +
				std::string(IRC::AVAILABLE_CHANNEL_MODES));
		client.numericReply(IRC::RPL_ISUPPORT,
			"NICKLEN=" + std::to_string(IRC::NICKLEN) + " USERLEN=" + std::to_string(IRC::USERLEN) +
				" CHANNELLEN=" + std::to_string(IRC::CHANNELLEN) + " :are supported by this server");
	}
}

void Server::broadcastToChannels(Client& client, std::string& msg)
{
	std::unordered_set<int> received;
	for (const auto& [channelName, channelPtr] : _channels)
	{
		if (channelPtr->hasClient(client.getSocket()))
		{
			for (const auto& [memberSocket, memberPtr] : channelPtr->getMembers())
			{
				if (memberSocket != client.getSocket() && !received.contains(memberSocket))
				{
					memberPtr->sendMessage(msg);
					received.insert(memberSocket);
				}
			}
		}
	}
}

Channel* Server::createChannel(const std::string& name)
{
	auto& newChannel = _channels[name];
	newChannel = std::make_unique<Channel>(name);
	return newChannel.get();
}

Channel* Server::findChannel(const std::string& name)
{
	auto it = _channels.find(name);
	if (it != _channels.end())
		return it->second.get();
	return nullptr;
}

void Server::removeChannel(const std::string& name)
{
	_channels.erase(name);
}

Client* Server::findClient(const std::string& name)
{
	for (const auto& [socket, clientPtr] : _clients)
		if (!clientPtr->getNickname().empty() && clientPtr->getNickname() == name)
			return clientPtr.get();
	return nullptr;
}