#include "JsonSerializer.h"

#include "../USYM.h"

#include <filesystem>
#include <fstream>

using SR = ISerializer::SerializeResult;
using json = nlohmann::json;

void JsonSerializer::Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym)
{
	targetFileName = aTargetFileNameNoExtension + ".json";
	pUsym = apUsym;
}

SR JsonSerializer::SerializeToFile()
{
	if (targetFileName == "")
		return SR::kNoTargetFile;

	if (!pUsym)
		return SR::kNoData;

	if (!SerializeHeader())
		return SR::kHeaderFailed;

	if (!SerializeTypeSymbols())
		return SR::kTypeSymbolsFailed;

	if (!SerializeFunctionSymbols())
		return SR::kFunctionSymbolsFailed;

	std::ofstream file(targetFileName);
	if (file.fail())
		return SR::kFileCreationFailed;

	file << std::setw(4) << j << std::endl;

	return SR::kOk;
}

bool JsonSerializer::SerializeHeader()
{
	j["magic"] = pUsym->header.magic;

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
		functionSymbols.push_back(symbol);
	}

	j["functionSymbols"] = functionSymbols;

	return true;
}
