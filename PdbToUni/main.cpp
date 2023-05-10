#include <spdlog/spdlog.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <UniversalSymbolsFormat/USYM.h>
#include "DiaInterface.h"

void InitializeLogger()
{
  auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console->set_pattern("%^[%H:%M:%S] [%l]%$ %v");
  auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ console });
  //logger->set_level(spdlog::level::debug);
  set_default_logger(logger);
}

int main(int argc, char* argv[])
{
  InitializeLogger();

  auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\Users\Someone\Desktop\cvdump\rust_sample.pdb)");
  if (!pUsymResult)
  {
    spdlog::error("Failed to load symbols from DIA.");
    exit(1);
  }

  USYM& usym = pUsymResult.value();

  usym.SetSerializer(ISerializer::Type::kJson);
  //auto result = usym.Serialize();

#if 0
  CoInitialize(NULL);

  CComPtr<IDiaDataSource> pSource;
  auto hr = CoCreateInstance(CLSID_DiaSource,
    NULL,
    CLSCTX_INPROC_SERVER,
    __uuidof(IDiaDataSource),
    (void**)&pSource);

  if (FAILED(hr))
  {
    spdlog::error("Could not CoCreate CLSID_DiaSource. Register msdia80.dll.");
    exit(1);
  }

  wchar_t wszFilename[] = L"C:\\Users\\Someone\\Desktop\\cvdump\\rust_sample.pdb";
  if (FAILED(pSource->loadDataFromPdb(wszFilename)))
  {
    if (FAILED(pSource->loadDataForExe(wszFilename, NULL, NULL)))
    {
      spdlog::error("loadDataFromPdb/Exe");
      exit(1);
    }
  }

  CComPtr<IDiaSession> psession;
  if (FAILED(pSource->openSession(&psession)))
  {
    spdlog::error("openSession");
    exit(1);
  }

  CComPtr<IDiaSymbol> pglobal;
  if (FAILED(psession->get_globalScope(&pglobal)))
  {
    spdlog::error("get_globalScope");
    exit(1);
  }

  CComPtr<IDiaEnumTables> pTables;
  if (FAILED(psession->getEnumTables(&pTables)))
  {
    spdlog::error("getEnumTables");
  }

  spdlog::info("Hello World!");

  ULONG celt;
  CComPtr<IDiaTable> pTable;
  while (SUCCEEDED(hr = pTables->Next(1, &pTable, &celt)) && celt == 1)
  {
    break;
  }

  CoUninitialize();
#endif
}
