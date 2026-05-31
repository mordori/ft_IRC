#pragma once

#include <string_view>
#include <vector>

#include "../Client.hpp"
#include "../Server.hpp"
#include "../Utils.hpp"
#include "ICommand.hpp"


class Join : public ICommand
{
private:
	static std::string_view split(std::string_view& str)
	{
		auto pos = str.find(',');
		auto splited = str.substr(0, pos);
		str = (pos == std::string_view::npos) ? std::string_view{} : str.substr(pos + 1);
		return splited;
	}
	
	static bool isValidChannel(std::string_view name)
	{
		if (name.empty() || name[0] != '#' || name.length() > IRC::CHANNELLEN)
			return false;
		for (char c : name)
		{
			 if (c == '\0' || c == '\a' || c == '\r' || c == '\n' || c == ' ' || c == ',' || c == ':')
            			return false;
		}
		return true;		
	}

public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			server.log(LOG_WARNING, "Not enough parameters for JOIN command");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
			return;
		}
		if (!client.isRegistered())
		{
			server.log(LOG_WARNING, "User registration is required for JOIN command");
			client.numericReply(IRC::ERR_NOTREGISTERED, ":You have not registered");
			return;
		}
		// JOIN 0 without a '#' prefix (Part all channels)
		if (params[0] == "0")
		{
			for (auto& [name, channel] : server.getAllChannels())
			{
				if (!channel->hasClient(client.getSocket()))
					continue;
				std::string msg = client.getUserPrefix() + " PART " + channel->getName() + " :";
				for (auto& [socket, member] : channel->getMembers())
					member->sendMessage(msg);
				channel->removeMember(client.getSocket());
				server.log(LOG_INFO, client.getNickname() + " left channel " + channel->getName());
			}
			client.clearChannels();
			return;
		}

		std::string_view channels = params[0];
		std::string_view keys = params.size() >= 2 ? params[1] : std::string_view{};
		while (!channels.empty())
		{
			std::string_view name = split(channels);
			std::string_view password = split(keys);

			if (!isValidChannel(name))
			{
				server.log(LOG_WARNING, "Channel name is not valid.");
				client.numericReply(IRC::ERR_BADCHANMASK, std::string(name) + " :Bad Channel Mask");
				return;
			}

			Channel* channel = server.findChannel(std::string(name));
			bool isNew = (channel == nullptr);
			if (!isNew)
			{
				if (channel->hasClient(client.getSocket()))
					continue;
				if (channel->hasKey() && channel->getPassword() != password)
				{
					server.log(LOG_WARNING, "Wrong channel key!");
					client.numericReply(IRC::ERR_BADCHANNELKEY, std::string(name) + " :Cannot join channel (+k)");
					continue;
				}
				if (channel->isInviteOnly() && !channel->isInvited(client.getSocket()))
				{
					server.log(LOG_WARNING, "Channel is invite-only!");
					client.numericReply(IRC::ERR_INVITEONLYCHAN, std::string(name) + " :Cannot join channel (+i)" );
					continue;
				}
				if (channel->isFull())
				{
					server.log(LOG_WARNING, "Channel is full");
					client.numericReply(IRC::ERR_CHANNELISFULL, std::string(name) + " :Cannot join channel (+l)" );
					continue;
				}
			}
			else
			{
				channel = server.createChannel(std::string(name));
			}

			channel->addMember(client);
			client.joinChannel(channel);
			std::string msg = ":" + client.getUserPrefix() + " JOIN " + std::string(name);
			client.sendMessage(msg);
			
			if (isNew)
			{
				channel->addOperator(client);
				std::string modeO = "Mode " + std::string(name) + " +o " + client.getNickname();
				client.sendMessage(modeO);
				server.log(LOG_INFO, client.getNickname() + " is an operator of channel " + std::string(name));
			}
			if (channel->hasTopic())
			{
				client.numericReply(IRC::RPL_TOPIC, std::string(name) + " :" + channel->getTopic());
			}
			
			client.numericReply(IRC::RPL_NAMREPLY, "= " +  std::string(name) + " :" + channel->allMembers());
			client.numericReply(IRC::RPL_ENDOFNAMES, std::string(name) + " :End of /NAMES list");

			for (auto& [socket, member] : channel->getMembers())
			{
				if (member->getSocket() != client.getSocket())
					member->sendMessage(msg);
			}
			server.log(LOG_INFO, client.getNickname() + " joined " + std::string(name));			
		}
	}
};

