add_subdirectory(window_test)
add_subdirectory(mandelbrot)
add_subdirectory(texture_test)
add_subdirectory(balls)
add_subdirectory(orbit)
add_subdirectory(3dtest)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    ${CMAKE_CURRENT_BINARY_DIR}/res)
