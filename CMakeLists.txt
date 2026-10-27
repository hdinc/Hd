cmake_minimum_required(VERSION 3.29)

project(Hd VERSION 0.1.0)

if(NOT TARGET glm::glm)
    find_package(glm REQUIRED)
endif()

# hinted glyphs for imgui and small canvas text, stb_truetype and distance
# fields only without it
option(HD_USE_FREETYPE "Hinted text through FreeType" ON)

if(EMSCRIPTEN)
    # glfw and freetype are emscripten ports, gl is webgl 2 through the
    # gles 3 headers
    add_library(glfw INTERFACE)
    target_compile_options(glfw INTERFACE --use-port=contrib.glfw3)
    target_link_options(glfw INTERFACE --use-port=contrib.glfw3)
    if(HD_USE_FREETYPE)
        add_library(Freetype::Freetype INTERFACE IMPORTED)
        target_compile_options(Freetype::Freetype INTERFACE -sUSE_FREETYPE=1)
        target_link_options(Freetype::Freetype INTERFACE -sUSE_FREETYPE=1)
        set(FREETYPE_FOUND TRUE)
    endif()
    add_subdirectory(deps/glad_web)
else()
    find_package(glfw3 REQUIRED)
    if(HD_USE_FREETYPE)
        find_package(Freetype QUIET)
    endif()
    add_subdirectory(deps/glad)
endif()

add_subdirectory(deps/imgui)
add_subdirectory(deps/hdres)
add_subdirectory(deps/stb)
add_subdirectory(deps/msdfgen)

set(SRC
    src/Hd/Window.cpp
    src/Hd/Shader.cpp
    src/Hd/Texture.cpp
    src/Hd/Gui.cpp
    src/Hd/FrameBuffer.cpp
    src/Hd/Camera.cpp
    src/Hd/CamCtrl.cpp
    src/Hd/CamCtrl2D.cpp
    src/Hd/CamCtrl3D.cpp
    src/Hd/Resource.cpp
    src/Hd/ResourceDefault.cpp
    src/Hd/Graphics2D.cpp
    src/Hd/Text.cpp
    src/Hd/Font.cpp
    src/Hd/Msdf.cpp
    src/Hd/Outline.cpp
    src/Hd/Renderer2D.cpp
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
)

target_link_libraries(Hd PUBLIC glad imgui hdres stb glfw glm::glm PRIVATE msdfgen)
if(FREETYPE_FOUND)
    target_link_libraries(Hd PRIVATE Freetype::Freetype)
    target_compile_definitions(Hd PRIVATE HD_HAS_FREETYPE)
endif()

set_target_properties(Hd PROPERTIES
    OUTPUT_NAME Hd-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)

# a web build is only linked into the program
if(EMSCRIPTEN)
    return()
endif()

install(TARGETS Hd glad imgui hdres hdres-bin stb msdfgen
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