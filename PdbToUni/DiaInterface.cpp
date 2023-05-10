#include "DiaInterface.h"

#include <spdlog/spdlog.h>

#include <Windows.h>
#include <atlcomcli.h>
#include <dia2.h>

#include <stdexcept>
#include <memory>
#include <unordered_set>

namespace DiaInterface
{
	static CComPtr<IDiaSession> s_pSession = nullptr;
	static CComPtr<IDiaDataSource> s_pDataSource = nullptr;
	static CComPtr<IDiaSymbol> s_pGlobalScopeSymbol = nullptr;

	static std::unordered_set<uint32_t> s_symbolIndexIds{};

	void Release()
	{
		if (s_pSession)
			s_pSession.Release();
		if (s_pDataSource)
			s_pDataSource.Release();

		CoUninitialize();
	}

	void InitializeDia(const char* apFileName)
	{
    CoInitialize(NULL);

    HRESULT result = CoCreateInstance(CLSID_DiaSource,
      NULL,
      CLSCTX_INPROC_SERVER,
      __uuidof(IDiaDataSource),
      (void**)&s_pDataSource);

    if (FAILED(result))
      throw std::runtime_error("CoCreateInstance failed.");

		int fileNameLength = MultiByteToWideChar(CP_UTF8, 0, apFileName, -1, NULL, 0);
		auto fileNameUnicode = std::make_unique<WCHAR[]>(fileNameLength);
		MultiByteToWideChar(CP_UTF8, 0, apFileName, -1, fileNameUnicode.get(), fileNameLength);

    if (FAILED(s_pDataSource->loadDataFromPdb(fileNameUnicode.get())))
      throw std::runtime_error("loadDataFromPdb failed.");

		if (FAILED(s_pDataSource->openSession(&s_pSession)))
			throw std::runtime_error("openSession failed.");

		if (FAILED(s_pSession->get_globalScope(&s_pGlobalScopeSymbol)))
			throw std::runtime_error("get_globalScope failed.");
	}

	std::string GetNameFromSymbol(IDiaSymbol* apSymbol)
	{
		std::string name = "";

		BSTR pwName = nullptr;
		if (apSymbol->get_name(&pwName) == S_OK)
		{
			auto nameLen = (size_t)SysStringLen(pwName) + 1;
			char* pName = new char[nameLen];
			wcstombs(pName, pwName, nameLen);

			name = pName;

			delete[] pName;
			SysFreeString(pwName);
		}

		return name;
	}

	std::string GetBaseName(BasicType aType, size_t aSize)
	{
		switch (aType)
		{
		case BasicType::btChar:
			return "char";
		case BasicType::btWChar:
			return "wchar";
		case BasicType::btInt:
			switch (aSize)
			{
			case 1:
				return "int8_t";
			case 2:
				return "int16_t";
			case 4:
				return "int32_t";
			case 8:
				return "int64_t";
			default:
				return "int";
			}
		case BasicType::btUInt:
			switch (aSize)
			{
			case 1:
				return "uint8_t";
			case 2:
				return "uint16_t";
			case 4:
				return "uint32_t";
			case 8:
				return "uint64_t";
			default:
				return "uint";
			}
		case BasicType::btFloat:
			return "float";
		case BasicType::btBool:
			return "bool";
		case BasicType::btLong:
			return "long";
		case BasicType::btULong:
			return "unsigned long";
		case BasicType::btChar8:
			return "char8_t";
		case BasicType::btChar16:
			return "char16_t";
		case BasicType::btChar32:
			return "char32_t";
		case BasicType::btVoid:
		case BasicType::btBCD:
		case BasicType::btCurrency:
		case BasicType::btDate:
		case BasicType::btComplex:
		case BasicType::btBit:
		case BasicType::btBSTR:
		case BasicType::btHresult:
		default:
			return "void";
		}
	}

	std::string GeneratePointerName()
	{
		static size_t s_counter = 0;
		s_counter++;
		return std::format("pUnk{}", s_counter);
	}

	bool DoesSymbolExist(uint32_t aSymbolIndexId)
	{
		return s_symbolIndexIds.find(aSymbolIndexId) != s_symbolIndexIds.end();
	}

	std::optional<USYM::TypeSymbol> CreateBaseTypeSymbol(IDiaSymbol* apSymbol)
	{
		USYM::TypeSymbol symbol{};

		DWORD id = 0;
		apSymbol->get_symIndexId(&id);
		symbol.id = id;

		DWORD baseType = 0;
		apSymbol->get_baseType(&baseType);

		ULONGLONG size = 0;
		apSymbol->get_length(&size);
		symbol.length = size;

		symbol.name = GetBaseName(static_cast<BasicType>(baseType), size);

		return symbol;
	}

