# examples/CMakeLists.txt

function(veloxla_add_example name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE VeloxLA::veloxla)
    target_compile_features(${name} PRIVATE cxx_std_17)
endfunction()

veloxla_add_example(basic_usage          basic_usage.cpp)
veloxla_add_example(expression_chaining  expression_chaining.cpp)
veloxla_add_example(custom_contraction   custom_contraction.cpp)
