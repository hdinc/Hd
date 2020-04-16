cmake_minimum_required(VERSION 3.10)

project(Hd VERSION 0.0.0)

find_package(glfw3 CONFIG REQUIRED)

set(SRC
    src/glad/glad.c
    src/Hd/Window.cpp
)

add_library(Hd ${SRC})

target_include_directories(Hd PUBLIC include)

target_link_libraries(Hd glfw)
if(UNIX)
    target_link_libraries(Hd dl)
endif(UNIX)

add_subdirectory(apps)
