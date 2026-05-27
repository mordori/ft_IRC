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
		if (nick.empty() || nick.length() > IRC::NICKLEN)
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
			server.log(LOG_ERROR, "Password incorrect or missing");
			client.numericReply(IRC::ERR_PASSWDMISMATCH, ":Password incorrect or missing");
			return;
		}
		if (params.empty())
		{
			server.log(LOG_WARNING, "No nickname given");
			client.numericReply(IRC::ERR_NONICKNAMEGIVEN, ":No nickname given");
			return;
		}
		std::string_view nick = params[0];

		if (!isNickValid(nick))
		{
			server.log(LOG_ERROR, "Erroneus nickname choice");
			client.numericReply(IRC::ERR_ERRONEUSNICKNAME, std::string(nick) + " :Erroneus nickname");
			return;
		}

		if (server.isNickInUse(nick))
		{
			server.log(LOG_WARNING, "Duplicate nickname choice");
			client.numericReply(IRC::ERR_NICKNAMEINUSE, std::string(nick) + " :Nickname is already in use");
			return;
		}
		std::string oldPrefix = client.getUserPrefix();
		std::string oldNick = client.getNickname();
		client.setNickname(nick);

		if (client.isRegistered())
		{
			std::string msg = oldPrefix + " NICK :" + std::string(nick);
			client.sendMessage(msg);
			server.broadcastToChannels(client, msg);
			
			std::string readableMsg = "<" + oldNick + "> changed nickname to <" + std::string(nick) + ">";
			server.log(LOG_INFO, readableMsg);
			readableMsg = "Changed nickname to " + std::string(nick);
			client.sendMessage(readableMsg);
		}
		else
		{
			std::string wrongformat = "<" + std::string(nick) + "> :Nickname registered";
			server.log(LOG_INFO, wrongformat);
			client.sendMessage(wrongformat);
			server.registerClient(client);
		}
	}
};
