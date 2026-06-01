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

void Channel::removeOperator(int socket)
{
	_operators.erase(socket);
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

std::string	Channel::printCreationTime() {
	auto creationTime = std::chrono::system_clock::to_time_t(getCreationTime());
	auto local = *std::localtime(&creationTime);
	std::ostringstream	display;
	display << std::put_time(&local, " %e-%b-%Y %H:%M") << std::endl;
	return display.str();
}