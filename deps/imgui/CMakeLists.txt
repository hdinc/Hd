set(SRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

add_library(imgui ${SRC})

target_compile_features(imgui PUBLIC cxx_std_14)

target_include_directories(imgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/imgui>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/imgui/backends>
)

target_link_libraries(imgui glfw)

install(DIRECTORY ./
    DESTINATION include/Hd-${PROJECT_VERSION}/deps/imgui
    FILES_MATCHING PATTERN "*.h"
)

set_target_properties(imgui PROPERTIES
    OUTPUT_NAME imgui-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)
