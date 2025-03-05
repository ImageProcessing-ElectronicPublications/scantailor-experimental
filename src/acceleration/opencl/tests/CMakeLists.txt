INCLUDE_DIRECTORIES(BEFORE ..)

SET(
    sources
    "${CMAKE_SOURCE_DIR}/src/tests/main.cpp"
    TestCopy.cpp
    TestTranspose.cpp
    TestGaussBlur.cpp
    TestDewarp.cpp
    TestAffineTransform.cpp
    TestRenderPolynomialSurface.cpp
    TestSavGolFilter.cpp
    TestBinaryFill.cpp
    TestBinaryRasterOp.cpp
    TestHitMissTransform.cpp
    Utils.cpp Utils.h
)
SOURCE_GROUP("Sources" FILES ${sources})

SET(
    libs
    opencl dewarping imageproc math foundation
)
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    LIST(APPEND libs Qt5::Core)
ELSE()
    LIST(APPEND libs Qt6::Core)
ENDIF()
LIST(APPEND libs
    ${OpenCL_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_PRG_EXECUTION_MONITOR_LIBRARY} ${EXTRA_LIBS}
)

ADD_EXECUTABLE(opencl_tests ${sources})
TARGET_LINK_LIBRARIES(opencl_tests ${libs})

TARGET_COMPILE_DEFINITIONS(opencl_tests
    PRIVATE
    CL_TARGET_OPENCL_VERSION=${OPENCL_VERSION}
)

# We want the executable located where we copy all the DLLs.
SET_TARGET_PROPERTIES(
    opencl_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

ADD_TEST(NAME opencl_tests COMMAND opencl_tests --log_level=message)
