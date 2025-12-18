# Build instructions for Unices

## Preparations

Install following packages:

### Debian-based distros

* libboost-all-dev
* libtiff-dev
* libjpeg-dev
* libeigen3-dev
* qtbase5-dev
* qttools5-dev
* gcc
* g++
* cmake

For build with Qt6 install following packages:

### Debian-based distros

* qt6-base-dev
* qt6-tools-dev

## Compilation

Use following command:

``sh
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
cmake --build .
``

For Qt6 build use following command:

``sh
mkdir build-qt6
cd build-qt6
cmake -D ST_USE_QT6=ON -D CMAKE_BUILD_TYPE=Release ..
cmake --build .
``
