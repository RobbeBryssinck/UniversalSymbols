#pragma once

#include "Serializers/ISerializer.h"

#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>

struct USYM
{
  enum class OriginalFormat : uint8_t
  {
    kPdb = 0,
    kDwarf,

    kUnknown = 0xFF
  };

  enum class Architecture : uint8_t
  {
    kX86 = 0,
    kX86_64,
    kArm32,
    kArm64,

    kUnknown = 0xFF
  };

  struct Header
  {
    uint32_t magic = 'MYSU';
    OriginalFormat originalFormat{ OriginalFormat::kUnknown };
    Architecture architecture{ Architecture::kUnknown };
  };

  struct Symbol
  {
    uint32_t id{};
    std::string name{};
  };

  // TODO: have child classes for TypeSymbol (enum, UDT, ptr, etc.), and assign them to different vectors
  struct TypeSymbol : public Symbol
  {
    bool operator==(const TypeSymbol& aOther) const
    {
      return
        length == aOther.length
        && memberVariableCount == aOther.memberVariableCount
        && name == aOther.name
        && memberVariableIds == aOther.memberVariableIds;
    }

    uint64_t length{};
    uint64_t memberVariableCount{};
    std::vector<uint32_t> memberVariableIds{};
  };

  // https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/cv-call-e?view=vs-2022
  // TODO: MS docs out of date, since there are a lot more values in CV_call_e?
  enum class CallingConvention : uint8_t
  {
    kNearC = 0,
    kNearFast,
    kNearStd,
    kNearSys,
    kThiscall,
    kCLRCall,

    kUnknown = 0xFF
  };

  struct FunctionSymbol : public Symbol
  {
    uint32_t returnTypeId{};
    uint32_t argumentCount{};
    std::vector<uint32_t> argumentTypeIds{};
    CallingConvention callingConvention{ CallingConvention::kUnknown };
    size_t virtualAddress{};
  };

  void SetSerializer(ISerializer::Type aType);

  ISerializer::SerializeResult Serialize(const char* apOutputFileNoExtension);

private:
  void PurgeDuplicateTypes();

  std::unique_ptr<ISerializer> pSerializer = nullptr;

public:
  Header header{};
  std::unordered_map<uint32_t, TypeSymbol> typeSymbols{};
  std::unordered_map<uint32_t, FunctionSymbol> functionSymbols{};
};
