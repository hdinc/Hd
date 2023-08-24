cmake_minimum_required(VERSION 3.10)

project(Hd VERSION 0.0.0)

set(GLFW_BUILD_DOCS off CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS off CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES off CACHE BOOL "" FORCE)
set(GLFW_INSTALL off CACHE BOOL "" FORCE)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)
add_subdirectory(deps/imgui)
add_subdirectory(deps/stb)
add_subdirectory(deps/glm)
add_subdirectory(deps/myar)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(SRC
    src/Hd/Window.cpp
    src/Hd/Shader.cpp
    src/Hd/Texture.cpp
    src/Hd/Gui.cpp
    src/Hd/FrameBuffer.cpp
    src/Hd/Camera.cpp
    src/Hd/Resource.cpp
)

add_library(Hd ${SRC})
target_link_libraries(Hd glfw glad imgui stb glm myar)

target_include_directories(Hd PUBLIC include)

include(cmake/resource.cmake)
add_resource_dir(Hd res/)
