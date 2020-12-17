cpp_executable(simpleJitCompiler
    CPPFILES
        main.cpp

    LIBRARIES
        dl
        pthread
        tinfo

    INCLUDE_PATHS
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDES}
)

