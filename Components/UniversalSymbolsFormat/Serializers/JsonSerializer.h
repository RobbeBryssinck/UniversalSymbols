#pragma once

#include "ISerializer.h"

#include <json.hpp>

class JsonSerializer final : public ISerializer
{
public:
	void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override;

protected:
	bool SerializeHeader() override;
	bool SerializeTypeSymbols() override;
	bool SerializeFunctionSymbols() override;
	bool WriteToFile() override;

private:
	nlohmann::json j = nlohmann::json({});
};