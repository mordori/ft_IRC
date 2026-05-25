#include "../inc/Client.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

#include "../inc/Server.hpp"
#include "../inc/Utils.hpp"
#include <iostream>
Client::~Client()
{
	if (_socket != -1)
		close(_socket);
}

void Client::receiveBytes()
{
	std::array<char, 1024> buffer{};
	
	ssize_t bytesReceived{ recv(_socket, buffer.data(), buffer.size(), 0) };
	if (bytesReceived > 0)
	{
		std::span<char> bytes{ buffer.data(), static_cast<std::size_t>(bytesReceived) };
		_bufferIn.append(bytes.data(), bytes.size());
		while (true)
		{
			std::size_t end{ _bufferIn.find("\r\n") };
			if (end == std::string::npos)
				break;
			std::string_view message{ _bufferIn.data(), end };
			_server.handleRequest(*this, message);
			if (this->isDisconnected())
				return;
			_bufferIn.erase(0, end + 2);
		}
	}
	else
		this->setDisconnect(true);
}

// added "\r\n" - IRC messages should terminated with \r\n (CRLF)
void Client::sendMessage(std::string_view message)
{
	_bufferOut.append(message.data(), message.size());
	_bufferOut.append("\r\n");
	_server.modEvents(_socket, EPOLLIN | EPOLLOUT | EPOLLET);
	sendBytes();
}

void Client::sendBytes()
{
	if (_bufferOut.empty())
	{
		_server.modEvents(_socket, EPOLLIN | EPOLLET | EPOLLRDHUP);
		return;
	}
	ssize_t bytesSent{ send(_socket, _bufferOut.data(), _bufferOut.size(), 0) };
	if (bytesSent > 0)
		_bufferOut.erase(0, static_cast<std::size_t>(bytesSent));
	else
	{
		this->setDisconnect(true);
		return;
	}
	if (!_bufferOut.empty())
  		_server.modEvents(_socket, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);
    	else
  		_server.modEvents(_socket, EPOLLIN | EPOLLET | EPOLLRDHUP);
}

// : <servername> <numeric> <nickname> <messages>
void Client::numericReply(std::string_view numeric, std::string_view msg)
{
	std::string nick = _nickname.empty() ? "*" : _nickname;

	std::string numericMsg = std::string(":") + std::string(_server.getHostname()) + " " + std::string(numeric) + " " +
							 std::string(nick) + " " + std::string(msg);

	sendMessage(numericMsg);
}

// IRC User PrefiX: Nickname!username@hostname (e.g., :Alice!alice@example.com PRIVMSG)
std::string Client::getUserPrefix() const { return (":" + _nickname + "!" + _username + "@" + _hostname); }
