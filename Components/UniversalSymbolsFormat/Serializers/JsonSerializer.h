#pragma once

#include "ISerializer.h"

#include <json.hpp>

class JsonSerializer final : public ISerializer
{
public:
	SerializeResult SerializeToFile() override;
	void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override;

private:
	bool SerializeHeader();
	bool SerializeTypeSymbols();
	bool SerializeFunctionSymbols();

	nlohmann::json j = nlohmann::json({});
};