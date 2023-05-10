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

	bool BuildUserDefinedTypeList(USYM& aUsym)
	{
		CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
		if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagUDT, nullptr, nsNone, &pCurrentSymbol)))
		{
			IDiaSymbol* rgelt = nullptr;
			ULONG pceltFetched = 0;

			while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
			{
				CComPtr<IDiaSymbol> pChildSymbol(rgelt);

				USYM::TypeSymbol& symbol = aUsym.typeSymbols.emplace_back();

				DWORD id = 0;
				pChildSymbol->get_symIndexId(&id);
				symbol.id = id;

				BSTR pwName = nullptr;
				if (pChildSymbol->get_name(&pwName) == S_OK)
				{
					auto nameLen = (size_t)SysStringLen(pwName) + 1;
					char* pName = new char[nameLen];
					wcstombs(pName, pwName, nameLen);

					symbol.name = pName;

					delete[] pName;
					SysFreeString(pwName);
				}

				spdlog::info("{}: {}", symbol.id, symbol.name);
			}
		}

		return true;
	}

	std::optional<USYM> CreateUsymFromFile(const char* apFileName)
	{
		try
		{
			InitializeDia(apFileName);
			
			USYM usym{};

			BuildUserDefinedTypeList(usym);

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