	std::optional<USYM::TypeSymbol> CreateUDTSymbol(IDiaSymbol* apSymbol)
	{
		USYM::TypeSymbol symbol{};

		DWORD id = 0;
		apSymbol->get_symIndexId(&id);
		symbol.id = id;

		symbol.name = GetNameFromSymbol(apSymbol);

		ULONGLONG length = 0;
		apSymbol->get_length(&length);
		symbol.length = length;

		return symbol;
	}

	std::optional<USYM::TypeSymbol> CreateEnumSymbol(IDiaSymbol* apSymbol)
	{
		USYM::TypeSymbol symbol{};

		DWORD id = 0;
		apSymbol->get_symIndexId(&id);
		symbol.id = id;

		symbol.name = GetNameFromSymbol(apSymbol);

		ULONGLONG length = 0;
		apSymbol->get_length(&length);
		symbol.length = length;

		return symbol;
	}

	std::optional<USYM::TypeSymbol> CreatePointerTypeSymbol(IDiaSymbol* apSymbol)
	{
		USYM::TypeSymbol symbol{};

		DWORD id = 0;
		apSymbol->get_symIndexId(&id);
		symbol.id = id;

		symbol.name = GeneratePointerName();

		ULONGLONG length = 0;
		apSymbol->get_length(&length);
		symbol.length = length;

		return symbol;
	}

	std::optional<USYM::TypeSymbol> CreateTypeSymbol(IDiaSymbol* apSymbol)
	{
		DWORD symTag = 0;
		HRESULT result = apSymbol->get_symTag(&symTag);
		if (result != S_OK)
			return std::nullopt;

		std::optional<USYM::TypeSymbol> symbol{};

		switch (symTag)
		{
		case SymTagBaseType:
			symbol = CreateBaseTypeSymbol(apSymbol);
			break;
		case SymTagUDT:
			symbol = CreateUDTSymbol(apSymbol);
			break;
		case SymTagEnum:
			symbol = CreateEnumSymbol(apSymbol);
			break;
		case SymTagPointerType:
			symbol = CreatePointerTypeSymbol(apSymbol);
			break;
		default:
			symbol = std::nullopt;
		}

		if (!symbol)
			return std::nullopt;

		s_symbolIndexIds.insert(symbol->id);

		return symbol;
	}

	// TODO: duplicate symbols?
	// TODO: symplify these builders, lots of duplicate code
	void BuildBaseTypeList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagBaseType, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pType(rgelt);

				auto result = CreateTypeSymbol(pType);
				if (!result)
					spdlog::error("Failed to create base type symbol.");

