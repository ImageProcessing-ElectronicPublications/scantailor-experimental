CMAKE_MINIMUM_REQUIRED(VERSION 3.21.0)

IF(NOT DEFINED ST_ARCH)
    SET(ST_ARCH "X86")
    SET(ST_SUPPRESS_OPENGL FALSE)
ENDIF()
IF(ST_ARCH STREQUAL "ARM")
    # Because OpenGL is always software here
    SET(ST_SUPPRESS_OPENGL TRUE)
    SET(ST_USE_QT6 ON)
ENDIF()

IF(POLICY CMP0003)
    # Allow linking with full library path instead of -L/path -llibrary
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF()

IF(POLICY CMP0048)
    # When PROJECT() is missing the VERSION argument, set the corresponding
    # CMake variables to zero.
    CMAKE_POLICY(SET CMP0048 NEW)
ENDIF()

SET(
    CMAKE_USER_MAKE_RULES_OVERRIDE
    "${CMAKE_SOURCE_DIR}/cmake/default_cflags.cmake"
)
SET(
    CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
    "${CMAKE_SOURCE_DIR}/cmake/default_cxxflags.cmake"
)

PROJECT("Scan Tailor Experimental")

# This should be defined after project because of CMAKE_SIZEOF_VOID_P
MATH(EXPR ST_ARCH_BITS "${CMAKE_SIZEOF_VOID_P} * 8")
IF(ST_ARCH STREQUAL "X86")
    SET(ST_FULLARCH "${ST_ARCH}-${ST_ARCH_BITS}") 
ELSE()
    SET(ST_FULLARCH "${ST_ARCH}${ST_ARCH_BITS}") 
ENDIF()

include_directories(src)

# Keep the paths outside of the build directory in RPATH
# when installing. That's useful for building with say
# a custom build of Qt.
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Scan Tailor needs to find its shared libraries.
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/scantailor-experimental")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

IF(MSVC)
    # Disable checked iterators for extra performance.
    ADD_DEFINITIONS(-D_SECURE_SCL=0)
ENDIF()

IF(DEBUG_CLI)
    ADD_DEFINITIONS(-DDEBUG_CLI)
ENDIF(DEBUG_CLI)

ENABLE_TESTING()

# An undocumented side-effect of CONFIGURE_FILE() is that it makes
# the whole project depend on the file we are parsing / copying.
CONFIGURE_FILE(
    "${PROJECT_SOURCE_DIR}/src/version.h"
    "${PROJECT_BINARY_DIR}/src/.version.h" COPYONLY
)

