#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

#include "../inc/Server.hpp"
#include "../inc/Utils.hpp"

// lsof -i :<port>
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserver <port> <password>\n";
		return 1;
	}
	std::uint16_t port{};
	std::string_view portStr{ argv[1] };
	auto params = std::from_chars(portStr.data(), portStr.data() + portStr.size(), port);
	if (params.ec != std::errc{} || params.ptr != portStr.data() + portStr.size())
	{
		std::cout << '\n' << "Invalid input!" << "\n\n";
		return 1;
	}
	if (port < IRC::PORT_MIN)
	{
		std::cerr << "Port number out of accepted range\n";
		return 1;
	}

	Server mastermind(port, std::string{ argv[2] });
	if (!mastermind.setupServer())
	{
		std::cerr << "Failed to setup server.\n";
		return 1;
	}
	std::cout << "Server started successfully. Port: " << port << ".\n";
	mastermind.serverAccept();
	mastermind.startServer();
	return 0;
}
