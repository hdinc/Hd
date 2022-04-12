add_executable(texture_test main.cpp)
target_link_libraries(texture_test Hd)

include(../../cmake/resource.cmake)

add_resource_dir(texture_test res)
