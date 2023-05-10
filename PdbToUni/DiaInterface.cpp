#include "DiaInterface.h"

#include <spdlog/spdlog.h>

#include <Windows.h>
#include <atlcomcli.h>
#include <dia2.h>

#include <stdexcept>
#include <memory>

namespace DiaInterface
{
	static CComPtr<IDiaSession> s_pSession = nullptr;
	static CComPtr<IDiaDataSource> s_pDataSource = nullptr;
	static CComPtr<IDiaSymbol> s_pGlobalScopeSymbol = nullptr;

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

				USYM::TypeSymbol& symbol = aUsym.typeSymbols.emplace_back();

				DWORD id = 0;
				pType->get_symIndexId(&id);
				symbol.id = id;

				symbol.name = GetNameFromSymbol(pType);

				ULONGLONG length = 0;
				pType->get_length(&length);
				symbol.length = length;
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
				}

				DWORD argumentCount = 0;
				result = pFunctionType->get_count(&argumentCount);
				if (result == S_OK)
					symbol.argumentCount = argumentCount;

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
			BuildUserDefinedTypeList(usym);
			BuildFunctionList(usym);

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
