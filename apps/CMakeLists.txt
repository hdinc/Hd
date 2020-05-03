add_subdirectory(window_test)
add_subdirectory(mandelbrot)

add_custom_target(copy_resources ALL COMMAND cp -ru ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR})
