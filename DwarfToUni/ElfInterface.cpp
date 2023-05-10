#include "ElfInterface.h"

#include <Reader.h>

namespace ElfInterface
{
	static Reader s_reader{};
	static bool s_is64Bit{};

	std::optional<USYM> CreateUsymFromFile(const char* apFileName)
	{
		if (!s_reader.LoadFromFile(apFileName))
			return std::nullopt;

		USYM usym{};

		return usym;
	}
}