# Prevent this leftover from old builds to be used in favour
# of the one in ${PROJECT_SOURCE_DIR}
IF(NOT "${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
    FILE(REMOVE "${PROJECT_BINARY_DIR}/src/version.h")
ENDIF()

# Extract STFAMILY and VERSION from version.h
FILE(READ "${PROJECT_SOURCE_DIR}/src/version.h" version_h_contents)
STRING(
    REGEX REPLACE
    ".*#define[ \\t]+STFAMILY[ \\t]+\"([^\"]*)\".*"
    "\\1" ST_FAMILY "${version_h_contents}"
)
IF("${ST_FAMILY}" STREQUAL "${version_h_contents}")
    MESSAGE(FATAL_ERROR "Failed to extract STFAMILY from version.h")
ENDIF()
STRING(TOLOWER "${ST_FAMILY}" ST_FAMILY_LOWER)
# Make ST_FAMILY with capital letter
STRING(TOUPPER "${ST_FAMILY}" ST_FAMILY)
STRING(SUBSTRING "${ST_FAMILY}" 0 1 ST_FAMILY)
STRING(SUBSTRING "${ST_FAMILY_LOWER}" 1 -1 ST_FAMILY_TEMP)
STRING(CONCAT ST_FAMILY "${ST_FAMILY}" "${ST_FAMILY_TEMP}")

# VERSION must be either empty or be in the form of X.Y.Z.Y
STRING(
    REGEX REPLACE
    ".*#define[ \\t]+VERSION[ \\t]+\"(([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)?)\".*"
    "\\1" ST_VERSION "${version_h_contents}"
)
IF("${ST_VERSION}" STREQUAL "${version_h_contents}")
    MESSAGE(FATAL_ERROR "Failed to extract VERSION from version.h")
ENDIF()

# Generate release date string
STRING(SUBSTRING "${ST_VERSION}" "2" "4" ST_RELEASE_YEAR)
STRING(SUBSTRING "${ST_VERSION}" "7" "2" ST_RELEASE_MONTH)
STRING(SUBSTRING "${ST_VERSION}" "10" "2" ST_RELEASE_DAY)
STRING(JOIN "-" ST_RELEASE_DATE "${ST_RELEASE_YEAR}" "${ST_RELEASE_MONTH}" "${ST_RELEASE_DAY}")

# This has to go quite early on, as otherwise we risk picking
# up an identically named header from a system include path.
INCLUDE_DIRECTORIES(src src/foundation src/math src/interaction src/zones)

# For config.h
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/src")

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(TestCXXAcceptsFlag)
INCLUDE(CMakeDependentOption)
INCLUDE(cmake/SetDefaultBuildType.cmake)
INCLUDE(cmake/SetDefaultGccFlags.cmake)
INCLUDE(cmake/ToNativePath.cmake)
INCLUDE(cmake/UpdateTranslations.cmake)
INCLUDE(cmake/CopyToBuildDir.cmake)
INCLUDE(cmake/LibToDLL.cmake)

ST_SET_DEFAULT_BUILD_TYPE(Release)
IF(CMAKE_COMPILER_IS_GNUCC)
    ST_SET_DEFAULT_GCC_FLAGS()
ENDIF(CMAKE_COMPILER_IS_GNUCC)

GET_FILENAME_COMPONENT(source_outer_dir "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)
GET_FILENAME_COMPONENT(build_outer_dir "${PROJECT_BINARY_DIR}/.." ABSOLUTE)

SET(OpenGL_GL_PREFERENCE "GLVND")
INCLUDE(FindOpenGL)
CMAKE_DEPENDENT_OPTION(
    ENABLE_OPENGL "OpenGL may be used for UI acceleration" ON
    "OPENGL_FOUND; NOT ST_SUPPRESS_OPENGL" OFF
)

INCLUDE(FindOpenCL)
CMAKE_DEPENDENT_OPTION(
    ENABLE_OPENCL "OpenCL may be used for acceleration of image processing" ON
    "OpenCL_FOUND" OFF
)

IF(ENABLE_OPENCL)
    SET(OPENCL_VERSION 120 CACHE STRING "OpenCL version. Default is 120")
    MARK_AS_ADVANCED(OPENCL_VERSION)
ENDIF()

SET(STAGING_LIBS_DIR "")

IF(WIN32)
    SET(STAGING_LIBS_DIR "${DEPS_BUILD_DIR}/staging/libs")
ENDIF()

FIND_PACKAGE(Eigen3 REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${EIGEN3_INCLUDE_DIR})
add_compile_options(-DEIGEN_MPL2_ONLY)

# Once Eigen starts using C++11 alignas() feature, this can be removed.
ADD_DEFINITIONS(-DEIGEN_DONT_ALIGN_STATICALLY)

SET(CMAKE_AUTOMOC ON)
IF(ST_USE_QT6 STREQUAL "ON")
    SET(qt_min_version 6.2.2)
    FIND_PACKAGE(Qt6Core ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt6Gui ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt6Widgets ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt6Xml ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt6Network ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt6Linguist ${qt_min_version} REQUIRED)
    IF(ENABLE_OPENGL)
        FIND_PACKAGE(Qt6OpenGL ${qt_min_version} REQUIRED)
    ENDIF()
    IF(APPLE)
        # Not used but required for loading cocoa plugin
        FIND_PACKAGE(Qt6PrintSupport ${qt_min_version} REQUIRED)
    ENDIF(APPLE)
 ELSE()
    SET(qt_min_version 5.3)
    FIND_PACKAGE(Qt5Core ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt5Gui ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt5Widgets ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt5Xml ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt5Network ${qt_min_version} REQUIRED)
    FIND_PACKAGE(Qt5LinguistTools ${qt_min_version} REQUIRED)
    IF(ENABLE_OPENGL)
        FIND_PACKAGE(Qt5OpenGL ${qt_min_version} REQUIRED)
    ENDIF()
    IF(APPLE)
        # Not used but required for loading cocoa plugin
        FIND_PACKAGE(Qt5PrintSupport ${qt_min_version} REQUIRED)
    ENDIF(APPLE)
    IF(NOT DEFINED QT_DEFAULT_MAJOR_VERSION)
        SET(QT_DEFAULT_MAJOR_VERSION 5)
    ENDIF()
ENDIF()

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES("${ZLIB_INCLUDE_DIR}")

FIND_PACKAGE(TIFF REQUIRED)
INCLUDE_DIRECTORIES("${TIFF_INCLUDE_DIR}")

IF(APPLE)
    FIND_PACKAGE(JPEG REQUIRED)
ENDIF()

IF(WIN32)
    ADD_DEFINITIONS(-DUSE_LIBTIFF_DLL)
ENDIF()

# Begin of Boost
IF(NOT ST_NO_TESTS STREQUAL "ON")
IF(WIN32)
    SET(Boost_USE_STATIC_LIBS ON)
ELSE(WIN32)
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
ENDIF(WIN32)

SET(Boost_USE_MULTITHREADED ON)
IF(POLICY CMP0167)
    FIND_PACKAGE(Boost 1.70.0 COMPONENTS unit_test_framework CONFIG)
ELSE()
    FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
ENDIF()
IF(NOT Boost_FOUND)
    MESSAGE(
        FATAL_ERROR
        "Could not find boost headers or libraries. "
        "You may need to install a package named libboost1.35-dev or similarly. "
        "Hint: create a Boost_DEBUG variable in cmake and set it to YES." 
    )
ENDIF(NOT Boost_FOUND)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF()
# End of Boost

SET(EXTRA_LIBS "")

IF(UNIX)
    FIND_PACKAGE(Threads REQUIRED)
    LINK_LIBRARIES(Threads::Threads)
ELSEIF(WIN32)
    ADD_DEFINITIONS(-DNOMINMAX)
ENDIF(UNIX)

# Begin of Boost
IF(NOT ST_NO_TESTS STREQUAL "ON")
    ADD_DEFINITIONS(-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
ENDIF()
# End of Boost

# Generate license file
## Read plain text ScanTailor license
FILE(READ "LICENSE" ST_LICENSE)
## Read HTML ScanTailor license
FILE(READ "src/resources/GPLv3.html" ST_LICENSE_HTML)
SET(ST_LICENSE_HTML "<h1>Scantailor-${ST_FAMILY}-${ST_VERSION}</h1>${ST_LICENSE_HTML}")
## Add additional licenses for dependencies
IF(ST_LICENSE_DEPS_DIR)
    SET(ST_LICENSE_SEPARATOR "\n====================\n")
    SET(ST_LICENSE_SEARCH "${ST_LICENSE_DEPS_DIR}/*.txt")
    FILE(GLOB ST_LICENSE_DEPS_FILES LIST_DIRECTORIES FALSE "${ST_LICENSE_SEARCH}")
    FOREACH(ST_LICENSE_DEPS_FILE ${ST_LICENSE_DEPS_FILES})
        FILE(READ "${ST_LICENSE_DEPS_FILE}" ST_LICENSE_TEMP)
        GET_FILENAME_COMPONENT(ST_LICENSE_DEPS_FILE_SHORTNAME "${ST_LICENSE_DEPS_FILE}" NAME_WLE)
        ### Append to plain text license
        SET(
        ST_LICENSE
        "${ST_LICENSE}${ST_LICENSE_SEPARATOR}${ST_LICENSE_DEPS_FILE_SHORTNAME}${ST_LICENSE_SEPARATOR}${ST_LICENSE_TEMP}"
        )
        ### Append to HTML license
        STRING(REGEX REPLACE "\n" "<br>\n" ST_LICENSE_TEMP "${ST_LICENSE_TEMP}")
        SET(ST_LICENSE_HTML "${ST_LICENSE_HTML}<h1>${ST_LICENSE_DEPS_FILE_SHORTNAME}</h1><p>${ST_LICENSE_TEMP}</p>")
    ENDFOREACH()
ENDIF()
## Build new license files
CONFIGURE_FILE("LICENSE_COMPLETE.in" "LICENSE_COMPLETE" NEWLINE_STYLE CRLF)
CONFIGURE_FILE("src/resources/license.html.in" "src/resources/license.html" NEWLINE_STYLE CRLF)

IF(WIN32)
    LIST(APPEND EXTRA_LIBS winmm imm32 ws2_32 ole32 oleaut32 uuid gdi32 comdlg32 winspool)
ENDIF()

IF(NOT TIFF_LIBRARY)
    LIST(APPEND EXTRA_LIBS ${TIFF_LIBRARIES})
ELSE()
    LIST(APPEND EXTRA_LIBS ${TIFF_LIBRARY})
ENDIF()

IF(APPLE)
    IF(NOT JPEG_LIBRARY)
        LIST(APPEND EXTRA_LIBS ${JPEG_LIBRARIES})
    ELSE()
        LIST(APPEND EXTRA_LIBS ${JPEG_LIBRARY})
    ENDIF()
ENDIF()

IF(NOT ZLIB_LIBRARY)
    LIST(APPEND EXTRA_LIBS ${ZLIB_LIBRARIES})
ELSE()
    LIST(APPEND EXTRA_LIBS ${ZLIB_LIBRARY})
ENDIF()

SET(MAYBE_QT_OPENGL_MODULE "")
IF(ENABLE_OPENGL)
    LIST(APPEND EXTRA_LIBS ${OPENGL_LIBRARIES})
    IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
        SET(MAYBE_QT_OPENGL_MODULE Qt5::OpenGL)
    ELSE()
        SET(MAYBE_QT_OPENGL_MODULE Qt6::OpenGL)
    ENDIF()
ENDIF()

# Crash reporter, only Windows + MSVC supported for now.
# Must go before generating config.h, as ENABLE_CRASH_REPORTER
# goes in there.
IF(MSVC)
    OPTION(ENABLE_CRASH_REPORTER "Enable crash reporter (only for official builds)" OFF)
ENDIF(MSVC)

# Prepare config.h
IF(APPLE)
    SET(TRANSLATION_DIRS "../Resources/translations")
    SET(STYLESHEETS_DIR "../Resources/stylesheets")
    SET(PLUGIN_DIRS "../PlugIns")
ELSEIF(WIN32)
    SET(TRANSLATION_DIRS ".:translations")
    SET(STYLESHEETS_DIR "stylesheets")
    SET(PLUGIN_DIRS ".")
ELSE()
    SET(TRANSLATION_DIRS ".:${CMAKE_INSTALL_PREFIX}/share/scantailor-experimental/translations")
    SET(STYLESHEETS_DIR "${CMAKE_INSTALL_PREFIX}/share/scantailor-experimental/stylesheets")
    SET(PLUGIN_DIRS ".:${CMAKE_INSTALL_PREFIX}/lib/scantailor-experimental")
ENDIF()

CONFIGURE_FILE(src/config.h.in ${CMAKE_BINARY_DIR}/src/config.h @ONLY)

ADD_SUBDIRECTORY(src)
INCLUDE_DIRECTORIES("${toplevel_ui_sources_BINARY_DIR}")

IF(APPLE)
    INSTALL(TARGETS scantailor-experimental BUNDLE DESTINATION .)
    INSTALL(TARGETS scantailor-experimental-cli RUNTIME DESTINATION scantailor-experimental.app/Contents/Library)
ELSE()
    INSTALL(TARGETS scantailor-experimental scantailor-experimental-cli RUNTIME DESTINATION bin)
ENDIF()

IF(ENABLE_CRASH_REPORTER)
    FIND_PATH(
        SYMBOLS_PATH . PATHS "${build_outer_dir}/symbols"
        "${source_outer_dir}/symbols" NO_DEFAULT_PATH
        DOC "The directory to write symbol information into."
    )
    IF(NOT SYMBOLS_PATH)
        MESSAGE(FATAL_ERROR "SYMBOLS_PATH directory is not set.")
    ENDIF()

    # We can't build it, because it requires ATL, which is not part
    # of the Visual Studio Express Edition, so we rely on a pre-built
    # version which can be found in the Mozilla repository.
    FIND_PROGRAM(
        DUMP_SYMS_EXECUTABLE dump_syms PATHS "${build_outer_dir}" "${source_outer_dir}"
        DOC "Path to dump_syms.exe, which can be found in Mozilla repository."
    )
    IF(NOT DUMP_SYMS_EXECUTABLE)
        MESSAGE(
            FATAL_ERROR "dump_syms.exe wasn't found.  Specify its location manually by setting the DUMP_SYMS_EXECUTABLE variable.  dump_syms.exe may be found in the Mozilla repository under /toolkit/crashreporter/tools/win32"
        )
    ENDIF()

    INCLUDE_DIRECTORIES(crash_reporter crash_reporter/google-breakpad)
    TARGET_LINK_LIBRARIES(scantailor-experimental crash_handler)
ENDIF()

# Translations
TRANSLATION_SOURCES(
    scantailor-experimental
    ${common_sources} ${gui_only_sources} ${cli_only_sources}
)
FILE(GLOB TRANSLATION_FILES src/translations/scantailor-experimental_*.ts)
FILE(GLOB CR_TRANSLATION_FILES src/translations/crashreporter_*.ts)

FINALIZE_TRANSLATION_SET(scantailor-experimental ${TRANSLATION_FILES})
IF(ENABLE_CRASH_REPORTER)
    FINALIZE_TRANSLATION_SET(crashreporter ${CR_TRANSLATION_FILES})
ENDIF()
UPDATE_TRANSLATIONS_TARGET(update_translations scantailor-experimental crashreporter)

SET(ts_files ${TRANSLATION_FILES})
IF(ENABLE_CRASH_REPORTER)
    LIST(APPEND ts_files ${CR_TRANSLATION_FILES})
ENDIF(ENABLE_CRASH_REPORTER)

# Don't build *.qm files from *untranslated.ts
SET(FILTERED_TRANSLATION_FILES)
FOREACH(ts_file ${ts_files})
    IF ("${ts_file}" MATCHES ".*untranslated.ts")
        # Just skip it.
    ELSE("${ts_file}" MATCHES ".*untranslated.ts")
        LIST(APPEND FILTERED_TRANSLATION_FILES "${ts_file}")
    ENDIF("${ts_file}" MATCHES ".*untranslated.ts")
ENDFOREACH(ts_file)

IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_ADD_TRANSLATION(QM_FILES ${FILTERED_TRANSLATION_FILES})
ELSE()
    QT6_ADD_TRANSLATION(QM_FILES ${FILTERED_TRANSLATION_FILES})
ENDIF()
ADD_CUSTOM_TARGET(compile_translations ALL DEPENDS ${QM_FILES})

IF(APPLE)
    INSTALL(FILES ${QM_FILES} DESTINATION "scantailor-experimental.app/Contents/Resources/translations")
ELSE()
    INSTALL(FILES ${QM_FILES} DESTINATION "share/scantailor-experimental/translations")
ENDIF()

IF(APPLE)
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/src/stylesheets" DESTINATION scantailor-experimental.app/Contents/Resources)
ELSE()
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/src/stylesheets" DESTINATION "share/scantailor-experimental")
ENDIF()

IF(UNIX AND NOT APPLE)
    CONFIGURE_FILE("src/io.github.ImageProcessing_ElectronicPublications.scantailor-experimental.metainfo.xml.in" "src/share/metainfo/io.github.ImageProcessing_ElectronicPublications.scantailor-experimental.metainfo.xml")
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/src/share/" DESTINATION "share")
ENDIF()

# Source code packaging
IF(APPLE)
    SET(CPACK_PACKAGE_NAME "scantailor-experimental")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "scantailor-experimental")

    SET(CPACK_PACKAGE_VERSION_MAJOR ${ST_RELEASE_YEAR})
    SET(CPACK_PACKAGE_VERSION_MINOR ${ST_RELEASE_MONTH})
    SET(CPACK_PACKAGE_VERSION_PATCH ${ST_RELEASE_DAY})

    SET(CPACK_SOURCE_GENERATOR TGZ)
    SET(CPACK_GENERATOR DragNDrop)

    SET(CPACK_BINARY_BUNDLE OFF)
    SET(CPACK_BINARY_DRAGNDROP ON)
    SET(CPACK_BINARY_PRODUCTBUILD OFF)
    SET(CPACK_BINARY_TGZ ON)
    SET(CPACK_BINARY_STGZ OFF)
ELSE()
    SET(CPACK_CMAKE_GENERATOR "")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "scantailor-experimental-${ST_VERSION}")
ENDIF()

SET(
    CPACK_SOURCE_IGNORE_FILES
    "/\\\\.svn/"
    "/\\\\.git/"
    "~$"
    "\\\\.pcs$"
    "TODO.txt"
    "CMakeLists.txt.user"
    "/doxygen/"
    "${CMAKE_BINARY_DIR}"
)
INCLUDE(CPack)

IF(APPLE)
    INSTALL(IMPORTED_RUNTIME_ARTIFACTS scantailor-experimental
        RUNTIME_DEPENDENCY_SET runtime
        BUNDLE DESTINATION .
    )

    INSTALL(RUNTIME_DEPENDENCY_SET runtime
        DESTINATION scantailor-experimental.app/Contents/Frameworks
        PRE_EXCLUDE_REGEXES "^/System" "^/Library" "^/usr/lib"
        POST_EXCLUDE_REGEXES ""
    )
    
    IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
        CMAKE_PATH(SET QT_PLUGINS_DIR NORMALIZE ${Qt5Core_DIR}/../../../plugins)
    ELSE()
        CMAKE_PATH(SET QT_PLUGINS_DIR NORMALIZE ${Qt6Core_DIR}/../../../plugins)
    ENDIF()

    set(PLUGINS_DESTINATION scantailor-experimental.app/Contents/PlugIns)

    INSTALL(FILES ${QT_PLUGINS_DIR}/platforms/libqcocoa.dylib    DESTINATION ${PLUGINS_DESTINATION}/platforms)

    INSTALL(FILES ${QT_PLUGINS_DIR}/styles/libqmacstyle.dylib    DESTINATION ${PLUGINS_DESTINATION}/styles)

    INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/libqico.dylib   DESTINATION ${PLUGINS_DESTINATION}/imageformats)
    INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/libqjpeg.dylib  DESTINATION ${PLUGINS_DESTINATION}/imageformats)
    INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/libqsvg.dylib   DESTINATION ${PLUGINS_DESTINATION}/imageformats)
    INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/libqtiff.dylib  DESTINATION ${PLUGINS_DESTINATION}/imageformats)

    IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
        SET(QtCore_DIR "${Qt5Core_DIR}")
    ELSE()
        SET(QtCore_DIR "${Qt6Core_DIR}")
    ENDIF()

    CMAKE_PATH(SET Qt_DIR NORMALIZE "${QtCore_DIR}/../../..")
    
    FOREACH(qm_file ${QM_FILES})
        STRING(REGEX REPLACE "^.*scantailor-experimental_" "${Qt_DIR}translations/qtbase_" qtbase_qm_file "${qm_file}")
        IF(EXISTS "${qtbase_qm_file}")
            INSTALL(FILES "${qtbase_qm_file}" DESTINATION "scantailor-experimental.app/Contents/Resources/translations")
        ENDIF()
        STRING(REGEX REPLACE "^.*scantailor-experimental_(.*)\.qm" "\\1" qtbase_qm_name "${qm_file}")
        INSTALL(DIRECTORY DESTINATION "scantailor-experimental.app/Contents/Resources/${qtbase_qm_name}.lproj")
    ENDFOREACH()
