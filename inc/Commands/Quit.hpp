#pragma once

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"

class Quit : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		std::string reason;
		if (!params.empty())
			for (auto param : params)
				reason += " " + std::string(param);
		std::string broadcastMsg = client.getUserPrefix() + " QUIT :Quit:" + reason;

		std::unordered_set<Channel*> clientChannels = client.getChannels();
		if (!clientChannels.empty())
		{
			server.broadcastToChannels(client, broadcastMsg);
			for (Channel* channel : clientChannels)
			{
				channel->removeMember(client.getSocket());
				if (channel->getMemberSize() == 0)
					server.removeChannel(channel->getChannelName());
			}
			client.clearChannels();
		}

		if (!reason.empty())
			reason.insert(1, ":");
		std::string errMsg = "ERROR :Closing connection" + reason;
		client.sendMessage(errMsg);
		client.setDisconnect(true);
		server.log(LOG_INFO, client.getNickname() + " is quitting");
	}
};
