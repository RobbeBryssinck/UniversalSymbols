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

  PurgeDuplicateTypes();

  return pSerializer->SerializeToFile();
}

void USYM::PurgeDuplicateTypes()
{
  std::unordered_map<uint32_t, uint32_t> oldToNew{};

  for (const auto& [id1, symbol1] : typeSymbols)
  {
    for (const auto& [id2, symbol2] : typeSymbols)
    {
      if (symbol1 == symbol2 && id1 != id2 && !oldToNew.contains(id1) && !oldToNew.contains(id2))
        oldToNew[id2] = id1;
    }
  }

  for (auto& [id, symbol] : typeSymbols)
  {
    for (auto& memberId : symbol.memberVariableIds)
    {
      auto oldNewPair = oldToNew.find(memberId);
      if (oldNewPair == oldToNew.end())
        continue;

      memberId = oldNewPair->second;
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
