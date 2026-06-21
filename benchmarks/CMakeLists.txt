# benchmarks/CMakeLists.txt
#
# Setiap file benchmark dikompilasi jadi executable TERPISAH (bukan digabung
# satu binary besar). Alasannya: lo mau bisa jalanin satu kelas benchmark
# spesifik (misal cuma bench_expression_fusion) tanpa nunggu seluruh suite
# selesai — penting pas iterasi cepat optimasi satu area.

function(veloxla_add_benchmark name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE
        VeloxLA::veloxla
        benchmark::benchmark
    )
    target_compile_features(${name} PRIVATE cxx_std_17)

    # Benchmark WAJIB Release-level optimization meski user lupa set
    # CMAKE_BUILD_TYPE — benchmark yang jalan di -O0 itu data sampah.
    target_compile_options(${name} PRIVATE
        $<$<NOT:$<CONFIG:Release>>:-O3>
    )
endfunction()

veloxla_add_benchmark(bench_expression_fusion   bench_expression_fusion.cpp)
veloxla_add_benchmark(bench_elementwise         bench_elementwise.cpp)
veloxla_add_benchmark(bench_contraction         bench_contraction.cpp)
veloxla_add_benchmark(bench_matmul_vs_backend   bench_matmul_vs_backend.cpp)

# Target gabungan opsional: `cmake --build . --target run_all_benchmarks`
# buat jalanin semua sekaligus dan dump hasil ke results/ dengan timestamp.
add_custom_target(run_all_benchmarks
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/benchmarks/results
    COMMAND bench_expression_fusion --benchmark_out=${CMAKE_SOURCE_DIR}/benchmarks/results/expression_fusion.json --benchmark_out_format=json
    COMMAND bench_elementwise --benchmark_out=${CMAKE_SOURCE_DIR}/benchmarks/results/elementwise.json --benchmark_out_format=json
    COMMAND bench_contraction --benchmark_out=${CMAKE_SOURCE_DIR}/benchmarks/results/contraction.json --benchmark_out_format=json
    COMMAND bench_matmul_vs_backend --benchmark_out=${CMAKE_SOURCE_DIR}/benchmarks/results/matmul_vs_backend.json --benchmark_out_format=json
    DEPENDS bench_expression_fusion bench_elementwise bench_contraction bench_matmul_vs_backend
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Menjalankan seluruh VeloxLA benchmark suite dan menyimpan hasil JSON"
)
