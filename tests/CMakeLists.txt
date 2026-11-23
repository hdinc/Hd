# tests of the parts that need no window or gl context

if(MSVC)
    set(HD_TEST_WARNINGS /W4)
else()
    set(HD_TEST_WARNINGS -Wall -Wextra -Wpedantic -Wno-sign-compare)
endif()

# the targets and tests start with hd_, an app's own tests may have the same
# names
foreach(t search_test vt100_test zoom_test theme_test)
    add_executable(hd_${t} ${t}.cpp)
    target_link_libraries(hd_${t} Hd)
    target_compile_options(hd_${t} PRIVATE ${HD_TEST_WARNINGS})
endforeach()
add_test(NAME hd_search_test COMMAND hd_search_test)
add_test(NAME hd_vt100_test COMMAND hd_vt100_test)
add_test(NAME hd_zoom_test COMMAND hd_zoom_test)
add_test(NAME hd_theme_test COMMAND hd_theme_test ${CMAKE_CURRENT_BINARY_DIR})

# locks between processes (it runs copies of itself) and atomic replacement
add_executable(hd_files_test files_test.cpp)
target_link_libraries(hd_files_test Hd)
target_compile_options(hd_files_test PRIVATE ${HD_TEST_WARNINGS})
add_test(NAME hd_files_test COMMAND hd_files_test ${CMAKE_CURRENT_BINARY_DIR}/files_test)

# distance fields and curves of fonts: the library's and some of the system's
# when they're there
add_executable(hd_font_test font_test.cpp)
target_link_libraries(hd_font_test Hd)
target_compile_options(hd_font_test PRIVATE ${HD_TEST_WARNINGS})
add_test(NAME hd_font_test COMMAND hd_font_test ${PROJECT_SOURCE_DIR}/res/fonts/ElenaWebRegular.ttf
    /usr/share/fonts/noto/NotoSans-CondensedBoldItalic.ttf /usr/share/fonts/gsfonts/C059-BdIta.otf
    /usr/share/fonts/noto/NotoSerifDisplay-SemiCondensedBlackItalic.ttf)
set_tests_properties(hd_font_test PROPERTIES SKIP_RETURN_CODE 77)
