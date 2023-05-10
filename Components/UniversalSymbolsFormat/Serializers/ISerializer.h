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

	SerializeResult SerializeToFile();

	virtual ~ISerializer() = default;
	virtual void Setup(const std::string& aTargetFileName, USYM* apUsym) = 0;

protected:
	virtual bool SerializeHeader() = 0;
	virtual bool SerializeTypeSymbols() = 0;
	virtual bool SerializeFunctionSymbols() = 0;
	virtual bool WriteToFile() = 0;

	std::string targetFileName{};
	USYM* pUsym{};
};
