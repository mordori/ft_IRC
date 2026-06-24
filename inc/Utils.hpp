#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace IRC
{
constexpr std::uint16_t PORT_MIN = 1024;
constexpr std::size_t NICKLEN = 9;
constexpr std::size_t USERLEN = 9;
constexpr std::size_t CHANNELLEN = 50;
constexpr std::size_t EVENT_QUEUE_SIZE = 64;
constexpr std::size_t MAX_CHANNEL_SIZE = 200;

constexpr std::string_view SERVER_NAME = "ft_irc.mastermind";
constexpr std::string_view SERVER_VERSION = "mastermind-1.0";
constexpr std::string_view AVAILABLE_USER_MODES;
constexpr std::string_view AVAILABLE_CHANNEL_MODES = "itkol";

constexpr std::string_view RPL_WELCOME = "001";
constexpr std::string_view RPL_YOURHOST = "002";
constexpr std::string_view RPL_CREATED = "003";
constexpr std::string_view RPL_MYINFO = "004";
constexpr std::string_view RPL_ISUPPORT = "005";

constexpr std::string_view RPL_CHANNELMODEIS = "324";
constexpr std::string_view RPL_CREATIONTIME = "329";
constexpr std::string_view RPL_NOTOPIC = "331";
constexpr std::string_view RPL_TOPIC = "332";
constexpr std::string_view RPL_TOPICWHOTIME = "333";
constexpr std::string_view RPL_INVITING = "341";
constexpr std::string_view RPL_NAMREPLY = "353";
constexpr std::string_view RPL_ENDOFNAMES = "366";

constexpr std::string_view ERR_NOSUCHNICK = "401";
constexpr std::string_view ERR_NOSUCHCHANNEL = "403";
constexpr std::string_view ERR_UNKNOWNCOMMAND = "421";
constexpr std::string_view ERR_NONICKNAMEGIVEN = "431";
constexpr std::string_view ERR_ERRONEUSNICKNAME = "432";
constexpr std::string_view ERR_NICKNAMEINUSE = "433";
constexpr std::string_view ERR_USERNOTINCHANNEL = "441";
constexpr std::string_view ERR_NOTONCHANNEL = "442";
constexpr std::string_view ERR_USERONCHANNEL = "443";

constexpr std::string_view ERR_NOTREGISTERED = "451";
constexpr std::string_view ERR_NEEDMOREPARAMS = "461";
constexpr std::string_view ERR_ALREADYREGISTERED = "462";
constexpr std::string_view ERR_PASSWDMISMATCH = "464";
constexpr std::string_view ERR_CHANNELISFULL = "471";
constexpr std::string_view ERR_UNKNOWNMODE = "472";
constexpr std::string_view ERR_INVITEONLYCHAN = "473";
constexpr std::string_view ERR_BADCHANNELKEY = "475";
constexpr std::string_view ERR_BADCHANMASK = "476";
constexpr std::string_view ERR_CHANOPRIVSNEEDED = "482";

constexpr std::string_view ERR_INVALIDKEY = "525";
constexpr std::string_view ERR_INVALIDMODEPARAM = "696";
}

namespace Utils
{
inline std::string_view split(std::string_view& str)
{
	auto pos = str.find(',');
	auto splited = str.substr(0, pos);
	str = (pos == std::string_view::npos) ? std::string_view{} : str.substr(pos + 1);
	return splited;
}
}

enum
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_MISC
};
