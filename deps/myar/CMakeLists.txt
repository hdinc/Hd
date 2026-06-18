add_library(myar myar.cpp)

target_compile_features(myar PUBLIC cxx_std_17)

target_include_directories(myar
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Hd-${PROJECT_VERSION}/deps/myar>
)

install(DIRECTORY ./
    DESTINATION include/Hd-${PROJECT_VERSION}/deps/myar
    FILES_MATCHING PATTERN "*.h"
)

set_target_properties(myar PROPERTIES
    OUTPUT_NAME myar-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)

add_executable(myar-bin main.cpp)

target_link_libraries(myar-bin myar)

set_target_properties(myar-bin PROPERTIES
    OUTPUT_NAME myar-${PROJECT_VERSION}
)