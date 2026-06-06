#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "ICommand.hpp"

class Quit : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		std::string reason = "";
		if (!params.empty())
		{
			for (size_t i = 0; i < params.size(); i++)
				reason += " " + std::string(params[i]);
		}
		std::string broadcastMsg = client.getUserPrefix() + " QUIT :Quit:" + reason;
		
		//terminate a client’s connection to the server
		std::unordered_set<Channel*>	clientChannels = client.getChannels();
		if (!clientChannels.empty())
		{
			for (Channel* channel : clientChannels)
			{
				channel->removeMember(client.getSocket());
				channel->broadcastToMembers(broadcastMsg);
			}
		}
		client.clearChannels();
		std::string errMsg = "ERROR : Closing connection" + reason;
		client.sendMessage(errMsg);
		client.setDisconnect(true);
		server.log(LOG_INFO, client.getNickname() + " is quitting");
		server.removeClient(client.getSocket());
	}
};
