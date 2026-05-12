#pragma once

#include <cstddef>
#include <cstdint>

namespace IRC
{
	constexpr std::uint16_t PORT_MIN = 1024;
	constexpr std::size_t EVENT_QUEUE_SIZE = 64;

	constexpr std::int32_t RPL_WELCOME = 1;
	constexpr std::int32_t RPL_YOURHOST = 2;
	constexpr std::int32_t RPL_CREATED = 3;
	constexpr std::int32_t RPL_MYINFO = 4;

	constexpr std::int32_t ERR_NEEDMOREPARAMS = 461;
}
