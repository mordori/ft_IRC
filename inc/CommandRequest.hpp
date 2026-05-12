#pragma once

#include <string_view>
#include <vector>

struct CommandRequest
{
	std::string_view name;
	std::vector<std::string_view> params;

	explicit CommandRequest(std::string_view message);

	[[nodiscard]] bool isValid() const { return name.empty(); }
};
