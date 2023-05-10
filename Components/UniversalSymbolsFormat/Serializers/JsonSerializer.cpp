#include "JsonSerializer.h"

#include "../USYM.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

using json = nlohmann::json;

void JsonSerializer::Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym)
{
	targetFileName = aTargetFileNameNoExtension + ".json";
	pUsym = apUsym;
}

bool JsonSerializer::WriteToFile()
{
	std::ofstream file(targetFileName);
	if (file.fail())
		return false;

	file << std::setw(4) << j << std::endl;

	return true;
}

bool JsonSerializer::SerializeHeader()
{
	j["magic"] = pUsym->header.magic;
	j["originalFormat"] = pUsym->header.originalFormat;
	j["architecture"] = pUsym->header.architecture;

	return true;
}

bool JsonSerializer::SerializeTypeSymbols()
{
	std::unordered_map<std::string, json> idJsonMap{};
	for (const auto& [id, typeSymbol] : pUsym->typeSymbols)
	{
		json& symbol = idJsonMap[std::to_string(id)];
		symbol["id"] = typeSymbol.id;
		symbol["name"] = typeSymbol.name;
		symbol["type"] = typeSymbol.type;
		symbol["length"] = typeSymbol.length;
		symbol["memberVariableCount"] = typeSymbol.memberVariableCount;
		symbol["memberVariableIds"] = typeSymbol.memberVariableIds;
	}

	j["typeSymbols"] = idJsonMap;

	return true;
}

bool JsonSerializer::SerializeFunctionSymbols()
{
	std::unordered_map<std::string, json> idJsonMap{};
	for (const auto& [id, functionSymbol] : pUsym->functionSymbols)
	{
		json& symbol = idJsonMap[std::to_string(id)];
		symbol["id"] = functionSymbol.id;
		symbol["name"] = functionSymbol.name;
		symbol["returnTypeId"] = functionSymbol.returnTypeId;
		symbol["argumentCount"] = functionSymbol.argumentCount;
		symbol["argumentTypeIds"] = functionSymbol.argumentTypeIds;
		symbol["callingConvention"] = functionSymbol.callingConvention;
		symbol["virtualAddress"] = functionSymbol.virtualAddress;
	}

	j["functionSymbols"] = idJsonMap;

	return true;
}
