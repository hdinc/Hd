add_library(glad src/glad.c)

target_include_directories(glad PUBLIC include)

if(UNIX)
    target_link_libraries(glad dl)
endif()
