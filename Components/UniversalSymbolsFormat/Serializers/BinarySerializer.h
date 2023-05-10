#pragma once

#include "ISerializer.h"

#include <Writer.h>

class BinarySerializer final : public ISerializer
{
public:
	void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override;

protected:
	bool SerializeHeader() override;
	bool SerializeTypeSymbols() override;
	bool SerializeEnumSymbols() override;
	bool SerializeFunctionSymbols() override;
	bool WriteToFile() override;

	Writer writer{};
};