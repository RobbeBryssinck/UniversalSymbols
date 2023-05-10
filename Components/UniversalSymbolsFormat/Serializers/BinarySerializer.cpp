#include "BinarySerializer.h"

#include "../USYM.h"

void BinarySerializer::Setup(const std::string& aTargetFileNameNoExtension, USYM* apUsym)
{
	targetFileName = aTargetFileNameNoExtension + ".usym";
	pUsym = apUsym;
}

bool BinarySerializer::WriteToFile()
{
	return writer.WriteToFile(targetFileName);
}

bool BinarySerializer::SerializeHeader()
{
	writer.Write(pUsym->header.magic);
	writer.Write(pUsym->header.originalFormat);
	writer.Write(pUsym->header.architecture);

	return true;
}

bool BinarySerializer::SerializeTypeSymbols()
{
	const size_t symbolCount = pUsym->typeSymbols.size();
	writer.Write(symbolCount);

	for (const auto& [id, typeSymbol] : pUsym->typeSymbols)
	{
		writer.Write(typeSymbol.id);
		writer.WriteString(typeSymbol.name);
		writer.Write(typeSymbol.type);
		writer.Write(typeSymbol.length);
		writer.Write(typeSymbol.fieldCount);

		const size_t parameterCount = typeSymbol.fields.size();
		writer.Write(parameterCount);
		for (const auto& field : typeSymbol.fields)
		{
			writer.Write(field.id);
			writer.WriteString(field.name);
			writer.Write(field.underlyingTypeId);
			writer.Write(field.offset);
			writer.Write(field.isAnonymousUnion);
			writer.Write(field.unionId);
		}

		writer.Write(typeSymbol.typedefSource);
	}

	return true;
}

bool BinarySerializer::SerializeFunctionSymbols()
{
	const size_t symbolCount = pUsym->functionSymbols.size();
	writer.Write(symbolCount);

	for (const auto& [id, functionSymbol] : pUsym->functionSymbols)
	{
		writer.Write(functionSymbol.id);
		writer.WriteString(functionSymbol.name);
		writer.Write(functionSymbol.returnTypeId);
		writer.Write(functionSymbol.argumentCount);

		const size_t argumentTypeIdCount = functionSymbol.argumentTypeIds.size();
		writer.Write(argumentTypeIdCount);
		for (const auto argumentTypeId : functionSymbol.argumentTypeIds)
		{
			writer.Write(argumentTypeId);
		}

		writer.Write(functionSymbol.callingConvention);
		writer.Write(functionSymbol.virtualAddress);
	}

	return true;
}
