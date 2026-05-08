#include <iostream>
#include <charconv>

#include "Server.hpp"

// lsof -i :<port>
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserver <port> <password>\n";
		return 1;
	}
	uint16_t port{};
	char* endptr{ argv[1] + strlen(argv[1]) };
	auto params = std::from_chars(argv[1], endptr, port);
	if (params.ec != std::errc{} || params.ptr != endptr)
	{
		std::cout << '\n' << "Invalid input!" << "\n\n";
		return 1;
	}
	if (port < 1024)
	{
		std::cerr << "Port number out of accepted range\n";
		return 1;
	}
	std::string pw = argv[2];
	
	Server mastermind(port, pw);

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