				aUsym.typeSymbols.push_back(*result);
			}
		}
	}

	void BuildUserDefinedTypeList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagUDT, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pType(rgelt);

				auto result = CreateTypeSymbol(pType);
				if (!result)
					spdlog::error("Failed to create user defined type symbol.");

				aUsym.typeSymbols.push_back(*result);
			}
		}
	}

	void BuildEnumTypeList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagEnum, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pType(rgelt);

				auto result = CreateTypeSymbol(pType);
				if (!result)
					spdlog::error("Failed to create enum symbol.");

				aUsym.typeSymbols.push_back(*result);
			}
		}
	}

	void BuildPointerTypeList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagPointerType, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pType(rgelt);

				auto result = CreateTypeSymbol(pType);
				if (!result)
					spdlog::error("Failed to create pointer type symbol.");

				aUsym.typeSymbols.push_back(*result);
			}
		}
	}

	void BuildFunctionList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagFunction, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pFunction(rgelt);
				HRESULT result = 0;

				USYM::FunctionSymbol& symbol = aUsym.functionSymbols.emplace_back();

				DWORD id = 0;
				pFunction->get_symIndexId(&id);
				symbol.id = id;

				symbol.name = GetNameFromSymbol(pFunction);

				IDiaSymbol* pFunctionType = nullptr;
				pFunction->get_type(&pFunctionType);
				if (!pFunctionType)
				{
					spdlog::warn("No function type for {}, skipping...", symbol.name);
					continue;
				}

				IDiaSymbol* pReturnType = nullptr;
				result = pFunctionType->get_type(&pReturnType);

				if (pReturnType && result == S_OK)
				{
					DWORD returnTypeId = 0;
					result = pReturnType->get_symIndexId(&returnTypeId);
					if (result == S_OK)
						symbol.returnTypeId = returnTypeId;

					if (!DoesSymbolExist(returnTypeId))
					{
						auto result = CreateTypeSymbol(pReturnType);
						if (!result)
						{
							DWORD symTag = 0;
							pReturnType->get_symTag(&symTag);
							spdlog::error("Failed to create return type symbol {}.", symTag);
						}
						else
							aUsym.typeSymbols.push_back(*result);
					}
				}

				DWORD argumentCount = 0;
				result = pFunctionType->get_count(&argumentCount);
				if (result == S_OK)
					symbol.argumentCount = argumentCount;

				CComPtr<IDiaEnumSymbols> pEnumArgs = nullptr;
				if (SUCCEEDED(pFunctionType->findChildren(SymTagFunctionArgType, nullptr, nsNone, &pEnumArgs)) && symbol.name == "rust_sample::test_func5")
				{
					IDiaSymbol* pCurrentArg = nullptr;
					ULONG argNum = 0;

					while (SUCCEEDED(pEnumArgs->Next(1, &pCurrentArg, &argNum)) && (argNum == 1))
					{
						CComPtr<IDiaSymbol> pCurrentArgManaged(pCurrentArg);

						IDiaSymbol* pArgumentType = nullptr;
						pCurrentArgManaged->get_type(&pArgumentType);
						
						DWORD argTypeId = 0;
						pArgumentType->get_symIndexId(&argTypeId);

						if (!DoesSymbolExist(argTypeId))
						{
							auto result = CreateTypeSymbol(pArgumentType);
							if (!result)
								spdlog::error("Failed to create argument type symbol.");
							else
							{
								aUsym.typeSymbols.push_back(*result);
								symbol.argumentTypeIds.push_back(argTypeId);
							}
						}
					}
				}

				DWORD callingConvention = 0;
				result = pFunctionType->get_callingConvention(&callingConvention);
				if (result == S_OK)
				{
					using CC = USYM::CallingConvention;
					switch (callingConvention)
					{
					case CV_call_e::CV_CALL_NEAR_C:
						symbol.callingConvention = CC::kNearC;
						break;
					case CV_call_e::CV_CALL_NEAR_FAST:
						symbol.callingConvention = CC::kNearFast;
						break;
					case CV_call_e::CV_CALL_NEAR_STD:
						symbol.callingConvention = CC::kNearStd;
						break;
					case CV_call_e::CV_CALL_NEAR_SYS:
						symbol.callingConvention = CC::kNearSys;
						break;
					case CV_call_e::CV_CALL_THISCALL:
						symbol.callingConvention = CC::kThiscall;
						break;
					case CV_call_e::CV_CALL_CLRCALL:
						symbol.callingConvention = CC::kCLRCall;
						break;
					default:
						symbol.callingConvention = CC::kUnknown;
					}
				}

				DWORD relativeVirtualAddress = 0;
				result = pFunction->get_addressOffset(&relativeVirtualAddress);
				if (result == S_OK)
					symbol.relativeVirtualAddress = relativeVirtualAddress;
				
				s_symbolIndexIds.insert(symbol.id);
			}
		}
	}

	void BuildHeader(USYM& aUsym)
	{
		aUsym.header.originalFormat = USYM::OriginalFormat::kPdb;
		
		DWORD machineType = 0;
		HRESULT result = s_pGlobalScopeSymbol->get_machineType(&machineType);
		if (result == S_OK)
		{
			switch (machineType)
			{
			case IMAGE_FILE_MACHINE_I386: // TODO: not sure about this one
				aUsym.header.architecture = USYM::Architecture::kX86;
				break;
			case IMAGE_FILE_MACHINE_AMD64:
				aUsym.header.architecture = USYM::Architecture::kX86_64;
				break;
			case IMAGE_FILE_MACHINE_ARM:
				aUsym.header.architecture = USYM::Architecture::kArm32;
				break;
			case IMAGE_FILE_MACHINE_ARM64:
				aUsym.header.architecture = USYM::Architecture::kArm64;
				break;
			default:
				aUsym.header.architecture = USYM::Architecture::kUnknown;
			}
		}
	}

	std::optional<USYM> CreateUsymFromFile(const char* apFileName)
	{
		try
		{
			InitializeDia(apFileName);
			
			USYM usym{};

			BuildHeader(usym);
			BuildBaseTypeList(usym);
			BuildUserDefinedTypeList(usym);
			BuildEnumTypeList(usym);
			BuildPointerTypeList(usym);
			BuildFunctionList(usym);
			// TODO: reduce duplicates

			return usym;
		}
		catch (const std::exception& e)
		{
			spdlog::error("{}", e.what());
			Release();
			return std::nullopt;
		}
	}
}
