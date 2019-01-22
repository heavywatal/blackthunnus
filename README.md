# blackthunnus

[![Build Status](https://travis-ci.org/heavywatal/blackthunnus.svg?branch=master)](https://travis-ci.org/heavywatal/blackthunnus)

Individual-based simulator of pacific bluefin tuna.

[Project page on GitHub](https://github.com/heavywatal/blackthunnus)


## Requirements

- Unix-like environment (macOS, Linux, WSL, MinGW on MSYS2, etc.)
- C++14 compiler (clang++ >= Apple LLVM 8.1, g++ >= 5.3)
- [CMake](https://cmake.org/) (>= 3.4.0)

The following libraries are optional or automatically installed:

- [clippson](https://github.com/heavywatal/clippson)
- [cxxwtl](https://github.com/heavywatal/cxxwtl)
- [sfmt-class](https://github.com/heavywatal/sfmt-class)
- [zlib](https://zlib.net)


## R interface

You can install and use this program via [R package "tekkamaki"](https://heavywatal.github.io/tekkamaki/).


## Installation of command-line version

The easiest way is to use [Homebrew](https://brew.sh/)/[Linuxbrew](http://linuxbrew.sh/).
The following command installs blackthunnus and all the dependencies:
```sh
brew install heavywatal/tap/blackthunnus
```

Alternatively, you can get the source code from GitHub manually:
```sh
git clone https://github.com/heavywatal/blackthunnus.git
cd blackthunnus/
mkdir build
cd build/
YOUR_PREFIX=${HOME}/local  # or /usr/local
cmake -DCMAKE_INSTALL_PREFIX=$YOUR_PREFIX ..
make -j2
make install
```
