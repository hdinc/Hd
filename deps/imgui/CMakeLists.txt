set(SRC
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui.cpp
    examples/imgui_impl_glfw.cpp
    examples/imgui_impl_opengl3.cpp
)

add_library(imgui ${SRC})
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui glad glfw)

target_include_directories(imgui PUBLIC ./)
target_include_directories(imgui PUBLIC examples/ )
