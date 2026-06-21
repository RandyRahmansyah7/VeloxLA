# CompilerWarnings.cmake
#
# Centralized warning flags supaya konsisten di semua target (core lib, tests,
# benchmarks, examples). Strict by default — tensor/SIMD code rawan bug
# off-by-one dan implicit conversion yang harus ketahuan sejak compile time.

function(veloxla_set_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} INTERFACE
            /W4
            /permissive-
        )
    else()
        target_compile_options(${target_name} INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wdouble-promotion
        )
    endif()
endfunction()
