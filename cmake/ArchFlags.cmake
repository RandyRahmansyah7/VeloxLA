# ArchFlags.cmake
#
# Setup flag arsitektur buat SIMD kernel manual (immintrin.h).
# AVX2 jadi baseline minimum karena hampir semua CPU x86_64 modern (post-2013
# Intel Haswell, post-2015 AMD Excavator/Zen) support ini. AVX-512 opsional
# karena coverage CPU masih terbatas (server-grade Intel, Zen4+ AMD).
#
# Native arch detection (-march=native) SENGAJA TIDAK dipakai sebagai default
# karena binary hasil build jadi tidak portable ke CPU lain — buruk untuk
# distribusi library. User yang build untuk mesin sendiri bisa override lewat
# -DVELOXLA_NATIVE_ARCH=ON.

option(VELOXLA_NATIVE_ARCH "Build dengan -march=native (tidak portable, hanya untuk benchmark lokal)" OFF)

function(veloxla_set_arch_flags target_name)
    include(CheckCXXCompilerFlag)

    if(VELOXLA_NATIVE_ARCH)
        if(NOT MSVC)
            check_cxx_compiler_flag("-march=native" VELOXLA_HAS_MARCH_NATIVE)
            if(VELOXLA_HAS_MARCH_NATIVE)
                target_compile_options(${target_name} INTERFACE -march=native)
                message(STATUS "  Arch flags          : -march=native (non-portable build)")
                return()
            endif()
        endif()
    endif()

    if(MSVC)
        # MSVC tidak punya flag granular sejenis -mavx2; /arch:AVX2 paling dekat.
        target_compile_options(${target_name} INTERFACE /arch:AVX2)
        if(VELOXLA_ENABLE_AVX512)
            target_compile_options(${target_name} INTERFACE /arch:AVX512)
        endif()
    else()
        check_cxx_compiler_flag("-mavx2" VELOXLA_HAS_AVX2)
        check_cxx_compiler_flag("-mfma" VELOXLA_HAS_FMA)

        if(VELOXLA_HAS_AVX2)
            target_compile_options(${target_name} INTERFACE -mavx2)
        else()
            message(WARNING "Compiler tidak support -mavx2 — kernel akan fallback ke scalar path")
        endif()

        if(VELOXLA_HAS_FMA)
            target_compile_options(${target_name} INTERFACE -mfma)
        endif()

        if(VELOXLA_ENABLE_AVX512)
            check_cxx_compiler_flag("-mavx512f" VELOXLA_HAS_AVX512F)
            if(VELOXLA_HAS_AVX512F)
                target_compile_options(${target_name} INTERFACE -mavx512f -mavx512vl -mavx512bw)
                target_compile_definitions(${target_name} INTERFACE VELOXLA_HAS_AVX512)
            else()
                message(WARNING "VELOXLA_ENABLE_AVX512=ON tapi compiler tidak support -mavx512f")
            endif()
        endif()
    endif()

    if(VELOXLA_HAS_AVX2 OR MSVC)
        target_compile_definitions(${target_name} INTERFACE VELOXLA_HAS_AVX2)
    endif()
endfunction()
