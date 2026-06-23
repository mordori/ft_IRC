#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class User : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (!client.isPassGiven())
		{
			server.log(LOG_ERROR, "Password incorrect or missing");
			client.numericReply(IRC::ERR_PASSWDMISMATCH, ":Password incorrect or missing");
			return;
		}
		if (params.size() < 4)
		{
			server.log(LOG_WARNING, client.getNickname() + ": Not enough parameters for USER command");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
			return;
		}
		if (client.isRegistered())
		{
			server.log(LOG_WARNING, client.getNickname() + ": Reregistering user attempt");
			client.numericReply(IRC::ERR_ALREADYREGISTERED, ":You may not reregister");
			return;
		}
		std::string username = std::string(params[0]);
		std::string_view realname = params[3];
		if (!username.empty() && username[0] != '~')
			username = "~" + username;
		if (username.length() > IRC::USERLEN)
			username.resize(IRC::USERLEN);
		client.setUsername(username);
		client.setRealname(realname);
		server.registerClient(client);
		server.log(LOG_INFO, client.getNickname() + ": Set up user info");
	}
};
