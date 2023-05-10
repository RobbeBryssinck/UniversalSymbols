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
  auto result = usym.Serialize(R"(C:\Users\Someone\Desktop\cvdump\rust_sample)");
}
