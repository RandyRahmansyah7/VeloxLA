#include <benchmark/benchmark.h>
#include <veloxla/veloxla.hpp>

static void BM_Elementwise(benchmark::State& state) {
    // TODO: implementasi benchmark sesuai roadmap.
    for (auto _ : state) {
        // placeholder workload
        volatile int x = 0;
        benchmark::DoNotOptimize(x);
    }
}
BENCHMARK(BM_Elementwise);

BENCHMARK_MAIN();
