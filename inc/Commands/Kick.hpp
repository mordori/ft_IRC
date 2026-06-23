#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Kick : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.size() < 2)
		{
			server.log(LOG_WARNING, "Not enough parameters for KICK command");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
			return;
		}

		std::string_view name = params[0];
		std::string_view clientList = params[1];
		std::string comment;
		if (params.size() > 2)
			comment = " :" + std::string(params[2]);
		std::string channelName = std::string(name);
		Channel* channel = server.findChannel(channelName);
		if (!channel)
		{
			server.log(LOG_WARNING, channelName + " does not exist");
			client.numericReply(IRC::ERR_NOSUCHCHANNEL, channelName + " :No such channel");
			return;
		}
		if (!channel->hasClient(client.getSocket()))
		{
			server.log(LOG_WARNING, "Client is not on channel!");
			client.numericReply(IRC::ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
			return;
		}
		if (!channel->isOperator(client.getSocket()))
		{
			server.log(LOG_WARNING, "Client is not an operator!");
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
			return;
		}

		while (!clientList.empty())
		{
			std::string_view nick = Utils::split(clientList);
			if (nick.empty())
				continue;
			std::string targetNick = std::string(nick);
			Client* target = server.findClient(std::string(nick));
			if (!target || !channel->retrieveClient(targetNick))
			{
				server.log(LOG_WARNING, targetNick + " is not found or not on channel!");
				client.numericReply(IRC::ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
				continue;
			}

			std::string msg = client.getUserPrefix() + " KICK " + channelName + " " + targetNick + comment;
			for (const auto& [socket, member] : channel->getMembers())
				member->sendMessage(msg);
			channel->removeMember(target->getSocket());
			target->leaveChannel(channel);
			server.log(LOG_INFO, client.getNickname() + " kicked " + targetNick + " from " + channelName);
		}

		if (channel->getMembers().empty())
		{
			server.log(LOG_INFO, channelName + " is empty. Removing the channel");
			server.removeChannel(channelName);
		}
	}
};
