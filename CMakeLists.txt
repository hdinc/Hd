cmake_minimum_required(VERSION 3.29)

project(Hd VERSION 0.1.0)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory(deps/glad)
add_subdirectory(deps/imgui)
add_subdirectory(deps/myar)
add_subdirectory(deps/stb)

set(SRC
    src/Hd/Window.cpp
    src/Hd/Shader.cpp
    src/Hd/Texture.cpp
    src/Hd/Gui.cpp
    src/Hd/FrameBuffer.cpp
    src/Hd/Camera.cpp
    src/Hd/Resource.cpp
    src/Hd/Graphics2D.cpp
    src/Hd/Text.cpp
)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_library(Hd ${SRC})

if(MSVC)
    target_compile_options(Hd PRIVATE /W4)
else()
    target_compile_options(Hd PRIVATE -Wall -Wextra -pedantic)
endif()

include(cmake/resource.cmake)

add_resource_dir(Hd res)

target_include_directories(Hd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/include>
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(Hd PUBLIC glad imgui myar stb glfw glm::glm ${OpenCV_LIBS})

set_target_properties(Hd PROPERTIES
    OUTPUT_NAME Hd-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)

install(TARGETS Hd glad imgui myar myar-bin stb
    EXPORT Hd_export
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES "1942b4fa-b2c5-4546-9385-83f254070067")

install(EXPORT Hd_export
    # NAMESPACE Hd::
    DESTINATION lib/cmake/Hd-${PROJECT_VERSION}
    FILE HdConfig.cmake
    EXPORT_PACKAGE_DEPENDENCIES
)

install(DIRECTORY include
    DESTINATION include/Hd-${PROJECT_VERSION}
)

install(FILES cmake/resource.cmake
    DESTINATION lib/cmake/Hd-${PROJECT_VERSION}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HdConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HdConfigVersion.cmake
    DESTINATION lib/cmake/Hd-${PROJECT_VERSION}
)