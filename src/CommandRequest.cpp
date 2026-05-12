#include "../inc/CommandRequest.hpp"

#include <cstddef>
#include <string_view>

CommandRequest::CommandRequest(std::string_view message)
{
	std::size_t start{ message.find_first_not_of(" \t") };
	if (start == std::string_view::npos)
		return;
	message.remove_prefix(start);
	std::size_t space = message.find(' ');
	if (space == std::string_view::npos)
	{
		name = message;
		return;
	}
	name = message.substr(0, space);
	message.remove_prefix(space + 1);
	while (!message.empty())
	{
		start = message.find_first_not_of(" \t");
		if (start == std::string_view::npos)
			break;
		message.remove_prefix(start);
		if (message.front() == ':')
		{
			params.emplace_back(message.substr(1));
			break;
		}
		space = message.find(' ');
		if (space != std::string_view::npos)
		{
			params.emplace_back(message.substr(0, space));
			message.remove_prefix(space + 1);
		}
		else
		{
			params.emplace_back(message);
			break;
		}
	}
}
