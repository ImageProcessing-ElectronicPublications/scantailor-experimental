PROJECT("Math library")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

SET(
    GENERIC_SOURCES
    math_config.h
    HomographicTransform.h
    SidesOfLine.cpp SidesOfLine.h
    ToLineProjector.cpp ToLineProjector.h
    ArcLengthMapper.cpp ArcLengthMapper.h
    LineBoundedByRect.cpp LineBoundedByRect.h
    LineBoundedByPolygon.cpp LineBoundedByPolygon.h
    PolylineIntersector.cpp PolylineIntersector.h
    LinearFunction.cpp LinearFunction.h
    QuadraticFunction.cpp QuadraticFunction.h
    XSpline.cpp XSpline.h
)
SOURCE_GROUP("Sources" FILES ${GENERIC_SOURCES})

SET(
    SPFIT_SOURCES
    spfit/FittableSpline.h
    spfit/FrenetFrame.cpp spfit/FrenetFrame.h
    spfit/ConstraintSet.cpp spfit/ConstraintSet.h
    spfit/SqDistApproximant.cpp spfit/SqDistApproximant.h
    spfit/ModelShape.h
    spfit/PolylineModelShape.cpp spfit/PolylineModelShape.h
    spfit/LinearForceBalancer.cpp spfit/LinearForceBalancer.h
    spfit/OptimizationResult.cpp spfit/OptimizationResult.h
    spfit/Optimizer.cpp spfit/Optimizer.h
    spfit/SplineFitter.cpp spfit/SplineFitter.h
)
SOURCE_GROUP("Sources\\Spline Fitting Framework" FILES ${SPFIT_SOURCES})

SET(
    ADIFF_SOURCES
    adiff/SparseMap.cpp adiff/SparseMap.h
    adiff/Function.cpp adiff/Function.h
)
SOURCE_GROUP("Sources\\Differentiation Framework" FILES ${ADIFF_SOURCES})

ADD_LIBRARY(math STATIC ${GENERIC_SOURCES} ${SPFIT_SOURCES} ${ADIFF_SOURCES})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(math foundation Qt5::Core)
ELSE()
    TARGET_LINK_LIBRARIES(math foundation Qt6::Core)
ENDIF()

IF(NOT STE_NO_TESTS STREQUAL "ON")
    ADD_SUBDIRECTORY(spfit/tests)
    ADD_SUBDIRECTORY(adiff/tests)
ENDIF()
