cmake_minimum_required(VERSION 3.10)

project(Hd VERSION 0.0.0)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
add_subdirectory(deps/glad)
add_subdirectory(deps/imgui)
add_subdirectory(deps/stb)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SRC
    src/Hd/Window.cpp
    src/Hd/Shader.cpp
    src/Hd/Texture.cpp
    src/Hd/Gui.cpp
    src/Hd/FrameBuffer.cpp
    src/Hd/Camera2D.cpp
    src/Hd/WindowCallbacks.cpp
)

add_library(Hd ${SRC})
target_link_libraries(Hd glfw glad imgui stb glm)

target_include_directories(Hd PUBLIC include)

option(build_apps "build_apps" OFF)
if(build_apps)
    add_subdirectory(apps)
endif(build_apps)
