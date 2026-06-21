#include <benchmark/benchmark.h>
#include <veloxla/veloxla.hpp>

static void BM_Contraction(benchmark::State& state) {
    // TODO: implementasi benchmark sesuai roadmap.
    for (auto _ : state) {
        // placeholder workload
        volatile int x = 0;
        benchmark::DoNotOptimize(x);
    }
}
BENCHMARK(BM_Contraction);

BENCHMARK_MAIN();
