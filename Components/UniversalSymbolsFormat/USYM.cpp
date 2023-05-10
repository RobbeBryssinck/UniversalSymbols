#include "USYM.h"

#include "Serializers/BinarySerializer.h"
#include "Serializers/JsonSerializer.h"

#include <stdexcept>

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

  for (const auto& symbol1 : typeSymbols)
  {
    for (const auto& symbol2 : typeSymbols)
    {
      if (symbol1 == symbol2 && symbol1.id != symbol2.id && !oldToNew.contains(symbol1.id) && !oldToNew.contains(symbol2.id))
        oldToNew[symbol2.id] = symbol1.id;
    }
  }

  for (auto& symbol : typeSymbols)
  {
    for (auto& id : symbol.memberVariableIds)
    {
      auto oldNewPair = oldToNew.find(id);
      if (oldNewPair == oldToNew.end())
        continue;

      id = oldNewPair->second;
    }
  }

  for (auto& function : functionSymbols)
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

  typeSymbols.erase(std::remove_if(typeSymbols.begin(), typeSymbols.end(), [&oldToNew](USYM::TypeSymbol& aSymbol) {
    return oldToNew.contains(aSymbol.id);
    }), typeSymbols.end());
}
