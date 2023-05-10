#pragma once

#include <UniversalSymbolsFormat/USYM.h>
#include <Reader.h>

#include <optional>
#include <vector>
#include <memory>

namespace ElfInterface
{
	std::optional<USYM> CreateUsymFromFile(const char* apFileName);
}
