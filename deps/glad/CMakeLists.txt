add_library(glad src/glad.c)

target_include_directories(glad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/glad/include>
)

install(DIRECTORY include
    DESTINATION include/Hd-${PROJECT_VERSION}/deps/glad
)

if(UNIX)
    target_link_libraries(glad dl)
endif()

set_target_properties(glad PROPERTIES
    OUTPUT_NAME glad-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)