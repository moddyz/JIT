cpp_program(simpleJitCompiler
    CPPFILES
        main.cpp

    LIBRARIES
        ${CLANG_LIBRARIES}
        ${LLVM_LIBRARIES}
        dl
        pthread
        tinfo

    INCLUDE_PATHS
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDES}
)

