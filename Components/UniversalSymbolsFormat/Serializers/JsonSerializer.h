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

	nlohmann::json j = nlohmann::json({});
};