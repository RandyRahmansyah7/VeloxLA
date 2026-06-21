# VeloxLA

Header-only C++ tensor library berfokus pada performa CPU murni lewat
expression-template fusion, cache-aware tensor contraction, dan SIMD manual
(AVX2/AVX-512). Delegasi opsional ke OpenBLAS/MKL/BLIS untuk operasi GEMM
murni.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
```

## Opsi CMake

| Opsi | Default | Keterangan |
|---|---|---|
| `VELOXLA_BUILD_TESTS` | ON | Build Catch2 test suite |
| `VELOXLA_BUILD_BENCHMARKS` | ON | Build Google Benchmark suite |
| `VELOXLA_BUILD_EXAMPLES` | ON | Build contoh program |
| `VELOXLA_ENABLE_AVX512` | OFF | Aktifkan kernel AVX-512 |
| `VELOXLA_ENABLE_OPENMP` | ON | Aktifkan threading OpenMP |
| `VELOXLA_USE_OPENBLAS` | OFF | Dispatch matmul ke OpenBLAS |
| `VELOXLA_USE_MKL` | OFF | Dispatch matmul ke Intel MKL |
| `VELOXLA_USE_BLIS` | OFF | Dispatch matmul ke AMD BLIS/AOCL |
| `VELOXLA_NATIVE_ARCH` | OFF | Build dengan `-march=native` (non-portable) |

Lihat `docs/design/` untuk rasional arsitektur.
