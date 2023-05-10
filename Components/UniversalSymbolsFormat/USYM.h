#pragma once

#include "Serializers/ISerializer.h"

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

  struct FieldSymbol : public Symbol
  {
    bool operator==(const FieldSymbol& aOther) const
    {
      return
        underlyingTypeId == aOther.underlyingTypeId
        && offset == aOther.offset
        && name == aOther.name
        && isAnonymousUnion == aOther.isAnonymousUnion
        && unionId == aOther.unionId;
    }

    uint32_t underlyingTypeId{};
    size_t offset{};
    bool isAnonymousUnion{};
    uint32_t unionId{};
  };

  // TODO: have child classes for TypeSymbol (enum, UDT, ptr, etc.), and assign them to different vectors
  struct TypeSymbol : public Symbol
  {
    bool operator==(const TypeSymbol& aOther) const
    {
      return
        length == aOther.length
        && fieldCount == aOther.fieldCount
        && name == aOther.name
        && fields == aOther.fields
        && typedefSource == aOther.typedefSource;
    }

    enum class Type : uint8_t
    {
      kBase,
      kStruct,
      kClass,
      kUnion,
      kInterface,
      kEnum,
      kPointer,
      kTypedef,
      kArray,

      kUnknown = 0xFF
    };

    Type type{ Type::kUnknown };
    uint64_t length{};
    uint64_t fieldCount{};
    std::vector<FieldSymbol> fields{};
    uint32_t typedefSource{};
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

  const TypeSymbol& GetTypeSymbolByName(const char* apName) const;
  const FunctionSymbol& GetFunctionSymbolByName(const char* apName) const;

private:
  void PurgeDuplicateTypes();
  bool VerifyTypeIds();

  std::unique_ptr<ISerializer> pSerializer = nullptr;

public:
  Header header{};
  std::unordered_map<uint32_t, TypeSymbol> typeSymbols{};
  std::unordered_map<uint32_t, FunctionSymbol> functionSymbols{};
};

namespace std
{
  template <> class hash<USYM::FieldSymbol>
  {
  public:
    size_t operator()(const USYM::FieldSymbol& aSymbol) const
    {
      // TODO: verify whether this works.
      return hash<uint32_t>()(aSymbol.underlyingTypeId);
    }
  };

  template <> class hash<USYM::TypeSymbol>
  {
  public:
    size_t operator()(const USYM::TypeSymbol& aSymbol) const
    {
      size_t symbolHash = hash<uint64_t>()(aSymbol.length) ^ hash<uint64_t>()(aSymbol.fieldCount);
      for (const auto& field : aSymbol.fields)
        symbolHash ^= hash<USYM::FieldSymbol>()(field);
      return symbolHash ^ hash<std::string>()(aSymbol.name);
    }
  };
} // namespace std
