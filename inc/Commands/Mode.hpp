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
			client.numericReply(IRC::RPL_CHANNELMODEIS, channel->getChannelName() + " " + getModes(channel));
			client.numericReply(IRC::RPL_CREATIONTIME, channel->getChannelName() + channel->printCreationTime());
			server.log(LOG_INFO, client.getNickname() + ": Mode inquiry for channel " + channel->getChannelName());
			return;
		}

	//Main work: Parse the whole mode string (+ or - itkol) along with mode arguments (tracked with paramIndex)
		std::string_view modeString = params[1];
		if (!checkModeStringStart(modeString))
		{
			server.log(LOG_ERROR, client.getNickname() + ": [MODE] Mode string does not start with correct character");
			client.sendMessage(client.getNickname() + " :Mode string does not start with correct character");
			return;
		}
		//From here on, needs to be channel operator
		if (!channel->isOperator(client.getSocket()))
		{
			server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE] Client not operator");
			client.numericReply(IRC::ERR_CHANOPRIVSNEEDED, channel->getChannelName() + " :Not channel operator");
			return;
		}
		
		std::string	appliedModes;
		std::string	appliedParams;
		int	AddOrRemove = 0;
		size_t	paramIndex = 2;
		for (int i = 0; i < modeString.size(); i++)
		{
			if (modeString[i] == '+')
			{
				if (AddOrRemove == 1) //++ or +i+k
					continue;
				AddOrRemove = 1;
				if (appliedModes.ends_with('-')) //-+
					appliedModes.back() = '+';
				else
					appliedModes += modeString[i];
				continue;
			}
			else if (modeString[i] == '-')
			{
				if (AddOrRemove == -1)
					continue;
				AddOrRemove = -1;
				if (appliedModes.ends_with('+'))
					appliedModes.back() = '-';
				else
					appliedModes += modeString[i];
				continue;
			}
			switch (modeString[i])
			{
				case 'i':
					channel->setModeInvite(AddOrRemove);
					appliedModes += modeString[i];
					break;
				
				case 't':
					channel->setModeTopic(AddOrRemove);
					appliedModes += modeString[i];
					break;
				
				case 'k':
					if (AddOrRemove == 1)
					{
						//more checks with param
						appliedParams += " " + std::string(params[paramIndex]);
						paramIndex++;
					}
					else if (AddOrRemove == -1)
					{
						//remove key
					}
					appliedModes += modeString[i];
					break;
				
				case 'o':
				{
					if (params[paramIndex].empty())
					{
						server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE o] No target user provided");
						client.numericReply(IRC::ERR_NEEDMOREPARAMS, channel->getChannelName() + " :[MODE o] Need a target");
						break;
					}
					std::string	nick = std::string(params[paramIndex]);
					if (!server.isNickInUse(nick))
					{
						server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE o] No such nick");
						client.numericReply(IRC::ERR_NOSUCHNICK, nick + " :[MODE o] No such nick");
						break;
					}
					Client* target = channel->retrieveClient(nick);
					if (!target)
					{
						server.log(LOG_ERROR, client.getNickname() + ": [MODE o] Target user " + nick + " not on channel " + channel->getChannelName());
						client.numericReply(IRC::ERR_USERNOTINCHANNEL, nick + " " + channel->getChannelName() + " :They aren't on that channel");
						break;
					}
					if (AddOrRemove == 1)
						channel->addOperator(*target);
					else if (AddOrRemove == -1)
						channel->removeOperator(target->getSocket());
					appliedModes += modeString[i];
					appliedParams += " " + std::string(params[paramIndex]);
					paramIndex++;
					break;
				}
					
				case 'l':
					if (AddOrRemove == 1)
					{
						if (params[paramIndex].empty())
						{
							server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE +l] No number provided");
							client.numericReply(IRC::ERR_NEEDMOREPARAMS, channel->getChannelName() + " :[MODE +l] Need a number");
							break;
						}
						std::string_view str = params[paramIndex];
						size_t	num{};
						auto res = std::from_chars(str.data(), str.data() + str.size(), num);
						if (res.ec != std::errc{} || res.ptr != str.data() + str.size() || num == 0)
						{
							server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE +l] Invalid number");
							client.sendMessage(client.getNickname() + " :[MODE +l] Invalid number");
							break;
						}
						if (num > IRC::MAX_CHANNEL_SIZE)
							num = IRC::MAX_CHANNEL_SIZE;
						channel->setMemberLimit(num);
						appliedParams += " " + std::string(params[paramIndex]);
						paramIndex++;
					}
					else if (AddOrRemove == -1)
					{
						channel->setMemberLimit(IRC::MAX_CHANNEL_SIZE);
					}
					appliedModes += modeString[i];
					break;
				
				default:
					server.log(LOG_ERROR, client.getNickname() + " " + channel->getChannelName() + ": [MODE] Unknown mode " + modeString[i]);
					client.numericReply(IRC::ERR_UNKNOWNMODE, modeString[i] + " :is an unknown mode char");
					break;
			}
		}
		if (appliedModes.size() >= 2 && appliedModes[1] != '+' && appliedModes[1] != '-')
		{
			std::string	broadcastMsg = client.getUserPrefix() + " MODE " + channel->getChannelName() + " " + appliedModes + appliedParams;
			channel->broadcastToMembers(broadcastMsg);
		}
	}

	std::string&	getModes(Channel* channel)
	{
		std::string	modes;
		if (channel->isInviteOnly())
			modes += 'i';
		if (channel->hasTopicRestriction())
			modes += 't';
		if (channel->hasKey())
			modes += 'k';
			//add key?
		if (channel->getMemberLimit() < IRC::MAX_CHANNEL_SIZE)
			modes += 'l';
			//itoa memberLimit
		if (!modes.empty())
			modes.insert(modes[0], 1, '+');
		return modes;
	}
};
