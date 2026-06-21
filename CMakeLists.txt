cmake_minimum_required(VERSION 3.20)

project(VeloxLA
    VERSION 0.1.0
    DESCRIPTION "Header-only C++ tensor library with expression-template fusion"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Project-wide settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Selalu generate compile_commands.json buat tooling (clangd, dll)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default ke Release build kalau user nggak spesifikasikan
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(VELOXLA_BUILD_TESTS "Build Catch2 unit tests" ON)
option(VELOXLA_BUILD_BENCHMARKS "Build Google Benchmark suite" ON)
option(VELOXLA_BUILD_EXAMPLES "Build example programs" ON)

option(VELOXLA_ENABLE_AVX512 "Enable AVX-512 kernels (requires CPU support)" OFF)
option(VELOXLA_ENABLE_OPENMP "Enable OpenMP threading" ON)

option(VELOXLA_USE_OPENBLAS "Dispatch matmul to OpenBLAS backend" OFF)
option(VELOXLA_USE_MKL "Dispatch matmul to Intel MKL backend" OFF)
option(VELOXLA_USE_BLIS "Dispatch matmul to AMD BLIS/AOCL backend" OFF)

# ---------------------------------------------------------------------------
# Core library target (header-only interface library)
# ---------------------------------------------------------------------------
add_library(veloxla INTERFACE)
add_library(VeloxLA::veloxla ALIAS veloxla)

target_include_directories(veloxla INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(veloxla INTERFACE cxx_std_17)

# Compiler warnings & SIMD/arch flags (lihat cmake/CompilerWarnings.cmake)
include(CompilerWarnings)
veloxla_set_warnings(veloxla)

include(ArchFlags)
veloxla_set_arch_flags(veloxla)

# ---------------------------------------------------------------------------
# Threading (OpenMP)
# ---------------------------------------------------------------------------
if(VELOXLA_ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(veloxla INTERFACE OpenMP::OpenMP_CXX)
        target_compile_definitions(veloxla INTERFACE VELOXLA_HAS_OPENMP)
    else()
        message(WARNING "VELOXLA_ENABLE_OPENMP=ON tapi OpenMP tidak ditemukan di sistem ini")
    endif()
endif()

# ---------------------------------------------------------------------------
# Optional BLAS-family backends (runtime dispatch — lihat backend/backend_dispatch.cpp)
# ---------------------------------------------------------------------------
set(VELOXLA_BACKEND_SOURCES "")

if(VELOXLA_USE_OPENBLAS)
    find_package(OpenBLAS REQUIRED)
    target_link_libraries(veloxla INTERFACE OpenBLAS::OpenBLAS)
    target_compile_definitions(veloxla INTERFACE VELOXLA_USE_OPENBLAS)
endif()

if(VELOXLA_USE_MKL)
    find_package(MKL REQUIRED)
    target_link_libraries(veloxla INTERFACE MKL::MKL)
    target_compile_definitions(veloxla INTERFACE VELOXLA_USE_MKL)
endif()

if(VELOXLA_USE_BLIS)
    find_package(BLIS REQUIRED)
    target_link_libraries(veloxla INTERFACE BLIS::BLIS)
    target_compile_definitions(veloxla INTERFACE VELOXLA_USE_BLIS)
endif()

if(VELOXLA_USE_OPENBLAS AND VELOXLA_USE_MKL)
    message(FATAL_ERROR "Pilih salah satu backend saja: OpenBLAS atau MKL, jangan keduanya")
endif()

# backend_dispatch.cpp adalah satu-satunya compiled translation unit di core
# (deteksi CPU vendor runtime) — sisanya tetap header-only.
if(VELOXLA_USE_OPENBLAS OR VELOXLA_USE_MKL OR VELOXLA_USE_BLIS)
    add_library(veloxla_backend STATIC
        src/backend/backend_dispatch.cpp
    )
    target_link_libraries(veloxla_backend PUBLIC veloxla)
    target_include_directories(veloxla_backend PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    )
    add_library(VeloxLA::backend ALIAS veloxla_backend)
endif()

# ---------------------------------------------------------------------------
# Dependencies fetched on-demand (tests & benchmarks only — tidak mempengaruhi
# consumer yang cuma pakai veloxla sebagai header-only library)
# ---------------------------------------------------------------------------
if(VELOXLA_BUILD_TESTS OR VELOXLA_BUILD_BENCHMARKS)
    include(FetchContent)
endif()

if(VELOXLA_BUILD_TESTS)
    FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.6.0
    )
    FetchContent_MakeAvailable(catch2)

    enable_testing()
    add_subdirectory(tests)
endif()

if(VELOXLA_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.4
    )
    FetchContent_MakeAvailable(googlebenchmark)

    add_subdirectory(benchmarks)
endif()

if(VELOXLA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Install & export (supaya consumer bisa find_package(VeloxLA))
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS veloxla
    EXPORT VeloxLATargets
)

install(DIRECTORY include/veloxla
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT VeloxLATargets
    FILE VeloxLATargets.cmake
    NAMESPACE VeloxLA::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VeloxLA
)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/VeloxLAConfig.cmake.in
    ${CMAKE_BINARY_DIR}/VeloxLAConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VeloxLA
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/VeloxLAConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/VeloxLAConfig.cmake
    ${CMAKE_BINARY_DIR}/VeloxLAConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VeloxLA
)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "VeloxLA configuration summary")
message(STATUS "  Build type           : ${CMAKE_BUILD_TYPE}")
message(STATUS "  AVX-512 enabled       : ${VELOXLA_ENABLE_AVX512}")
message(STATUS "  OpenMP enabled        : ${VELOXLA_ENABLE_OPENMP}")
message(STATUS "  Backend OpenBLAS      : ${VELOXLA_USE_OPENBLAS}")
message(STATUS "  Backend MKL           : ${VELOXLA_USE_MKL}")
message(STATUS "  Backend BLIS          : ${VELOXLA_USE_BLIS}")
message(STATUS "  Build tests           : ${VELOXLA_BUILD_TESTS}")
message(STATUS "  Build benchmarks      : ${VELOXLA_BUILD_BENCHMARKS}")
message(STATUS "  Build examples        : ${VELOXLA_BUILD_EXAMPLES}")
message(STATUS "")
