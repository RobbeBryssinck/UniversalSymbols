#include "USYM.h"

#include "Serializers/BinarySerializer.h"
#include "Serializers/JsonSerializer.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

void USYM::SetSerializer(ISerializer::Type aType)
{
  switch (aType)
  {
  case ISerializer::Type::kBinary:
    pSerializer = std::make_unique<BinarySerializer>();
    break;
  case ISerializer::Type::kJson:
    pSerializer = std::make_unique<JsonSerializer>();
    break;
  default:
    throw std::runtime_error("No serializer for type found.");
  }
}

ISerializer::SerializeResult USYM::Serialize(const char* apOutputFileNoExtension)
{
  using SR = ISerializer::SerializeResult;

  if (!pSerializer)
    return SR::kSerializerUninitialized;

  pSerializer->Setup(apOutputFileNoExtension, this);

  VerifyTypeIds();

  spdlog::info("Type symbol count before: {}", typeSymbols.size());

  PurgeDuplicateTypes();

  spdlog::info("Type symbol count after: {}", typeSymbols.size());

  // TODO: write verification function that checks if all ids exist
  VerifyTypeIds();

  return pSerializer->SerializeToFile();
}

void USYM::PurgeDuplicateTypes()
{
  std::unordered_map<uint32_t, uint32_t> oldToNew{};
  std::unordered_map<size_t, uint32_t> hashesToIds{};

  for (const auto& [id, symbol] : typeSymbols)
  {
    size_t hash = std::hash<USYM::TypeSymbol>()(symbol);
    const auto fetchedId = hashesToIds.find(hash);
    if (fetchedId != hashesToIds.end())
    {
      oldToNew[id] = fetchedId->second;
    }
    else
    {
      hashesToIds[hash] = id;
    }
  }

  for (auto& [id, symbol] : typeSymbols)
  {
    for (auto& field : symbol.fields)
    {
      auto oldNewPair = oldToNew.find(field.underlyingTypeId);
      if (oldNewPair == oldToNew.end())
        continue;

      field.underlyingTypeId = oldNewPair->second;
    }
  }

  for (auto& [functionId, function] : functionSymbols)
  {
    auto oldNewPair = oldToNew.find(function.returnTypeId);
    if (oldNewPair == oldToNew.end())
      continue;

    function.returnTypeId = oldNewPair->second;

    for (auto& id : function.argumentTypeIds)
    {
      auto oldNewPair = oldToNew.find(id);
      if (oldNewPair == oldToNew.end())
        continue;

      id = oldNewPair->second;
    }
  }

  std::erase_if(typeSymbols, [&oldToNew](const auto& item) {
    auto const& [id, symbol] = item;
    return oldToNew.contains(id);
  });
}

// TODO: why are some return types null?
bool USYM::VerifyTypeIds()
{
  bool purity = true;

  for (const auto& [id, symbol] : functionSymbols)
  {
    if (symbol.returnTypeId != 0 && typeSymbols.find(symbol.returnTypeId) == typeSymbols.end())
    {
      purity = false;
      spdlog::warn("Return type id of function {} not found: {}", symbol.id, symbol.returnTypeId);
    }

    for (const auto argumentTypeId : symbol.argumentTypeIds)
    {
      if (argumentTypeId != 0 && typeSymbols.find(argumentTypeId) == typeSymbols.end())
      {
        purity = false;
        spdlog::warn("Argument type id of function {} not found: {}", symbol.id, argumentTypeId);
      }
    }
  }

  for (const auto& [id, symbol] : typeSymbols)
  {
    for (const auto& fieldSymbol : symbol.fields)
    {
      if (fieldSymbol.underlyingTypeId != 0 && typeSymbols.find(fieldSymbol.underlyingTypeId) == typeSymbols.end())
      {
        purity = false;
        spdlog::warn("Field type id of type {} not found: {}", id, fieldSymbol.underlyingTypeId);
      }
    }
  }

  return purity;
}
