#pragma once

#include "Serializers/ISerializer.h"

#include <memory>
#include <optional>

struct USYM
{
  struct Header
  {
    uint32_t magic = 'MYSU';
  };

  struct TypeSymbol
  {
    uint64_t id{};
    std::string name{};
  };

  struct FunctionSymbol
  {
    uint64_t id{};
    std::string name{};
    std::optional<TypeSymbol> returnType{};
    int32_t argumentCount{};
    std::vector<TypeSymbol> arguments{};
  };

  USYM() = delete;
  USYM(ISerializer::Type aType);

  ISerializer::SerializeResult Serialize();

  std::unique_ptr<ISerializer> pSerializer;

  Header header{};
  std::vector<TypeSymbol> typeSymbols{};
  std::vector<FunctionSymbol> functionSymbols{};
};
