#include "BinarySerializer.h"

#include "../USYM.h"

using SR = ISerializer::SerializeResult;

void BinarySerializer::Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym)
{
	targetFileName = aTargetFileNameNoExtension + ".usym";
	pUsym = apUsym;
}

SR BinarySerializer::SerializeToFile()
{
	if (targetFileName == "")
		return SR::kNoTargetFile;

	if (!pUsym)
		return SR::kNoData;

	if (!SerializeHeader())
		return SR::kHeaderFailed;

	if (!SerializeTypeSymbols())
		return SR::kTypeSymbolsFailed;

	if (!writer.WriteToFile(targetFileName))
		return SR::kFileCreationFailed;

	return SR::kOk;
}

bool BinarySerializer::SerializeHeader()
{
	writer.Write(pUsym->header.magic);

	return true;
}

bool BinarySerializer::SerializeTypeSymbols()
{
	const size_t symbolCount = pUsym->typeSymbols.size();
	writer.Write(symbolCount);

	for (const auto& typeSymbol : pUsym->typeSymbols)
	{
		writer.Write(typeSymbol);
	}

	return true;
}
