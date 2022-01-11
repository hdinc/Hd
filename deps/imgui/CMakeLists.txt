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
target_link_libraries(imgui glfw)

target_include_directories(imgui PUBLIC ./)
target_include_directories(imgui PUBLIC backends/ )
