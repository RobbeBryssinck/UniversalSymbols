#include <benchmark/benchmark.h>

#include <DiaProcessor/DiaInterface.h>

static void BM_DiaProcessorSmall(benchmark::State& state) {
  for (auto _ : state)
  {
    DiaInterface::CreateUsymFromFile("CppApp1.pdb");
  }
}
BENCHMARK(BM_DiaProcessorSmall);

static void BM_DiaProcessorLarge(benchmark::State& state) {
  for (auto _ : state)
  {
    DiaInterface::CreateUsymFromFile("binding.pdb");
  }
}
BENCHMARK(BM_DiaProcessorLarge);

BENCHMARK_MAIN();