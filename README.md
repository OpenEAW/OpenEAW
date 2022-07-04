# OpenEAW

OpenEAW is an open-source re-implementation of Star Wars: Empire at War and its expansion pack "Forces of Corruption".

It aims to recreate, and where possible, improve the game.

To be able to run OpenEAW, you must own and have installed a copy of the original Star Wars: Empire at War and/or Forces of Corruption.

## Requirements

OpenEAW requires:

* a C++17-capable compiler
* [Conan](https://conan.io/) 1.46 or newer.
* [CMake](https://cmake.org/) 3.15 or newer.
* [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) 12.
* [ClangTidy](https://clang.llvm.org/extra/clang-tidy/) 10.

## Getting Started

Make sure that the requirements mentioned above are installed.

## Building

Building uses Conan to automatically install all required dependencies.
CMake uses a _multi-configuration generator_ for Visual Studio which ignores `CMAKE_BUILD_TYPE` and allows specifying the build type at build time, rather than configuration time:
```
mkdir build && cd build
conan install .. -s build_type=Release
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan/conan_toolchain.cmake
cmake --build . --config Release
```

## Contributing
Please refer to the [Code of Conduct](CODE_OF_CONDUCT.md) and the [Contributing guidelines](CONTRIBUTING.md).
