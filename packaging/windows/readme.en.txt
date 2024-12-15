This document describes building the Windows version of Scan Tailor.

Earlier versions of Scan Tailor supported both Visual Studio and MinGW
compilers. MinGW support was dropped at some point, in order to reduce
maintenance effort. Furthermore, Scan Tailor started to use some C++17
features, making Visual Studio versions below 2017 not oficially supported.


                                Downloading Prerequisites

First, download the following software.  Unless stated otherwise, take the
latest stable version.

1. Visual Studio 2017 Community for Windows Desktop or higher.
   Homepage: https://visualstudio.microsoft.com/
2. CMake >= 2.8.9
   Homepage: http://www.cmake.org
3. jpeg library
   Homepage: http://www.ijg.org/
   The file we need will be named jpegsrc.v9.tar.gz or similarly.
4. zlib
   Homepage: http://www.zlib.net/
   We need a file named like zlib-x.x.x.tar.gz, where x.x.x represents
   the version number.
5. libpng
   Homepage: http://www.libpng.org/pub/png/libpng.html
   We need a file named like libpng-x.x.x.tar.gz, where x.x.x represents
   the version number.
6. libtiff
   Homepage: http://www.remotesensing.org/libtiff/
   Because libtiff is updated rarely, but vulnerabilities in it are found often,
   it's better to patch it right away.  In that case, take it from here:
   http://packages.debian.org/source/sid/tiff
   There you will find both the original libtiff and a set of patches for it.
   The process of patching libtiff is described later in this document.
   If you aren't going to distribute your Scan Tailor build and aren't going
   to open files from untrusted sources, then you don't really need patching it.
7. Qt5 >= 5.3 (tested with 5.12.12, 6.8.1)
   Homepage: http://qt-project.org/
   Either a source-only or any of the binary versions will do. In either case
   a custom build of Qt will be made, though a binary version will result in
   less things to build.
8. Strawberry Perl (necessary to build Qt)
   Homepage: https://strawberryperl.com/
   When installing make sure that "Add Perl to the PATH environment
   variable" option is set.
9. Boost (tested with 1.86.0)
   Homepage: http://boost.org/
   You can download it in any file format, provided you know how to unpack it.
10. Eigen3 (tested with 3.4.0)
   Homepage: http://eigen.tuxfamily.org/
11. NSIS 3.x (tested with 3.09)
   Homepage: http://nsis.sourceforge.net/
   You should also download plugin https://nsis.sourceforge.io/NsProcess_plugin and install into the same folder with nsis:
   * Folder `NsProcess.zip\Include` should be copied into `c:\Program Files (x86)\NSIS\`.
   * File `NsProcess.zip\Plugin\nsProcess.dll` should be copied into folder `c:\Program Files (x86)\NSIS\Plugins\x86-ansi\`.
   * File `NsProcess.zip\Plugin\nsProcessW.dll` should be copied into folder `c:\Program Files (x86)\NSIS\Plugins\x86-unicode` and renamed into `nsProcess.dll`.


                                    Instructions

1. Create a build directory. Its full path should have no spaces. From now on
   this document will be assuming the build directory is C:\build

2. Unpack jpeg, libpng, libtiff, zlib, boost, Eigen, and scantailor
   itself to the build directory.  You should get a directory structure like
   this:
   C:\build
     | boost_1_86_0
     | eigen-3.4.0
     | jpeg-9e
     | libpng-1.6.44
     | scantailor
     | tiff-4.7.0
     | zlib-1.3.1
   
   If you went for a source-only version of Qt, unpack it there as well.
   Otherwise, install Qt into whatever directory its installer suggests.
   IMPORTANT: Tell the installer to install Source Components as well.
   
   If you don't know how to unpack .tar.gz files, try this tool:
   http://www.7-zip.org/

3. Install Visual Studio, Strawberry Perl, NSIS and CMake.

4. Compile dependancies (boost, jpeg, libpng, tiff, zlib, Qt).
   You can also use vcpkg with CMake to retrieve and build dependancies so you don't need to download and build them by hands.

5. Compile scantailor (include path to previously compiled boost, eigen, jpeg, libpng, scantailor, tiff, zlib, Qt)
