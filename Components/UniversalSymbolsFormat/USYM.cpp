#include "USYM.h"

#include "Serializers/BinarySerializer.h"
#include "Serializers/JsonSerializer.h"

#include <stdexcept>

USYM::USYM(ISerializer::Type aType)
{
	switch (aType)
	{
	case ISerializer::Type::kBinary:
		pSerializer = std::make_unique<BinarySerializer>();
		break;
	case ISerializer::Type::kJson:
		pSerializer = std::make_unique<JsonSerializer>();
		break;
	default:
		throw std::runtime_error("No serializer for type found.");
	}
}

ISerializer::SerializeResult USYM::Serialize()
{
	using SR = ISerializer::SerializeResult;

	if (!pSerializer)
		return SR::kSerializerUninitialized;

	pSerializer->Setup("test", this);

	return pSerializer->SerializeToFile();
}
