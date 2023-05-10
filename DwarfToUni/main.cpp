#include <spdlog/spdlog.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "ElfInterface.h"

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

  auto pUsymResult = ElfInterface::CreateUsymFromFile(R"(C:\Users\Someone\Desktop\linux_rust_sample\target\debug\rust_linux_sample)");
  if (!pUsymResult)
  {
    spdlog::error("Failed to load USYM format from ELF binary.");
    exit(1);
  }

  USYM& usym = pUsymResult.value();

  usym.SetSerializer(ISerializer::Type::kJson);
  auto result = usym.Serialize();
}
