#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Pass : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			client.numericReply(ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
			return;
		}
		if (!client.isRegistered())
		{
			client.numericReply(ERR_ALREADYREGISTERED, ":You may not reregister");
			return;
		}
		if (params[0] != server.getPassword())
		{
			client.numericReply(ERR_PASSWDMISMATCH, ":Password incorrect");
			return;
		}
		client.setPassGiven(true);
	}
};