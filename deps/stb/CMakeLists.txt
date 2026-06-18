add_library(stb src/imp.c)

target_include_directories(stb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/stb/include>
)

install(DIRECTORY include
    DESTINATION include/Hd-${PROJECT_VERSION}/deps/stb
    FILES_MATCHING PATTERN "*.h"
)

set_target_properties(stb PROPERTIES
    OUTPUT_NAME stb-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)