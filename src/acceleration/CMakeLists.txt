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
TARGET_LINK_LIBRARIES(acceleration dewarping imageproc Qt5::Core Qt5::Gui)
