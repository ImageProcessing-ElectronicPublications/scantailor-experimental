INCLUDE_DIRECTORIES(BEFORE ..)

SET(
    sources
    main.cpp TestContentSpanFinder.cpp
    TestSmartFilenameOrdering.cpp
    TestQtPolygonIntersection.cpp
    ../ContentSpanFinder.cpp ../ContentSpanFinder.h
    ../SmartFilenameOrdering.cpp ../SmartFilenameOrdering.h
)

SOURCE_GROUP("Sources" FILES ${sources})

SET(
    libs
    imageproc math
)
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    LIST(APPEND libs Qt5::Widgets)
ELSE()
    LIST(APPEND libs Qt6::Widgets)
ENDIF()
LIST(APPEND libs ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${EXTRA_LIBS}
)

ADD_EXECUTABLE(generic_tests ${sources})
TARGET_LINK_LIBRARIES(generic_tests ${libs})

# We want the executable located where we copy all the DLLs.
SET_TARGET_PROPERTIES(
    generic_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

ADD_TEST(NAME generic_tests COMMAND generic_tests --log_level=message)
