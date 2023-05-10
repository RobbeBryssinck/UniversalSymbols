#include "JsonSerializer.h"

#include "../USYM.h"

#include <filesystem>
#include <fstream>

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
	json typeSymbols = json::array();
	for (const auto& typeSymbol : pUsym->typeSymbols)
	{
		json symbol = json::object();
		symbol["id"] = typeSymbol.id;
		symbol["name"] = typeSymbol.name;
		symbol["length"] = typeSymbol.length;
		symbol["memberVariableCount"] = typeSymbol.memberVariableCount;
		symbol["memberVariableIds"] = typeSymbol.memberVariableIds;
		typeSymbols.push_back(symbol);
	}

	j["typeSymbols"] = typeSymbols;

	return true;
}

bool JsonSerializer::SerializeFunctionSymbols()
{
	json functionSymbols = json::array();
	for (const auto& functionSymbol : pUsym->functionSymbols)
	{
		json symbol = json::object();
		symbol["id"] = functionSymbol.id;
		symbol["name"] = functionSymbol.name;
		symbol["returnTypeId"] = functionSymbol.returnTypeId;
		symbol["argumentCount"] = functionSymbol.argumentCount;
		symbol["argumentTypeIds"] = functionSymbol.argumentTypeIds;
		symbol["callingConvention"] = functionSymbol.callingConvention;
		symbol["virtualAddress"] = functionSymbol.virtualAddress;
		functionSymbols.push_back(symbol);
	}

	j["functionSymbols"] = functionSymbols;

	return true;
}
