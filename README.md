# OpenEAW

OpenEAW is an open-source re-implementation of Star Wars: Empire at War and its expansion pack "Forces of Corruption".

It aims to recreate, and where possible, improve the game.

To be able to run OpenEAW, you must own and have installed a copy of the original Star Wars: Empire at War and/or Forces of Corruption.

## Requirements

OpenEAW requires:

* a C++17-capable compiler
* [Conan](https://conan.io/) 2.5 or newer.
* [CMake](https://cmake.org/) 3.15 or newer.

## Getting Started

Make sure that the requirements mentioned above are installed.

## Building

Building uses Conan to automatically install all required dependencies and set up CMake presets:
```
conan install . -s build_type=Release -of build-release
cmake --preset conan-default
cmake --build --preset conan-release
```

After building, run the tests with CTest:
```
ctest --preset conan-release
```

To debug OpenEAW, run the following:
```
conan install . -s "&:build_type=Debug" -s build_type=Release -of build-debug
cmake --preset conan-default
cmake --build --preset conan-debug
ctest --preset conan-debug
```
This will build OpenEAW in Debug mode, but use Release builds for its dependencies.

## Contributing
Please refer to the [Code of Conduct](CODE_OF_CONDUCT.md) and the [Contributing guidelines](CONTRIBUTING.md).
