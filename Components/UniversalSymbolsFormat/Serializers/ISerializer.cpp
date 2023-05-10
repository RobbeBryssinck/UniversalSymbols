#include "ISerializer.h"

using SR = ISerializer::SerializeResult;

ISerializer::SerializeResult ISerializer::SerializeToFile()
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

	if (!WriteToFile())
		return SR::kFileCreationFailed;

	return SR::kOk;
}