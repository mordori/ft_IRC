#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "ICommand.hpp"

class Mode : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			server.log(LOG_ERROR, client.getNickname() + ": MODE error - No param provided");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, client.getNickname() + " :Need more param for MODE");
			return ;
		}
		if (!(params[0].starts_with("#") || params[0].starts_with("&"))) //# for regular channels, & for local channels
		{
			server.log(LOG_WARNING, client.getNickname() + ": MODE error - Attempting user mode");
			client.sendMessage("User modes not supported");
			return ;
		}
		// if (!server.existingChannel(params[0])) change so that can use channel from here on
		// {
		// 	server.log(LOG_WARNING, client.getNickname() + ": MODE error - Channel does not exist");
			// client.numericReply(IRC::ERR_NOSUCHCHANNEL, client.getNickname() + " " + std::string(params[0]) + " :No such channel");
		// 	return ;
		// }
		// if (client not in channel)
		// {
			// server.log(LOG_ERROR, client.getNickname() + ": Not in channel " + channel.getChannelName());
			// client.numericReply(IRC::ERR_NOTONCHANNEL, client.getNickname() + " " + channel.getChannelName() + " :You're not on that channel");
			// return;
		// }
		// if (params[1].empty())
		// {
		// 	std::string	reply = client.getNickname() + " " + channel.getChannelName() + " " + channel.getModes();
		// 	client.numericReply(IRC::RPL_CHANNELMODEIS, reply);
		// 	server.log(LOG_INFO, client.getNickname() + ": Mode inquiry for channel " + channel.getChannelName());
		// 	return;
		// }
		// else
		// {
		// 	if (client is not operator)
		// 	{
		// 		server.log(LOG_ERROR, client.getNickname() + ": MODE error - Client not operator");
		// 		client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, client.getNickname() + " " + channel.getChannelName() + " :Not channel operator");
		// 		return;
		// 	}
			// if (check mode)
			// {
			// 	ERR_UNKNOWNMODE (472)
			// "<client> <modechar> :is unknown mode char to me"

			// }
		// }
	}
};
