#pragma once

#include "ISerializer.h"

class BinarySerializer final : public ISerializer
{
public:
	SerializeResult SerializeToFile() override;
	void Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym) override;
};