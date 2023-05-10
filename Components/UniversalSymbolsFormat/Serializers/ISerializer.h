#pragma once

#include <string>
#include <memory>

struct USYM;

class ISerializer
{
public:
	enum class Type
	{
		kBinary = 0,
		kJson,
	};

	enum class SerializeResult
	{
		kOk = 0,
		kUnknown,
		kSerializerUninitialized,
		kNoTargetFile,
		kNoData,
		kFileCreationFailed,

		kHeaderFailed,
		kTypeSymbolsFailed,
		kFunctionSymbolsFailed,
	};

	virtual ~ISerializer() = default;
	virtual SerializeResult SerializeToFile() = 0;
	virtual void Setup(const std::string& aTargetFileName, USYM* apUsym) = 0;

protected:
	std::string targetFileName{};
	USYM* pUsym{};
};
