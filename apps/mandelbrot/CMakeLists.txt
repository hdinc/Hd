add_executable(mandelbrot main.cpp)
target_link_libraries(mandelbrot Hd imgui)

include(../../cmake/resource.cmake)

add_resource_dir(mandelbrot res)
