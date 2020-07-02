# LLVMExperiments

A collection of experiments with LLVM and Clang libraries.

## Table of Contents

- [Building](#building)
  - [Requirements](#requirements)
- [Build Status](#build-status)

## Documentation

Documentation based on the latest state of master, [hosted by GitHub Pages](https://moddyz.github.io/LLVMExperiments/).

## Building

A convenience build script is also provided, for building all targets, and optionally installing to a location:
```
./build.sh <OPTIONAL_INSTALL_LOCATION>
```

### Requirements

- `>= CMake-3.17`
- `>= C++17`
- `doxygen` and `graphviz` (optional for documentation)
- \>= `llvm-9.0.1`
- \>= `clang-9.0.1`

## Build Status

|       | master | 
| ----- | ------ | 
| macOS-10.14 | [![Build Status](https://travis-ci.com/moddyz/LLVMExperiments.svg?branch=master)](https://travis-ci.com/moddyz/LLVMExperiments) |

