add_executable(mandelbrot main.cpp)

target_link_libraries(mandelbrot Hd imgui)

add_custom_target(copy_shaders ALL COMMAND cp -u ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl ${CMAKE_CURRENT_BINARY_DIR}/)
