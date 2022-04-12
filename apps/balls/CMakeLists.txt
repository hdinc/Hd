add_executable(balls main.cpp balls.cpp balls.h)
target_link_libraries(balls Hd imgui)

include(../../cmake/resource.cmake)

add_resource_dir(balls res)
