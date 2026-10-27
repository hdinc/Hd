# the folder of this file, for bin2c.cmake
set(_hd_resource_dir ${CMAKE_CURRENT_LIST_DIR})

# function for adding resources to targets
function(add_resource_dir target folder)

    # get the files inside resource folder. when the files are changed res.hdres
    # will be recreated. when new files are added we need to rerun cmake
    file(GLOB_RECURSE files ${folder}/*)

    get_target_property(type ${target} TYPE)
    if (type STREQUAL "EXECUTABLE")
        set(pfix exe)
    else()
        set(pfix lib)
    endif ()

    # command for creating res.hdres
    add_custom_command(
        OUTPUT ${pfix}res.hdres
        DEPENDS hdres-bin ${files}
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:hdres-bin> archive ${folder} -o ${CMAKE_CURRENT_BINARY_DIR}/${pfix}res.hdres -f
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    # wasm-ld can't link binary files, the archive becomes an array in c.
    # Resource.cpp reads <pfix>res_hdres and <pfix>res_hdres_size there.
    if(EMSCRIPTEN)
        add_custom_command(
            OUTPUT ${target}_res.c
            DEPENDS ${pfix}res.hdres ${_hd_resource_dir}/bin2c.cmake
            COMMAND ${CMAKE_COMMAND} -DIN=${pfix}res.hdres -DOUT=${target}_res.c -DNAME=${pfix}res_hdres
                -P ${_hd_resource_dir}/bin2c.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
        target_sources(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${target}_res.c)
        return()
    endif()

#     if(APPLE)
#         # gnu-ld does not support macos
#         # macos ld does not support -b binary option
#         # objcopy from brew generates debug symbols objcopy -I binary -O default file.bin file.o
#         # llvm-objcopy does not support MachO ouput target
#         add_custom_command(
#             OUTPUT ${target}_res.s
#             DEPENDS ${pfix}res.hdres
#             COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resource.sh ${pfix}res_hdres ${pfix}res.hdres ${target}_res.s
#             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#             )
#
#         add_custom_command(
#             OUTPUT ${target}_res.o
#             DEPENDS ${target}_res.s
#             COMMAND gcc -c ${target}_res.s -o ${target}_res.o
#             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#             )
#
#         target_sources(${target} PRIVATE ${target}_res.o)
#     elseif(UNIX)
        add_custom_command(
            OUTPUT ${target}_res.o
            DEPENDS ${pfix}res.hdres
            COMMAND ld -r -b binary -o ${target}_res.o ${pfix}res.hdres
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )

        target_sources(${target} PRIVATE ${target}_res.o)
#    endif()


endfunction()
