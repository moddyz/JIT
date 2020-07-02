cmake_minimum_required(VERSION 3.17)

# Project Name
project(
    LLVMExperiments
    VERSION 0.0
    DESCRIPTION "Experiments with LLVM and Clang."
    LANGUAGES CXX
)

# Cross-platform compatibility.
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Discover modules & macros.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/macros)
include(Public)
set(LLVM_STATIC TRUE)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

# Recurse into src/.
add_subdirectory(src)
