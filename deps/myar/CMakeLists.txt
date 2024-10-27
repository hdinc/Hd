cmake_minimum_required(VERSION 3.10)
project(myar)

add_library(myar myar.cpp)
target_include_directories(myar PUBLIC .)
target_compile_features(myar PUBLIC cxx_std_14)

add_executable(myar-bin main.cpp)
target_link_libraries(myar-bin myar)
target_compile_features(myar-bin PUBLIC cxx_std_14)

set_target_properties(myar-bin PROPERTIES
    OUTPUT_NAME myar
    )
