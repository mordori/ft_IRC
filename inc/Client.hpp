#pragma once

#include <string>
#include <string_view>
#include <unordered_set>

class Server;
class Channel;

class Client
{
private:
	Server& _server;
	int _socket{ -1 };
	std::string _bufferIn;
	std::string _bufferOut;
	std::string _nickname;
	std::string _realname;
	std::string _username;
	std::string _hostname;
	std::string _password;
	bool _isRegisterd = false;
	bool _isPassGiven = false;
	bool _isDisconnected = false;
	std::unordered_set<Channel*> _channels;

public:
	Client(Server& server, int socket) : _server{ server }, _socket{ socket } {}
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	~Client();

	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void receiveBytes();
	void sendBytes();
	void sendMessage(std::string_view message);
	void numericReply(std::string_view numeric, std::string_view msg);

	[[nodiscard]] int getSocket() const { return _socket; }
	[[nodiscard]] const std::string& getNickname() const { return _nickname; }
	[[nodiscard]] const std::string& getUsername() const { return _username; }
	[[nodiscard]] const std::string& getRealname() const { return _realname; }
	[[nodiscard]] const std::string& getHostname() const { return _hostname; }
	[[nodiscard]] bool isRegistered() const { return _isRegisterd; }
	[[nodiscard]] bool isPassGiven() const { return _isPassGiven; }
	[[nodiscard]] bool isDisconnected() const { return _isDisconnected; }
	[[nodiscard]] const std::unordered_set<Channel*>& getChannels() const { return _channels; }

	void setHostname(std::string_view host) { _hostname = host; }
	void setNickname(std::string_view nick) { _nickname = nick; }
	void setUsername(std::string_view user) { _username = user; }
	void setRealname(std::string_view real) { _realname = real; }
	void setPassGiven(bool pass) { _isPassGiven = pass; }
	void setRegistered(bool registered) { _isRegisterd = registered; }
	void setDisconnect(bool status) { _isDisconnected = status; }

	std::string getUserPrefix() const;
	void joinChannel(Channel* channel);
	void leaveChannel(Channel* channel);
	void clearChannels();
	

};
