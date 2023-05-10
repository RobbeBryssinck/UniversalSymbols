#include "JsonSerializer.h"

#include "../USYM.h"

#include <filesystem>
#include <fstream>

using SR = ISerializer::SerializeResult;

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
