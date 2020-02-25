function(jit_program PROGRAM_NAME)

    set(options
    )
    
    set(oneValueArgs
    )

    set(multiValueArgs
        CPPFILES
        LIBRARIES
    )

    cmake_parse_arguments(args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # Build executable
    add_executable(${PROGRAM_NAME} ${args_CPPFILES})

    target_include_directories(
        ${PROGRAM_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include
    )

    target_link_libraries(
        ${PROGRAM_NAME} PRIVATE ${args_LIBRARIES}
    )

    if (LINUX)
        set_target_properties(${PROGRAM_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN/../lib/")
    endif()

    if (APPLE)
        set_target_properties(${PROGRAM_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,@executable_path/../lib/")
    endif()

    # Install
    install(
        TARGETS ${PROGRAM_NAME}
        DESTINATION bin
    )

endfunction() # jit_program

