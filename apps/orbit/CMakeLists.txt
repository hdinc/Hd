add_executable(orbit orbit.cpp circle.cpp)
find_package(glm REQUIRED)
target_link_libraries(orbit glm Hd)
