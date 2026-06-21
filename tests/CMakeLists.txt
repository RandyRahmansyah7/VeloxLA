# tests/CMakeLists.txt
#
# Setiap subfolder (core, expression, simd, ops) merepresentasikan satu unit
# area pengujian. File ditambahkan eksplisit (bukan GLOB) supaya CMake
# re-run otomatis kalau ada file baru — GLOB butuh re-configure manual dan
# itu sumber bug "kenapa test baru gw nggak jalan" yang sering kejadian.

add_executable(veloxla_tests
    core/test_tensor.cpp
    core/test_storage.cpp
    core/test_shape.cpp

    expression/test_binary_expr.cpp
    expression/test_fusion_correctness.cpp

    simd/test_avx2_kernel.cpp
    simd/test_simd_vs_scalar.cpp

    ops/test_contraction.cpp
    ops/test_matmul.cpp
)

target_link_libraries(veloxla_tests PRIVATE
    VeloxLA::veloxla
    Catch2::Catch2WithMain
)

target_compile_features(veloxla_tests PRIVATE cxx_std_17)

# Integrasi ke CTest — Catch2 punya CMake module sendiri buat auto-discover
# test case individual (bukan cuma satu blok "veloxla_tests" monolitik),
# supaya `ctest` bisa lapor per-test-case dan filter by name.
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(veloxla_tests)
