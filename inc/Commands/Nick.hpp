#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Nick : public ICommand
{
private:
	static bool isNickValid(std::string_view nick)
	{
		if (nick.empty() || nick.length() > 9)
			return false;
		if (std::isdigit(static_cast<unsigned char>(nick[0])))
			return false;
		if (nick[0] == '#' || nick[0] == ':')
			return false;
		const std::string special = "[]{}\\`_-^|";
		for (char c : nick)
		{
			if (!std::isalnum(static_cast<unsigned char>(c)) && special.find(c) == std::string::npos)
				return false;
		}
		return true;
	}

public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (!client.isPassGiven())
		{
			client.numericReply(IRC::ERR_PASSWDMISMATCH, ":Password incorrect or missing");
			return;
		}
		if (params.empty())
		{
			client.numericReply(IRC::ERR_NONICKNAMEGIVEN, ":No nickname given");
			return;
		}
		std::string_view nick = params[0];

		if (!isNickValid(nick))
		{
			client.numericReply(IRC::ERR_ERRONEUSNICKNAME, std::string(nick) + " :Erroneus nickname");
			return;
		}

		if (server.isNickInUse(nick))
		{
			client.numericReply(IRC::ERR_NICKNAMEINUSE, std::string(nick) + " :Nickname is already in use");
			return;
		}
		std::string oldPrefix = client.getUserPrefix();
		client.setNickname(nick);

		if (client.isRegistered())
		{
			std::string msg = oldPrefix + " NICK :" + std::string(nick);
			client.sendMessage(msg);
			server.broadcastToChannels(client, msg);
		}
		else
			server.registerClient(client);
	}
};
