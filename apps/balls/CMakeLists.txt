add_executable(balls main.cpp balls.cpp balls.h)
find_package(glm REQUIRED)
target_link_libraries(balls Hd glm imgui)
add_dependencies(balls copy_resources)
