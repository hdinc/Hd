add_executable(texture_test main.cpp)
target_link_libraries(texture_test Hd)
add_dependencies(texture_test copy_resources)
