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
			server.log(LOG_ERROR, client.getNickname() + ": [TOPIC] Client not registered");
			client.numericReply(IRC::ERR_NOTREGISTERED, ":You have not registered");
			return;
		}
		if (params.empty())
		{
			server.log(LOG_ERROR, client.getNickname() + ": [TOPIC] Not enough parameters");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, ":Not enough parameters");
			return;
		}

		std::string channelName{ params[0] };
		Channel* channel = server.findChannel(channelName);

		if (!channel)
		{
			server.log(LOG_ERROR, client.getNickname() + ": [TOPIC] No such channel");
			client.numericReply(IRC::ERR_NOSUCHCHANNEL, channelName + " :No such channel");
			return;
		}
		if (!channel->hasClient(client.getSocket()))
		{
			server.log(LOG_ERROR, client.getNickname() + ": [TOPIC] Client is not a member of the channel");
			client.numericReply(IRC::ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
			return;
		}

		if (params.size() == 1)
		{
			if (channel->hasTopic())
			{
				server.log(LOG_INFO, client.getNickname() + ": [TOPIC] Client requested topic");
				client.numericReply(IRC::RPL_TOPIC, channelName + " :" + channel->getTopic());
				client.numericReply(IRC::RPL_TOPICWHOTIME, channelName + " " + channel->getTopicSetter() + " " + printTopicSetTime(channel->getTopicSetTime()));
			}
			else
			{
				server.log(LOG_INFO, client.getNickname() + ": [TOPIC] No topic is set");
				client.numericReply(IRC::RPL_NOTOPIC, channelName + " :No topic is set");
			}
			return;
		}

		if (channel->hasTopicRestriction() && !channel->isOperator(client.getSocket()))
		{
			server.log(LOG_ERROR, client.getNickname() + ": [TOPIC] Client has no privileges to set a topic");
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, channelName + " :You are not channel operator");
			return;
		}

		std::string newTopic{ params[1] };
		channel->setTopic(newTopic, client.getNickname());
		std::string msg = client.getUserPrefix() + " TOPIC " + channelName + " :" + newTopic;
		channel->broadcastToMembers(msg);
		server.log(LOG_INFO, client.getNickname() + " changed topic for " + channelName);
	}

	static std::string printTopicSetTime(std::time_t time)
	{
		auto local = *std::localtime(&time);
		std::ostringstream display;
		display << std::put_time(&local, "%e-%b-%Y %H:%M");
		return display.str();
	}
};
