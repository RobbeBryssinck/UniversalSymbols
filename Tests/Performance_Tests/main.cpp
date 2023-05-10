#include <benchmark/benchmark.h>

#include <DiaProcessor/DiaInterface.h>
#include <UniversalSymbolsFormat/Serializers/ISerializer.h>

static void BM_DiaProcessorSmall(benchmark::State& state) {
  for (auto _ : state)
  {
    DiaInterface::CreateUsymFromFile("CppApp1.pdb");
  }
}
BENCHMARK(BM_DiaProcessorSmall)->Unit(benchmark::kMillisecond);

static void BM_DiaProcessorLarge(benchmark::State& state) {
  for (auto _ : state)
  {
    DiaInterface::CreateUsymFromFile("binding.pdb");
  }
}
BENCHMARK(BM_DiaProcessorLarge)->Unit(benchmark::kMillisecond);

static void BM_JsonSerializerSmall(benchmark::State& state) {
  USYM usym = DiaInterface::CreateUsymFromFile("CppApp1.pdb").value();
  usym.SetSerializer(ISerializer::Type::kJson);

  for (auto _ : state)
  {
    usym.Serialize("CppApp1");
  }
}
BENCHMARK(BM_JsonSerializerSmall)->Unit(benchmark::kMillisecond);

static void BM_JsonSerializerLarge(benchmark::State& state) {
  USYM usym = DiaInterface::CreateUsymFromFile("binding.pdb").value();
  usym.SetSerializer(ISerializer::Type::kJson);

  for (auto _ : state)
  {
    usym.Serialize("binding");
  }
}
BENCHMARK(BM_JsonSerializerLarge)->Unit(benchmark::kMillisecond);

static void BM_BinarySerializerSmall(benchmark::State& state) {
  USYM usym = DiaInterface::CreateUsymFromFile("CppApp1.pdb").value();
  usym.SetSerializer(ISerializer::Type::kBinary);

  for (auto _ : state)
  {
    usym.Serialize("CppApp1");
  }
}
BENCHMARK(BM_BinarySerializerSmall)->Unit(benchmark::kMillisecond);

static void BM_BinarySerializerLarge(benchmark::State& state) {
  USYM usym = DiaInterface::CreateUsymFromFile("binding.pdb").value();
  usym.SetSerializer(ISerializer::Type::kBinary);

  for (auto _ : state)
  {
    usym.Serialize("binding");
  }
}
BENCHMARK(BM_BinarySerializerLarge)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();