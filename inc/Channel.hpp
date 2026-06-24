#pragma once

#include <chrono>
#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Utils.hpp"

class Server;
class Client;

class Channel
{
private:
	std::string _name;
	std::chrono::system_clock::time_point _creationTime;
	std::string _topic;
	std::string _key;
	bool _inviteOnly = false;
	bool _privilegeRequired4Topic = false;
	std::size_t _memberLimit = IRC::MAX_CHANNEL_SIZE;
	std::string _topicSetter;
	std::chrono::system_clock::time_point _topicSetTime;

	std::unordered_map<int, Client*> _members;
	std::unordered_map<int, Client*> _operators;
	std::unordered_map<int, Client*> _invited;

public:
	Channel(std::string_view name)
		: _name{ name }
		, _creationTime{ std::chrono::system_clock::now() }
	{
	}
	Channel(const Channel&) = delete;
	Channel(Channel&&) = delete;
	~Channel() = default;

	Channel& operator=(const Channel&) = delete;
	Channel& operator=(Channel&&) = delete;

	[[nodiscard]] const std::string& getName() const { return _name; }
	[[nodiscard]] const std::string& getTopic() const { return _topic; }
	[[nodiscard]] const std::string& getPassword() const { return _key; }
	[[nodiscard]] const std::unordered_map<int, Client*>& getMembers() const { return _members; }

	bool hasClient(int socket) const;
	bool hasTopic() const;
	bool hasTopicRestriction() const;

	[[nodiscard]] const std::string& getTopicSetter() const { return _topicSetter; }
	[[nodiscard]] std::time_t getTopicSetTime() const { return std::chrono::system_clock::to_time_t(_topicSetTime); }

	void setTopic(const std::string& topic, const std::string& nick)
	{
		_topic = topic;
		_topicSetter = nick;
		_topicSetTime = std::chrono::system_clock::now();
	}

	bool hasKey() const;
	bool isOperator(int socket) const;
	bool isInviteOnly() const;
	bool isInvited(int socket) const;
	void addInvite(Client& client);
	void addMember(Client& client);
	void removeMember(int socket);
	void addOperator(Client& client);
	void removeOperator(int socket);
	bool isFull() const;
	std::size_t getMemberSize() const;
	std::string allMembers() const;

	[[nodiscard]] const std::string& getChannelName() const
	{
		return _name;
	}  //dup function, choose either getName() or getChannelName(), then make changes to other files
	[[nodiscard]] std::chrono::system_clock::time_point getCreationTime() const { return _creationTime; }
	std::string printCreationTime() const;
	void setModeInvite(int AddOrRemove);
	void setModeTopic(int AddOrRemove);
	void setPassword(const std::string& pw) { _key = pw; }
	void removePassword() { _key.clear(); }
	void setMemberLimit(std::size_t num) { _memberLimit = num; }
	std::size_t getMemberLimit() const { return _memberLimit; }
	Client* retrieveClient(const std::string& name);
	void broadcastToMembers(const std::string& msg);
};
