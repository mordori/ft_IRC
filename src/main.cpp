#include <iostream>
#include "Server.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserver <port> <password>\n";
		return 1;
	}
	int	port = std::stoi(argv[1]);
	//int	port = atoi(argv[1]); 
	// stoi is better in general
	// if we want to use atoi, const char* my_c_string = my_string.c_str();
	// int my_integer = atoi(my_c_string); 
	std::string pw = argv[2];
	
	Server mastermind(port, pw);

	// if (!mastermind.setupServer(port, pw)
	// 	|| !mastermind.serverListen()
	// 	|| !mastermind.serverAccept())
	if (!mastermind.setupServer())
	{
		std::cerr << "Failed to setup server.\n";
		return 1;
	}
	std::cout << "Server started successfully. Port: " << port << ".\n";
	mastermind.startServer();
	
	return 0;
}
