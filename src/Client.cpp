#include "../inc/Client.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "../inc/Server.hpp"

Client::~Client()
{
	if (_socket != -1)
		close(_socket);
}

void Client::receiveBytes()
{
	std::array<char, 1024> buffer{};
	while (true)
	{
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
				_bufferIn.erase(0, end + 2);
			}
		}
		else if (bytesReceived == 0)
		{
			_server.removeClient(_socket);
			break;
		}
		else
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				_server.removeClient(_socket);
			break;
		}
	}
}

void Client::sendMessage(std::string_view message)
{
	_bufferOut.append(message.data(), message.size());
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
	while (!_bufferOut.empty())
	{
		ssize_t bytesSent{ send(_socket, _bufferOut.data(), _bufferOut.size(), 0) };
		if (bytesSent > 0)
			_bufferOut.erase(0, static_cast<std::size_t>(bytesSent));
		else if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			_server.removeClient(_socket);
			return;
		}
	}
	_server.modEvents(_socket, EPOLLIN | EPOLLET | EPOLLRDHUP);
}
