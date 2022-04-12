add_executable(gravity main.cpp)
target_link_libraries(gravity Hd)

include (../../cmake/resource.cmake)

add_resource_dir(gravity res)
