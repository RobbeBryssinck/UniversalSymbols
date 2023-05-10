#include "USYM.h"

void USYM::Header::Serialize(Writer& aWriter) const
{
	aWriter.Write(magic);
}

void USYM::Header::Deserialize(Reader& aReader)
{
	aReader.Read(magic);
}

void USYM::Serialize(Writer& aWriter) const
{
	header.Serialize(aWriter);
}

void USYM::Deserialize(Reader& aReader)
{
	header.Deserialize(aReader);
}
