cmake_minimum_required(VERSION 3.29)

project(Hd VERSION 0.1.0)

if(NOT TARGET glm::glm)
    find_package(glm REQUIRED)
endif()
# theme files
if(NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json REQUIRED)
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
    src/Hd/Fonts.cpp
    src/Hd/Icons.cpp
    src/Hd/Theme.cpp
    src/Hd/Zoom.cpp
    src/Hd/FileList.cpp
    src/Hd/FileDialog.cpp
    src/Hd/Search.cpp
    src/Hd/Canvas2D.cpp
    src/Hd/Vt100.cpp
    src/Hd/TerminalView.cpp
    src/Hd/Platform.cpp
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

target_link_libraries(Hd PUBLIC glad imgui hdres stb glfw glm::glm PRIVATE msdfgen nlohmann_json::nlohmann_json)
if(WIN32)
    # file dialogs, opening URIs, the dark mode setting, dark title bars
    target_link_libraries(Hd PRIVATE comdlg32 shell32 advapi32 dwmapi)
endif()
if(FREETYPE_FOUND)
    target_link_libraries(Hd PRIVATE Freetype::Freetype)
    target_compile_definitions(Hd PRIVATE HD_HAS_FREETYPE)
endif()

# the installed fonts for the font picker (Fonts.h) from fontconfig, without
# it the usual font folders are scanned
option(HD_USE_FONTCONFIG "Find installed fonts with fontconfig" ON)
if(HD_USE_FONTCONFIG AND NOT EMSCRIPTEN)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FONTCONFIG QUIET IMPORTED_TARGET fontconfig)
    endif()
endif()
if(FONTCONFIG_FOUND)
    target_link_libraries(Hd PRIVATE PkgConfig::FONTCONFIG)
    target_compile_definitions(Hd PRIVATE HD_HAS_FONTCONFIG)
endif()

# the tests, run by ctest. an app including the library can set
# HD_BUILD_TESTS to run them with its own.
option(HD_BUILD_TESTS "Build the tests of the library" ${PROJECT_IS_TOP_LEVEL})
if(HD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set_target_properties(Hd PROPERTIES
    OUTPUT_NAME Hd-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)

# a web build is only linked into the program, with the page's side of
# Web.h
if(EMSCRIPTEN)
    target_sources(Hd PRIVATE src/Hd/Web.cpp)
    target_link_options(Hd INTERFACE "SHELL:--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/web/pre.js")
    set_property(TARGET Hd APPEND PROPERTY INTERFACE_LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/pre.js)
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