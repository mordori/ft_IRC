#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"

bool Channel::hasClient(int socket) const
{ 
	return _members.contains(socket);
}

bool Channel::hasTopic() const
{ 
	return !_topic.empty();
}

bool Channel::hasKey() const
{
	return !_key.empty();
}

bool Channel::isOperator(int socket) const
{
	return _operators.contains(socket);
}

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

bool Channel::isInvited(int socket) const
{
	return _invited.contains(socket);
}

void Channel::addMember(Client& client)
{ 
	_members[client.getSocket()] = &client;
}

void Channel::removeMember(int socket)
{
	_members.erase(socket);
}

void Channel::addOperator(Client& client)
{
	_operators[client.getSocket()] = &client; 
}

bool Channel::isFull() const
{
	return _memberLimit > 0 && _members.size() >= _memberLimit;
}

size_t Channel::getMemberSize() const
{
	return _members.size();
}

std::string Channel::allMembers() const
{
	std::string nameList;
	
	for (const auto& [socket, member] : _members)
	{
		if (!nameList.empty())
			nameList += " ";
		if (isOperator(socket))
			nameList += "@";
		nameList += member->getNickname();
	}
	return nameList;
}
	