ELSEIF(WIN32)
    # Copy some DLLs to the staging dir.

    find_file(LIBJPEG_DLL
        NAMES libjpeg.dll jpeg.dll jpeg62.dll jpeg-62.dll libjpeg-62.dll
        PATHS ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES bin lib
        NO_DEFAULT_PATH)
    IF(NOT LIBJPEG_DLL)
        message(SEND_ERROR "Could not find LIBJPEG dll. You may specify it manually.")
    ENDIF()

    find_file(LIBTIFF_DLL
        NAMES libtiff.dll tiff.dll
        PATHS ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES bin lib
        NO_DEFAULT_PATH)
    IF(NOT LIBTIFF_DLL)
        message(SEND_ERROR "Could not find LIBTIFF dll. You may specify it manually.")
    ENDIF()

    find_file(ZLIB_DLL
        NAMES zdll.dll zlib.dll zlib1.dll z.dll libzlib.dll
        PATHS ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES bin lib
        NO_DEFAULT_PATH)
    IF(NOT ZLIB_DLL)
        message(SEND_ERROR "Could not find ZLIB dll. You may specify it manually.")
    ENDIF()

    SET(DLL_DIR "${QT_SRC_DIR}/qtbase/bin")
    COPY_TO_BUILD_DIR(
        "${LIBJPEG_DLL}"
        "${LIBTIFF_DLL}" "${ZLIB_DLL}"
    )
    
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    SET(QtWidgets_LIBRARIES "${Qt5Widgets_LIBRARIES}")
    SET(QtGui_LIBRARIES "${Qt5Gui_LIBRARIES}")
    SET(QtCore_LIBRARIES "${Qt5Core_LIBRARIES}")
    SET(QtXml_LIBRARIES "${Qt5Xml_LIBRARIES}")
    SET(QtOpenGL_LIBRARIES "${Qt5OpenGL_LIBRARIES}")
    SET(QtCore_DIR "${Qt5Core_DIR}")
