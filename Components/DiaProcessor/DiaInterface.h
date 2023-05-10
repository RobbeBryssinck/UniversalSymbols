#pragma once

#include <UniversalSymbolsFormat/USYM.h>

#include <optional>

namespace DiaInterface
{
	std::optional<USYM> CreateUsymFromFile(const char* apFileName);
}
