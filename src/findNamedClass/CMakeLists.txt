cpp_executable(findNamedClass
    CPPFILES
        main.cpp

    LIBRARIES
        clangFrontend
        clangAST
        clangBasic
        clangTooling

    INCLUDE_PATHS
        ${CLANG_INCLUDE_DIRS}
)
