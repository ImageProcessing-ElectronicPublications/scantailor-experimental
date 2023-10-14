PROJECT(Acceleration)

INCLUDE_DIRECTORIES(.)
ADD_SUBDIRECTORY(opencl)

SET(
    sources
    acceleration_config.h
    AccelerationPlugin.h
    AcceleratableOperations.h
    NonAcceleratedOperations.cpp NonAcceleratedOperations.h
    DefaultAccelerationProvider.cpp DefaultAccelerationProvider.h
)
SOURCE_GROUP("Sources" FILES ${sources})
TRANSLATION_SOURCES(scantailor-experimental ${sources})

ADD_LIBRARY(acceleration STATIC ${sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(acceleration dewarping imageproc Qt5::Core Qt5::Gui)
ELSE()
    TARGET_LINK_LIBRARIES(acceleration dewarping imageproc Qt6::Core Qt6::Gui)
ENDIF()
