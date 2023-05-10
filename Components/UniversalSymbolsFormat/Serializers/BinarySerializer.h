#pragma once

#include "ISerializer.h"

#include <Writer.h>

class BinarySerializer final : public ISerializer
{
public:
	SerializeResult SerializeToFile() override;
	void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override;

private:
	bool SerializeHeader();
	bool SerializeTypeSymbols();

	Writer writer{};
};