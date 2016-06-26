# osux
This is an alternative osu developement kit in C. The project is also including [Cqfuj](https://github.com/Cqfuj)'s taiko star rating system.

## Compilation
The project can be compiled using Autotools or CMake.

#### Cmake
``` bash
mkdir build
cd build
cmake ..
make
make install
```

#### Autotools
```
./configure
make
make install
```
If you want to install the project in another directory add  `--prefix=/absolute/path/to/install/dir`.

## Components

#### Developement kit
The project implements the following functionalities:
* osu beatmap parser
* osu replay parser
* database for finding a beatmap path with its hash

#### Taiko ranking
See [taiko ranking](https://github.com/tomtix/osux/tree/master/taikorank#taiko-ranking-project).

## Dependencies
* YAML
* SQLite

