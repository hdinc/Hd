add_executable(orbit orbit.cpp circle.cpp)
target_link_libraries(orbit Hd)

include(../../cmake/resource.cmake)

add_resource_dir(orbit res)
