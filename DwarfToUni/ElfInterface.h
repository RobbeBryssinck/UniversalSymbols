#pragma once

#include <UniversalSymbolsFormat/USYM.h>

#include <optional>

namespace ElfInterface
{
	std::optional<USYM> CreateUsymFromFile(const char* apFileName);
}
