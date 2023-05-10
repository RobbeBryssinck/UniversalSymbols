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

  if (argc != 2)
  {
    spdlog::info("Usage: {} [path_to_pdb]", argv[0]);
    exit(1);
  }

  std::string target = argv[1];
  auto pUsymResult = DiaInterface::CreateUsymFromFile(target.c_str());

  //auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\Users\Someone\Desktop\rs-module-lexer-main\rs-module-lexer-main\target\debug\binding.pdb)");
  //auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\dev\crafting_interpreters_rust\target\debug\crafting_interpreters_rust.pdb)");
  //auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\Users\Someone\source\repos\TestApp1\x64\Debug\TestApp1.pdb)");
  //auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\dev\rust_args\target\debug\rust_args.pdb)");
  //auto pUsymResult = DiaInterface::CreateUsymFromFile(R"(C:\dev\rust_sample\target\debug\rust_sample.pdb)");

  if (!pUsymResult)
  {
    spdlog::error("Failed to load symbols from DIA.");
    exit(1);
  }

  USYM& usym = pUsymResult.value();

  usym.SetSerializer(ISerializer::Type::kJson);

  std::string output = target.substr(0, target.find_last_of("."));
  auto result = usym.Serialize(output.c_str());

  //auto result = usym.Serialize(R"(C:\Users\Someone\Desktop\rs-module-lexer-main\rs-module-lexer-main\target\debug\binding)");
  //auto result = usym.Serialize(R"(C:\dev\crafting_interpreters_rust\target\debug\crafting_interpreters_rust)");
  //auto result = usym.Serialize(R"(C:\Users\Someone\source\repos\TestApp1\x64\Debug\TestApp1)");
  //auto result = usym.Serialize(R"(C:\dev\rust_args\target\debug\rust_args)");
  //auto result = usym.Serialize(R"(C:\dev\rust_sample\target\debug\rust_sample)");
}
