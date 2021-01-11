add_executable(mandelbrot main.cpp)
find_package(glm REQUIRED)
target_link_libraries(mandelbrot Hd imgui glm)
add_dependencies(mandelbrot copy_resources)
