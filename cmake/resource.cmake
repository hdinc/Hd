# function for adding resources to targets
function(add_resource_dir target folder)

    # get the files inside resource folder. when the files are changed res.myar
    # will be recreated. when new files are added we need to rerun cmake
    file(GLOB_RECURSE files ${folder}/*)

    get_target_property(type ${target} TYPE)
    if (type STREQUAL "EXECUTABLE")
        set(pfix exe)
    else()
        set(pfix lib)
    endif ()

    # get the directory of myar binary
    get_target_property(MYAR_DIR myar-bin BINARY_DIR)

    # command for creating res.myar
    add_custom_command(
        OUTPUT ${pfix}res.myar
        DEPENDS myar-bin ${files}
        COMMAND ${MYAR_DIR}/myar archive ${folder} -o ${CMAKE_CURRENT_BINARY_DIR}/${pfix}res.myar -f
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

#     if(APPLE)
#         # gnu-ld does not support macos
#         # macos ld does not support -b binary option
#         # objcopy from brew generates debug symbols objcopy -I binary -O default file.bin file.o
#         # llvm-objcopy does not support MachO ouput target
#         add_custom_command(
#             OUTPUT ${target}_res.s
#             DEPENDS ${pfix}res.myar
#             COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/resource.sh ${pfix}res_myar ${pfix}res.myar ${target}_res.s 
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
            DEPENDS ${pfix}res.myar
            COMMAND ld -r -b binary -o ${target}_res.o ${pfix}res.myar
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )

        target_sources(${target} PRIVATE ${target}_res.o)
#    endif()


endfunction()
