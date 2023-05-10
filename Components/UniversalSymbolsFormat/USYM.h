#pragma once

#include "Serializers/ISerializer.h"

#include <memory>
#include <optional>
#include <vector>

struct USYM
{
  struct Header
  {
    uint32_t magic = 'MYSU';
  };

  struct Symbol
  {
    uint32_t id{};
    std::string name{};
  };

  struct TypeSymbol : public Symbol
  {
    uint64_t length{};
  };

  // https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/cv-call-e?view=vs-2022
  // TODO: MS docs out of date, since there are a lot more values in CV_call_e?
  enum class CallingConvention : uint32_t
  {
    kNearC = 0,
    kNearFast,
    kNearStd,
    kNearSys,
    kThiscall,
    kCLRCall,

    kUnknown = 0xFFFFFFFF,
  };

  struct FunctionSymbol : public Symbol
  {
    uint32_t returnTypeId{};
    uint32_t argumentCount{};
    std::vector<uint32_t> argumentTypeIds{}; // TODO
    CallingConvention callingConvention{ CallingConvention::kUnknown };
  };

  void SetSerializer(ISerializer::Type aType);

  ISerializer::SerializeResult Serialize();

  std::unique_ptr<ISerializer> pSerializer = nullptr;

  Header header{};
  std::vector<TypeSymbol> typeSymbols{};
  std::vector<FunctionSymbol> functionSymbols{};
};
