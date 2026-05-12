#pragma once

#include <string_view>
#include <vector>

class Client;
class Server;

class ICommand
{
public:
    ICommand() = default;
	ICommand(const ICommand&) = delete;
	ICommand(ICommand&&) = delete;
	virtual ~ICommand() = default;

	ICommand& operator=(const ICommand&) = delete;
	ICommand& operator=(ICommand&&) = delete;

	virtual void execute(Client& client, Server& server, const std::vector<std::string_view>& params) = 0;
};
