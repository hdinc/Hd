add_executable(resource_test main.cpp)
target_link_libraries(resource_test Hd)

include(../../cmake/resource.cmake)
add_resource_dir(resource_test res)
