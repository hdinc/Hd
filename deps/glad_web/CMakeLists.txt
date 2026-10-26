# glad's header for the web build: webgl 2 needs no loader, the functions
# come from the gles 3 headers
add_library(glad INTERFACE)
target_include_directories(glad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
