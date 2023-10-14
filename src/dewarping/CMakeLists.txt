PROJECT("Dewarping library")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    dewarping_config.h
    Curve.cpp Curve.h
    DetectVerticalBounds.cpp DetectVerticalBounds.h
    DepthPerception.cpp DepthPerception.h
    DistortionModel.cpp DistortionModel.h
    DistortionModelBuilder.cpp DistortionModelBuilder.h
    TextLineTracer.cpp TextLineTracer.h
    TextLineRefiner.cpp TextLineRefiner.h
    TextLineSegmenter.cpp TextLineSegmenter.h
    TopBottomEdgeTracer.cpp TopBottomEdgeTracer.h
    CylindricalSurfaceDewarper.cpp CylindricalSurfaceDewarper.h
    RasterDewarper.cpp RasterDewarper.h
    DewarpingImageTransform.cpp DewarpingImageTransform.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(dewarping imageproc math foundation Qt5::Widgets Qt5::Xml)
ELSE()
    TARGET_LINK_LIBRARIES(dewarping imageproc math foundation Qt6::Widgets Qt6::Xml)
ENDIF()

TRANSLATION_SOURCES(scantailor-experimental ${sources})
