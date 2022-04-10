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
        COMMAND ${MYAR_DIR}/myar archive ${folder} -o ${CMAKE_CURRENT_BINARY_DIR}/res.myar
        DEPENDS myar-bin ${files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    # add dependecy cant add files as dependency so wee need to create new
    #target
    add_custom_target(${target}_resources DEPENDS res.myar)
    add_dependencies(${target} ${target}_resources)

    # POST_BUILD command for patching target
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND objcopy --add-section res.myar=res.myar --set-section-flags
        res.myar=noload,readonly ${target} ${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

endfunction()
