add_executable(gravity main.cpp)
target_link_libraries(gravity Hd)
add_dependencies(gravity copy_resources)
