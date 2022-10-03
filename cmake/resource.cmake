# function for adding resources to targets
function(add_resource_dir target folder)

    # get the files inside resource folder. when the files are changed res.myar
    # will be recreated. when new files are added we need to rerun cmake
    file(GLOB_RECURSE files ${folder}/*)

    get_target_property(target_type ${target} TYPE)
    if (target_type STREQUAL "STATIC_LIBRARY")
        set(prefix "lib")
    elseif(target_type STREQUAL "EXECUTABLE")
        set(prefix "exe")
    endif ()


    # get the directory of myar binary
    get_target_property(MYAR_DIR myar-bin BINARY_DIR)

    # command for creating res.myar
    add_custom_command(
        OUTPUT ${prefix}_res.myar
        COMMAND ${MYAR_DIR}/myar archive ${folder} -o ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_res.myar -f
        DEPENDS myar-bin ${files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    add_custom_command(
        OUTPUT ${target}_res.o
        COMMAND objcopy -I binary -O elf64-x86-64 ${prefix}_res.myar ${target}_res.o
        DEPENDS ${prefix}_res.myar
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    target_sources(${target} PRIVATE ${target}_res.o)

endfunction()
