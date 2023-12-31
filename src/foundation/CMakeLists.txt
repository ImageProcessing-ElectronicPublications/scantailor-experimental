PROJECT("Foundation library")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    foundation_config.h
    NonCopyable.h IntrusivePtr.h RefCountable.h
    CopyableByMemcpy.h
    ObjectSwapper.h
    ObjectSwapperFactory.cpp ObjectSwapperFactory.h
    ObjectSwapperImpl.h
    ObjectSwapperImplGrid.h
    ObjectSwapperImplQImage.cpp ObjectSwapperImplQImage.h
    AlignedArray.h
    CachingFactory.h
    FastQueue.h
    SafeDeletingQObjectPtr.h
    ScopedIncDec.h ScopedDecInc.h
    Span.h VirtualFunction.h FlagOps.h
    AutoRemovingFile.cpp AutoRemovingFile.h
    Proximity.cpp Proximity.h
    RoundingHasher.cpp RoundingHasher.h
    Property.h
    PropertyFactory.cpp PropertyFactory.h
    PropertySet.cpp PropertySet.h
    PerformanceTimer.cpp PerformanceTimer.h
    GridLineTraverser.cpp GridLineTraverser.h
    LineIntersectionScalar.cpp LineIntersectionScalar.h
    XmlMarshaller.cpp XmlMarshaller.h
    XmlUnmarshaller.cpp XmlUnmarshaller.h
    DebugImages.cpp DebugImages.h
    RelativeMargins.h
    MaxAccumulator.h
    MinMaxAccumulator.h
    NumericTraits.h
    VecNT.h
    ToVec.h
    ToPoint.h
    PriorityQueue.h
    Grid.h
    GridAccessor.h
    ValueConv.h
    IndexSequence.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(foundation STATIC ${sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(foundation Qt5::Gui Qt5::Core Qt5::Xml)
ELSE()
    TARGET_LINK_LIBRARIES(foundation Qt6::Gui Qt6::Core Qt6::Xml)
ENDIF()
