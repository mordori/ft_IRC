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
	std::string _topic;
	std::string _key;
	bool _inviteOnly = false;
	
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

	[[nodiscard]] bool hasClient(int socket) const { return _members.contains(socket); }
	[[nodiscard]] const std::unordered_map<int, Client*>& getMembers() const { return _members; }
};
