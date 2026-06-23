#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Part : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			server.log(LOG_ERROR, client.getNickname() + ": [PART] No param provided");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
			return;
		}
		std::string reason;
		if (params.size() > 1)	//reason was provided for leaving channel(s)
		{
			reason = " :";
			for (size_t i = 1; i < params.size(); i++)
				reason += std::string(params[i]) + " ";
			reason.erase(reason.end() - 1);
		}

		std::string_view paramChannels = params[0];
		while (!paramChannels.empty())
		{
			std::string channelName = std::string(Utils::split(paramChannels));
			Channel* channel = server.findChannel(channelName);
			if (!channel)
			{
				server.log(LOG_ERROR, client.getNickname() + ": [PART] Channel " + channelName + " does not exist");
				client.numericReply(IRC::ERR_NOSUCHCHANNEL, channelName + " :No such channel");
				continue;
			}
			if (!channel->hasClient(client.getSocket()))
			{
				server.log(LOG_ERROR, client.getNickname() + ": [PART] Not in channel " + channel->getChannelName());
				client.numericReply(IRC::ERR_NOTONCHANNEL, channel->getChannelName() + " :You're not on that channel");
				continue;
			}
			std::string broadcastMsg = client.getUserPrefix() + " PART " + channel->getChannelName() + reason;
			channel->broadcastToMembers(broadcastMsg);
			client.leaveChannel(channel);
			channel->removeMember(client.getSocket());
			server.log(LOG_INFO, client.getNickname() + ": [PART] Leaving channel " + channel->getChannelName() + reason);
			if (channel->getMemberSize() == 0)
				server.removeChannel(channel->getChannelName());
		}
	}
};