ELSE()
    SET(QtWidgets_LIBRARIES "${Qt6Widgets_LIBRARIES}")
    SET(QtGui_LIBRARIES "${Qt6Gui_LIBRARIES}")
    SET(QtCore_LIBRARIES "${Qt6Core_LIBRARIES}")
    SET(QtXml_LIBRARIES "${Qt6Xml_LIBRARIES}")
    SET(QtOpenGL_LIBRARIES "${Qt6OpenGL_LIBRARIES}")
    SET(QtCore_DIR "${Qt6Core_DIR}")
ENDIF()
    
    # Qt's libs.
    SET(
        qt_libs
        ${QtWidgets_LIBRARIES} ${QtGui_LIBRARIES}
        ${QtCore_LIBRARIES} ${QtXml_LIBRARIES}
    )
    IF(ENABLE_OPENGL)
        LIST(APPEND qt_libs ${QtOpenGL_LIBRARIES})
    ENDIF()
    FOREACH(target ${qt_libs})
        GET_TARGET_PROPERTY(debug_loc "${target}" LOCATION_DEBUG)
        GET_TARGET_PROPERTY(release_loc "${target}" LOCATION_RELEASE)
        COPY_TO_BUILD_DIR("${debug_loc}" CONFIGURATIONS Debug)
        COPY_TO_BUILD_DIR("${release_loc}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDFOREACH()

    # Qt's plugins.
    cmake_path(SET Qt_DIR NORMALIZE "${QtCore_DIR}/../../..")
    cmake_path(APPEND Qt_DIR "plugins" OUTPUT_VARIABLE PLUGINS_DIR)

    IF(EXISTS "${PLUGINS_DIR}/imageformats/qjpeg.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/imageformats/qjpeg.dll" SUBDIR imageformats
        CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/imageformats/qjpegd.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/imageformats/qjpegd.dll" SUBDIR imageformats
        CONFIGURATIONS Debug)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/imageformats/qtiff.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/imageformats/qtiff.dll" SUBDIR imageformats
        CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/imageformats/qtiffd.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/imageformats/qtiffd.dll" SUBDIR imageformats
        CONFIGURATIONS Debug)
    ENDIF()
    
    IF(EXISTS "${PLUGINS_DIR}/platforms/qwindows.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/platforms/qwindows.dll" SUBDIR platforms
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDIF()
    
    IF(EXISTS "${PLUGINS_DIR}/platforms/qwindowsd.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/platforms/qwindowsd.dll" SUBDIR platforms
            CONFIGURATIONS Debug)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/styles/qwindowsvistastyle.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/styles/qwindowsvistastyle.dll" SUBDIR styles
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/styles/qwindowsvistastyled.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/styles/qwindowsvistastyled.dll" SUBDIR styles
            CONFIGURATIONS Debug)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/styles/qmodernwindowsstyle.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/styles/qmodernwindowsstyle.dll" SUBDIR styles
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/styles/qmodernwindowsstyled.dll")
        COPY_TO_BUILD_DIR("${PLUGINS_DIR}/styles/qmodernwindowsstyled.dll" SUBDIR styles
            CONFIGURATIONS Debug)
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/accessible/qtaccessiblewidgets.dll")
        COPY_TO_BUILD_DIR(
            "${PLUGINS_DIR}/accessible/qtaccessiblewidgets.dll" SUBDIR accessible
            CONFIGURATIONS Release MinSizeRel RelWithDebInfo
        )
    ENDIF()

    IF(EXISTS "${PLUGINS_DIR}/accessible/qtaccessiblewidgetsd.dll")
        COPY_TO_BUILD_DIR(
            "${PLUGINS_DIR}/accessible/qtaccessiblewidgetsd.dll" SUBDIR accessible
            CONFIGURATIONS Debug
        )
    ENDIF()
    
    #Qt's translations
    FOREACH(qm_file ${QM_FILES})
	    STRING(REGEX REPLACE "^.*scantailor-experimental_" "${Qt_DIR}/translations/qtbase_" qtbase_qm_file "${qm_file}")
        IF(EXISTS "${qtbase_qm_file}")
            COPY_TO_BUILD_DIR( "${qtbase_qm_file}" SUBDIR translations )
        ENDIF()
    ENDFOREACH()
    
    # Generate the target that will actually do the copying.
    GENERATE_COPY_TO_BUILD_DIR_TARGET(copy_to_build_dir)
    
    # Installer for Windows.  Must go last.
    ADD_SUBDIRECTORY(packaging/windows)
ENDIF()
