#pragma once

#include <string_view>
#include <vector>

#include "../Channel.hpp"
#include "../Client.hpp"
#include "ICommand.hpp"

class Mode : public ICommand
{
private:
	bool	checkModeStringStart(std::string_view str)
	{
		if (!(str.starts_with('+') || str.starts_with('-')))
			return false;
		return true;
	}

public:
	void execute(Client& client, Server& server, const std::vector<std::string_view>& params) override
	{
		if (params.empty())
		{
			server.log(LOG_ERROR, client.getNickname() + ": [MODE] No param provided");
			client.numericReply(IRC::ERR_NEEDMOREPARAMS, " :[MODE] Need more param");
			return ;
		}
		if (!params[0].starts_with("#"))
		{
			server.log(LOG_WARNING, client.getNickname() + ": [MODE] Attempting user mode");
			client.sendMessage("[MODE] User modes not supported");
			return ;
		}
		
		Channel* channel = server.findChannel(std::string(params[0]));
		if (!channel)
		{
			server.log(LOG_WARNING, client.getNickname() + ": [MODE] Channel " + std::string(params[0]) + " does not exist");
			client.numericReply(IRC::ERR_NOSUCHCHANNEL, std::string(params[0]) + " :No such channel");
			return ;
		}
		if (!channel->hasClient(client.getSocket()))
		{
			server.log(LOG_ERROR, client.getNickname() + ": Not in channel " + channel->getChannelName());
			client.numericReply(IRC::ERR_NOTONCHANNEL, channel->getChannelName() + " :You're not on that channel");
			return;
		}

		//Basic channel info inquiry, anyone can send command
		if (params[1].empty())
		{
			// client.numericReply(IRC::RPL_CHANNELMODEIS, channel->getChannelName() + " " + channel->getModes());
			//Need to build getModes(): check channel stats and build gradually a string with modes (+params)
			client.numericReply(IRC::RPL_CREATIONTIME, channel->getChannelName() + channel->printCreationTime());
			server.log(LOG_INFO, client.getNickname() + ": Mode inquiry for channel " + channel->getChannelName());
			return;
		}

		//From here on, needs to be channel operator
		if (!channel->isOperator(client.getSocket()))
		{
			server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE] Client not operator");
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, channel->getChannelName() + " :Not channel operator");
			return;
		}
		
		//Main work: Parse the whole mode string (+ or - itkol) along with mode arguments (tracked with paramIndex)
		std::string_view modeString = params[1];
		std::string	broadcastMsg;
		if (!checkModeStringStart(modeString))
		{
			server.log(LOG_ERROR, client.getNickname() + ": [MODE] Mode string does not start with correct character");
			client.sendMessage(client.getNickname() + " :Mode string does not start with correct character");
			return;
		}
		int	AddOrRemove = 0;
		size_t	paramIndex = 2;
		for (int i = 0; i < modeString.size(); i++)
		{
			if (modeString[i] == '+')
			{
				AddOrRemove = 1;
				broadcastMsg += modeString[i];
				continue;
			}
			else if (modeString[i] == '-')
			{
				AddOrRemove = -1;
				broadcastMsg += modeString[i];
				continue;
			}
			switch (modeString[i])
			{
			case 'i':
				channel->setModeInvite(AddOrRemove);
				broadcastMsg += modeString[i];
				break;
			case 't':
				channel->setModeTopic(AddOrRemove);
				broadcastMsg += modeString[i];
				break;
			case 'k':
				if (AddOrRemove == 1)
				{
					//more checks with param
				}
				else if (AddOrRemove == -1)
				{
					//remove key
				}
				broadcastMsg += modeString[i];
				break;
			case 'o':
				//more checks with param: if in channel
				// Client* target = channel.retrieveClient();
				if (AddOrRemove == 1)
				{
					// channel->addOperator(target);
				}
				else if (AddOrRemove == -1)
				{
					// channel->removeOperator(target.getSocket())
				}
				broadcastMsg += modeString[i];
				break;
			case 'l':
				if (AddOrRemove == 1)
				{
					//more checks with param
					// channel->setMemberLimit(param);		
				}
				else if (AddOrRemove == -1)
				{
					channel->setMemberLimit(IRC::MAX_CHANNEL_SIZE);
				}
				broadcastMsg += modeString[i];
				break;
			
			default:
				{
					server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE] Unknown mode " + modeString[i]);
					client.numericReply(IRC::ERR_UNKNOWNMODE, modeString[i] + " :is an unknown mode char");
				}
				break;
			}
		}
		channel->broadcastToMembers(client, broadcastMsg);
	}
};
