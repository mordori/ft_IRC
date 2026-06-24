#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Ping : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		(void)server;

		if (params.empty())
		{
			client.numericReply(IRC::ERR_NOORIGIN, " :No origin specified");
			return;
		}
		std::string token{ params[0] };
		std::string msg{ "PONG :" + token };
		client.sendMessage(msg);
	}
};
