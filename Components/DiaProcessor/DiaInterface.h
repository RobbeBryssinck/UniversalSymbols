#pragma once

#include <UniversalSymbolsFormat/USYM.h>

#include <optional>

namespace DiaInterface
{
  void InitializeDia(const char* apFileName);
  void Release();
	std::optional<USYM> CreateUsymFromFile(const char* apFileName);
}
