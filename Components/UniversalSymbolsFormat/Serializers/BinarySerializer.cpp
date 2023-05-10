#include "BinarySerializer.h"

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

	return SR::kUnknown;
}
