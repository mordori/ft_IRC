#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Pass : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			server.log(LOG_WARNING, "Not enough parameters");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
			return;
		}
		if (client.isRegistered())
		{
			server.log(LOG_WARNING, "Reregister attempt");
			client.numericReply(IRC::ERR_ALREADYREGISTERED, ":You may not reregister");
			return;
		}
		if (params[0] != server.getPassword())
		{
			server.log(LOG_ERROR, "Password incorrect");
			client.numericReply(IRC::ERR_PASSWDMISMATCH, ":Password incorrect");
			return;
		}
		client.setPassGiven(true);
	}
};
