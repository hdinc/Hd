cmake_minimum_required(VERSION 3.10)

project(Hd VERSION 0.0.0)

find_package(glfw3 CONFIG REQUIRED)

set(SRC
    Hd/glad/glad.c
    Hd/Hd/Window.cpp
)

add_library(Hd ${SRC})

target_include_directories(Hd PUBLIC Hd)
target_link_libraries(Hd glfw dl)

add_subdirectory(apps)
