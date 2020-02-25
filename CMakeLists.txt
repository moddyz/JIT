cmake_minimum_required(VERSION 3.10)

# Project Name
project(
    JIT VERSION 0.0
    DESCRIPTION "Just In Time compiler."
    LANGUAGES CXX
)

# Cross-platform compatibility.
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Load CMake modules
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/modules")
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/macros")
set(LLVM_STATIC TRUE)
find_package(LLVM REQUIRED)
include(Public)

# Build with C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler options
add_compile_options(-g -O3 -Werror)

# Build lib and program targets.
add_subdirectory(lib)
add_subdirectory(program)
