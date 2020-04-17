cmake_minimum_required(VERSION 3.10)

project(Hd VERSION 0.0.0)

find_package(glfw3 CONFIG REQUIRED)
add_subdirectory(deps/glad)
add_subdirectory(deps/imgui)

set(SRC
    src/Hd/Window.cpp
    src/Hd/Shader.cpp
)

add_library(Hd ${SRC})
target_link_libraries(Hd glfw glad imgui)

target_include_directories(Hd PUBLIC include)

add_subdirectory(apps)
