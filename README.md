# osux
This is an experimental osu developement kit in C. The project is also including [Cqfuj](https://github.com/Cqfuj)'s taiko star rating system.

## Compilation
The project can be compiled using Autotools or CMake (Both are currently limited to Unix environment though).

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
# (re-)generate the build system: (you must probably do this if you pulled the project from a git repo)
./autogen.sh

# build:
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
See [taiko ranking](https://github.com/tomtix/osux/tree/master/src/taikorank#taiko-ranking-project).

## (Bunch of) Dependencies
* libyaml
* libsqlite3
* libmysqlclient (mysql C-connector/ or mariadb fork, doesn't matter)
* libgts (GNU GTS)
* glib2.0 (The Gnome Portable Library) :3
* libcrypto (OpenSSL)
* liblzma (aka xz, xz utils)

Some of this dependencies have only the GNU buildsystem,
others only CMake, so it's pretty hard to get the project to
compile on Windows. We 'll try to make something up
eventually with MinGW

