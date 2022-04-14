# function for adding resources to targets
function(add_resource_dir target folder)

    # get the files inside resource folder. when the files are changed res.myar
    # will be recreated. when new files are added we need to rerun cmake
    file(GLOB_RECURSE files ${folder}/*)

    # get the directory of myar binary
    get_target_property(MYAR_DIR myar-bin BINARY_DIR)

    # command for creating res.myar
    add_custom_command(
        OUTPUT res.myar
        COMMAND ${MYAR_DIR}/myar archive ${folder} -o ${CMAKE_CURRENT_BINARY_DIR}/res.myar -f
        DEPENDS myar-bin ${files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    target_sources(${target} PRIVATE res.myar)
    set_target_properties(${target} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/res.myar)

    # POST_BUILD command for patching target
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND objcopy --add-section res.myar=res.myar --set-section-flags
        res.myar=noload,readonly ${target} ${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

endfunction()
