cmake_minimum_required(VERSION 3.10)
project(myar)

add_library(myar myar.cpp)
target_include_directories(myar PUBLIC .)

add_executable(myar-bin main.cpp)
target_link_libraries(myar-bin myar)

set_target_properties(myar-bin PROPERTIES
    OUTPUT_NAME myar
    )
