#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Topic : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (!client.isRegistered())
		{
			client.numericReply(IRC::ERR_NOTREGISTERED, ":You have not registered");
			return;
		}
		if (params.empty())
		{
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, ":Not enough parameters");
			return;
		}

		std::string channelName{ params[0] };
		Channel* channel = server.findChannel(channelName);

		if (!channel)
		{
			client.numericReply(IRC::ERR_NOSUCHCHANNEL, channelName);
			return;
		}
		if (!channel->hasClient(client.getSocket()))
		{
			client.numericReply(IRC::ERR_NOTONCHANNEL, channelName);
			return;
		}

		if (params.size() == 1)
		{
			if (channel->hasTopic())
			{
				client.numericReply(IRC::RPL_TOPIC, channelName + " :" + channel->getTopic());
				std::string time = std::to_string(channel->getTopicSetTime());
				client.numericReply(IRC::RPL_TOPICWHOTIME, channelName + " " + channel->getTopicSetter() + " " + time);
			}
			else
			{
				client.numericReply(IRC::RPL_NOTOPIC, channelName + " :No topic is set");
			}
			return;
		}

		if (channel->hasTopicRestriction() && !channel->isOperator(client.getSocket()))
		{
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, channelName + " :You are not channel operator");
			return;
		}

		std::string newTopic{ params[1] };
		channel->setTopic(newTopic, client.getNickname());
		std::string msg = client.getUserPrefix() + " TOPIC " + channelName + " :" + newTopic;
		channel->broadcastToMembers(msg);
		server.log(LOG_INFO, client.getNickname() + " changed topic for " + channelName);
	}
};
