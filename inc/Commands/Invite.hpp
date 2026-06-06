#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "ICommand.hpp"
#include "Utils.hpp"

class Invite : public ICommand
{
public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.size() < 2)
		{
			server.log(LOG_WARNING, "Not enough parameters for INVITE command");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
			return;
		}

		std::string_view invitedNick = params[0];
		std::string_view channelName = params[1];
		Client* invitedClient = server.findClient(std::string(invitedNick));
		if (!invitedClient)
		{
			server.log(LOG_WARNING, client.getNickname() + " is not found" );
			client.numericReply(IRC::ERR_NOSUCHNICK, std::string(invitedNick) + " :No such nick");
			return;
		}

		Channel* channel = server.findChannel(std::string(channelName));
		if (!channel)
		{
			server.log(LOG_WARNING, std::string(channelName) + " does not exist");
			client.numericReply(IRC::ERR_NOSUCHNICK, std::string(channelName) + " :No such channel");
			return;
		}
		if (!channel->hasClient(client.getSocket()))
		{
			server.log(LOG_WARNING, "Inviter is not on channel!");
			client.numericReply(IRC::ERR_NOTONCHANNEL, std::string(channelName) + " :You're not on that channel");
			return;
		}
		if (channel->hasClient(invitedClient->getSocket()))
		{
			server.log(LOG_WARNING, invitedClient->getNickname() + " is already on channel!" );
			client.numericReply(IRC::ERR_USERONCHANNEL, invitedClient->getNickname() + " " + std::string(channelName) + " :is already on channel");
			return;
		}
		if (channel->isInviteOnly() && !channel->isOperator(client.getSocket()))
		{
			server.log(LOG_WARNING, "Channel is invite only and client is not an operator!");
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, std::string(channelName) + " :You're not channel operator");
			return;
		}

		channel->addInvite(*invitedClient);
		client.numericReply(IRC::RPL_INVITING, std::string(invitedNick) + " " + std::string(channelName));
		std::string msg = ":" + client.getUserPrefix() + " INVITE " + invitedClient->getNickname() + " :" + std::string(channelName);
		invitedClient->sendMessage(msg);
		server.log(LOG_INFO, client.getNickname() + " invited " + std::string(invitedNick) + " to " + std::string(channelName));
	}
};
