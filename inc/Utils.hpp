#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace IRC
{
	constexpr std::uint16_t PORT_MIN = 1024;
	constexpr std::size_t EVENT_QUEUE_SIZE = 64;

	constexpr std::string_view RPL_WELCOME = "001";
	constexpr std::string_view RPL_YOURHOST = "002";
	constexpr std::string_view RPL_CREATED = "003";
	constexpr std::string_view RPL_MYINFO = "004";

	constexpr std::string_view ERR_NONICKNAMEGIVEN = "431";
	constexpr std::string_view ERR_ERRONEUSNICKNAME = "432";
	constexpr std::string_view ERR_NICKNAMEINUSE = "433";

	constexpr std::string_view ERR_NEEDMOREPARAMS = "461";
	constexpr std::string_view ERR_ALREADYREGISTERED = "462";
	constexpr std::string_view ERR_PASSWDMISMATCH = "464";
}
