# multi-channel signed distance fields for the canvas font. only the core,
# it needs nothing else.
file(GLOB MSDFGEN_SRC core/*.cpp)
add_library(msdfgen STATIC ${MSDFGEN_SRC})

target_compile_features(msdfgen PUBLIC cxx_std_11)
target_compile_definitions(msdfgen PUBLIC MSDFGEN_USE_CPP11 MSDFGEN_PUBLIC=)
target_include_directories(msdfgen
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
# fonts are converted at startup and when the font changes, slow unoptimized
if(NOT MSVC)
    target_compile_options(msdfgen PRIVATE -O2 -w)
endif()

set_target_properties(msdfgen PROPERTIES
    OUTPUT_NAME msdfgen-${PROJECT_VERSION}
    DEBUG_POSTFIX "_debug"
)
