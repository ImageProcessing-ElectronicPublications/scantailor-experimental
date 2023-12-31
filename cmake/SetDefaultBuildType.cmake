MACRO(ST_SET_DEFAULT_BUILD_TYPE TYPE_)
    IF(NOT CMAKE_BUILD_TYPE AND NOT DEFAULT_BUILD_TYPE_SET)
        SET(DEFAULT_BUILD_TYPE_SET TRUE CACHE INTERNAL "" FORCE)
        SET(
            CMAKE_BUILD_TYPE "${TYPE_}" CACHE STRING
            "Build type (Release Debug RelWithDebInfo MinSizeRel)" FORCE
        )
    ENDIF(NOT CMAKE_BUILD_TYPE AND NOT DEFAULT_BUILD_TYPE_SET)
ENDMACRO(ST_SET_DEFAULT_BUILD_TYPE)
