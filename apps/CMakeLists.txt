add_subdirectory(window_test)
add_subdirectory(mandelbrot)
add_subdirectory(texture_test)
add_subdirectory(balls)
add_subdirectory(orbit)

# TODO: -u option didnt work! copy only if files changed
add_custom_target(copy_resources ALL COMMAND rsync -u -r
    ${CMAKE_CURRENT_SOURCE_DIR}/res/ ${CMAKE_CURRENT_BINARY_DIR}/res/)
