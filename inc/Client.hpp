#pragma once

#include <string>
#include <string_view>

class Server;

class Client
{
private:
	Server& _server;
	int _socket{ -1 };
	std::string _bufferIn;
	std::string _bufferOut;

public:
	Client(Server& server, int socket) : _server{ server }, _socket{ socket } {}
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	~Client();

	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void receiveBytes();
	void sendBytes();
	void sendMessage(std::string_view message);
	[[nodiscard]] int getSocket() const { return _socket; }
};
