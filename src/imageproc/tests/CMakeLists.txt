INCLUDE_DIRECTORIES(BEFORE ..)

SET(
    sources
    main.cpp
    TestBinaryImage.cpp TestReduceThreshold.cpp
    TestSlicedHistogram.cpp
    TestConnCompEraser.cpp TestConnCompEraserExt.cpp
    TestGaussBlur.cpp
    TestGrayscale.cpp
    TestHoughTransform.cpp
    TestRasterOp.cpp TestShear.cpp
    TestOrthogonalRotation.cpp
    TestSkewFinder.cpp
    TestScale.cpp
    TestAffineTransform.cpp
    TestMorphology.cpp
    TestBinarize.cpp
    TestPolygonRasterizer.cpp
    TestSeedFill.cpp
    TestSEDM.cpp
    TestRastLineFinder.cpp
    TestColorMixer.cpp
    TestSavGolKernel.cpp
    TestSavGolFilter.cpp
    Utils.cpp Utils.h
)
SOURCE_GROUP("Sources" FILES ${sources})

SET(
    libs
    imageproc math foundation 
)
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    LIST(APPEND libs Qt5::Widgets Qt5::Xml)
ELSE()
    LIST(APPEND libs Qt6::Widgets Qt6::Xml)
ENDIF()
LIST(APPEND libs
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${EXTRA_LIBS}
)

REMOVE_DEFINITIONS(-DBUILDING_IMAGEPROC)
ADD_EXECUTABLE(imageproc_tests ${sources})
TARGET_LINK_LIBRARIES(imageproc_tests ${libs})

# We want the executable located where we copy all the DLLs.
SET_TARGET_PROPERTIES(
    imageproc_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

ADD_TEST(NAME imageproc_tests COMMAND imageproc_tests --log_level=message)
