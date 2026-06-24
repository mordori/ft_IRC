#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Privmsg : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (!client.isRegistered())
		{
			client.numericReply(IRC::ERR_NOTREGISTERED, ":You have not registered");
			return;
		}
		if (params.empty() || params[0].empty())
		{
			client.numericReply(IRC::ERR_NORECIPIENT, ":No recipient given");
			return;
		}

		if (params.size() < 2 || params[1].empty())
		{
			client.numericReply(IRC::ERR_NOTEXTTOSEND, " :No text to send");
			return;
		}

		std::string_view recipients = params[0];
		std::string_view msg = params[1];

		std::string prefix = client.getUserPrefix() + " PRIVMSG ";
		while (!recipients.empty())
		{
			std::string_view recipient = Utils::split(recipients);
			if (recipient.empty())
				continue;
			if (recipient[0] == '#')
			{
				Channel* channel = server.findChannel(std::string(recipient));
				if (!channel)
				{
					client.numericReply(IRC::ERR_NOSUCHCHANNEL, std::string(recipient) + " :No such channel");
					continue;
				}
				if (!channel->hasClient(client.getSocket()))
				{
					client.numericReply(IRC::ERR_CANNOTSENDTOCHAN, std::string(recipient) + " :Cannot send to channel");
					continue;
				}
				std::string chanMsg = prefix + std::string(recipient) + " :" + std::string(msg);
				for (const auto& [socket, member] : channel->getMembers())
					if (socket != client.getSocket())
						member->sendMessage(chanMsg);
			}
			else
			{
				Client* other = server.findClient(std::string(recipient));
				if (!other)
				{
					client.numericReply(IRC::ERR_NOSUCHNICK, std::string(recipient) + " :No such nick");
					continue;
				}
				std::string clientMsg = prefix + std::string(recipient) + " :" + std::string(msg);
				other->sendMessage(clientMsg);
			}
		}
	}
};
