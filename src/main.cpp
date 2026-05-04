#include <iostream>

#include "Server.hpp"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserver <port> <password>\n";
		return 1;
	}
	return 0;
}
