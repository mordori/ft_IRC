#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

class Server;

class Client;

class Channel
{
private:
	std::string _name;
	std::string	_modes;
	std::string _topic;
	std::string _key;
	bool _inviteOnly = false;
	size_t _memberLimit;
	
	std::unordered_map<int, Client*> _members;
	std::unordered_map<int, Client*> _operators;
	std::unordered_map<int, Client*> _invited; 

public:
	Channel(std::string_view name) : _name{ name } {}
	Channel(const Channel&) = delete;
	Channel(Channel&&) = delete;
	~Channel() = default ;

	Channel& operator=(const Channel&) = delete;
	Channel& operator=(Channel&&) = delete;

	[[nodiscard]] const std::string& getName() const { return _name; }
	[[nodiscard]] const std::string& getTopic() const { return _topic; }
	[[nodiscard]] const std::string& getPassword() const { return _key; } 
	[[nodiscard]] const std::unordered_map<int, Client*>& getMembers() const { return _members; }
	
	bool hasClient(int socket) const;
	bool hasTopic() const;
	bool hasKey() const;
	bool isOperator(int socket) const;
	bool isInviteOnly() const;
	bool isInvited(int socket) const;
	void addMember(Client& client);
	void removeMember(int socket);
	void addOperator(Client& client);
	bool isFull() const;
	size_t getMemberSize() const;
	std::string allMembers() const;

	
	[[nodiscard]] const std::string& getChannelName() const { return _name; }

	void	setModes(std::string_view mode); //modify _modes when ops make changes with MODE
	[[nodiscard]] const std::string& getModes() const { return _modes; }
};
