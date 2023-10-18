FIND_PROGRAM(
    MAKENSIS_EXE makensis
    PATHS "[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\NSIS;InstallLocation]"
)
IF(NOT MAKENSIS_EXE)
    MESSAGE(
        FATAL_ERROR
        "makensis.exe could not be found.\n"
        "makensis.exe is a part of NSIS. Get NSIS from http://nsis.sf.net/"
    )
ENDIF(NOT MAKENSIS_EXE)
    
IF(MSVC)
    include(InstallRequiredSystemLibraries)
    list(GET CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS 0 CRT_REDIST_PATH)
    cmake_path(REMOVE_FILENAME CRT_REDIST_PATH)
ENDIF()

MATH(EXPR bits "${CMAKE_SIZEOF_VOID_P} * 8")
SET(LICENSE_FILE "${CMAKE_SOURCE_DIR}/LICENSE")
IF(ST_ARCH STREQUAL "ARM")
SET(INSTALLER_FILENAME "scantailor-${ST_FAMILY_LOWER}-${ST_VERSION}-arm${bits}-install.exe")
ELSE()
SET(INSTALLER_FILENAME "scantailor-${ST_FAMILY_LOWER}-${ST_VERSION}-${bits}bit-install.exe")
ENDIF()
SET(REGISTER_EXTENSION_NSH "${CMAKE_CURRENT_SOURCE_DIR}/registerExtension.nsh")

SET(
    SOURCES
    "scantailor-${ST_FAMILY_LOWER}.nsi.in" "${CMAKE_SOURCE_DIR}/cmake/generate_nsi_file.cmake.in"
    "${CMAKE_SOURCE_DIR}/cmake/prepare_staging_dir.cmake.in"
)

SOURCE_GROUP("Sources" FILES ${SOURCES})
SOURCE_GROUP("Generated" FILES "${CMAKE_BINARY_DIR}/scantailor-${ST_FAMILY_LOWER}.nsi")

CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake/prepare_staging_dir.cmake.in"
    "${CMAKE_BINARY_DIR}/prepare_staging_dir.cmake" @ONLY
)

SET(
    dist_targets
    "scantailor-${ST_FAMILY_LOWER}" "scantailor-${ST_FAMILY_LOWER}-cli" CrashReporter opencl_plugin
)
SET(BUILT_BINARIES "")
FOREACH(target ${dist_targets})
    IF(TARGET ${target})
        LIST(APPEND BUILT_BINARIES "$<TARGET_FILE:${target}>")
    ENDIF()
ENDFOREACH()

SET(MAYBE_CRT_REDIST_PATH "")
IF(CRT_REDIST_PATH)
    SET(MAYBE_CRT_REDIST_PATH "-DCRT_REDIST_PATH=${CRT_REDIST_PATH}")
ENDIF()
ADD_CUSTOM_TARGET(
    prepare_staging_dir
    COMMAND "${CMAKE_COMMAND}" ARGS -DCFG=$<CONFIGURATION>
    "-DCONF_BUILD_DIR=$<TARGET_FILE_DIR:scantailor-experimental>"
    "-DBUILT_BINARIES=${BUILT_BINARIES}"
    ${MAYBE_CRASHREPORTER_EXE}
    ${MAYBE_OPENCL_PLUGIN_DLL}
    ${MAYBE_CRT_REDIST_PATH}
    -P "${CMAKE_BINARY_DIR}/prepare_staging_dir.cmake"
    DEPENDS "${CMAKE_SOURCE_DIR}/cmake/prepare_staging_dir.cmake.in"
    VERBATIM
)
ADD_DEPENDENCIES(
    prepare_staging_dir
    "scantailor-${ST_FAMILY_LOWER}" "scantailor-${ST_FAMILY_LOWER}-cli" compile_translations
)
IF(ENABLE_CRASH_REPORTER)
    ADD_DEPENDENCIES(prepare_staging_dir CrashReporter)
ENDIF()

SET(extra_deps "scantailor-${ST_FAMILY_LOWER}" compile_translations)

SET(DUMP_SYMBOLS_COMMANDS "")
IF(ENABLE_CRASH_REPORTER)
    LIST(APPEND extra_deps CrashReporter)
    SET(
        DUMP_SYMBOLS_COMMANDS
        COMMAND "${CMAKE_COMMAND}" ARGS -E echo "Dumping symbols..."
        
        COMMAND "${DUMP_SYMS_EXECUTABLE}" ARGS "$<TARGET_FILE:scantailor-experimental>"
        ">" "${SYMBOLS_PATH}/temp.sym"
        COMMAND "${CMAKE_COMMAND}" ARGS "-DSYMBOLS_PATH=${SYMBOLS_PATH}"
        -P "${CMAKE_SOURCE_DIR}/cmake/move_sym_file.cmake"
        
        COMMAND "${DUMP_SYMS_EXECUTABLE}" ARGS "$<TARGET_FILE:scantailor-experimental-cli>"
        ">" "${SYMBOLS_PATH}/temp.sym"
        COMMAND "${CMAKE_COMMAND}" ARGS "-DSYMBOLS_PATH=${SYMBOLS_PATH}"
        -P "${CMAKE_SOURCE_DIR}/cmake/move_sym_file.cmake"
    )
ENDIF(ENABLE_CRASH_REPORTER)

CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake/generate_nsi_file.cmake.in"
    "${CMAKE_BINARY_DIR}/generate_nsi_file.cmake" @ONLY
)

SET(
    scantailor_nsi_command
    OUTPUT "${CMAKE_BINARY_DIR}/scantailor-${ST_FAMILY_LOWER}.nsi"
    COMMAND "${CMAKE_COMMAND}" "-DCFG=$<CONFIGURATION>"
)
IF(ENABLE_CRASH_REPORTER)
    LIST(
        APPEND scantailor_nsi_command
        "-DCRASHREPORTER_EXE=$<TARGET_FILE:CrashReporter>"
    )
ENDIF()
LIST(
    APPEND scantailor_nsi_command
    -P "${CMAKE_BINARY_DIR}/generate_nsi_file.cmake"
    DEPENDS scantailor-${ST_FAMILY_LOWER}.nsi.in
    "${CMAKE_SOURCE_DIR}/cmake/generate_nsi_file.cmake.in"
    VERBATIM
)
ADD_CUSTOM_COMMAND(${scantailor_nsi_command})

ADD_CUSTOM_COMMAND(
    OUTPUT "${INSTALLER_FILENAME}"
    ${DUMP_SYMBOLS_COMMANDS}
    COMMAND "${MAKENSIS_EXE}" /V2 "scantailor-${ST_FAMILY_LOWER}.nsi"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    MAIN_DEPENDENCY "${CMAKE_BINARY_DIR}/scantailor-${ST_FAMILY_LOWER}.nsi"
    DEPENDS ${extra_deps}
    VERBATIM
)

OPTION(BUILD_INSTALLER_BY_DEFAULT "Wheter the \"installer\" target gets built by default" ON)
SET(maybe_all "")
IF(BUILD_INSTALLER_BY_DEFAULT)
    SET(maybe_all ALL)
ENDIF()
ADD_CUSTOM_TARGET(
    installer ${maybe_all}
    SOURCES ${SOURCES}
    DEPENDS "${INSTALLER_FILENAME}"
)
ADD_DEPENDENCIES(installer prepare_staging_dir)
