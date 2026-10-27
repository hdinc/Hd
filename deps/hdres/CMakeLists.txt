add_library(hdres hdres.cpp)

target_compile_features(hdres PUBLIC cxx_std_17)

target_include_directories(hdres
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/hdres>
)

install(DIRECTORY ./
    DESTINATION include/Hd-${PROJECT_VERSION}/deps/hdres
    FILES_MATCHING PATTERN "*.h"
)

set_target_properties(hdres PROPERTIES
    OUTPUT_NAME hdres-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)

add_executable(hdres-bin main.cpp)

target_link_libraries(hdres-bin hdres)

set_target_properties(hdres-bin PROPERTIES
    OUTPUT_NAME hdres-${PROJECT_VERSION}
)
# run by node in a web build, on the real file system
if(EMSCRIPTEN)
    target_link_options(hdres-bin PRIVATE -sNODERAWFS=1)
endif()
