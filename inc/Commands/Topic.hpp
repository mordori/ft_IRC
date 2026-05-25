#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "ICommand.hpp"

class Topic : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		(void)client;
		(void)params;
		(void)server;
	}
};
