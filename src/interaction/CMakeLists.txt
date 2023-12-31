PROJECT(interaction)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    DraggableObject.h
    DraggablePoint.cpp DraggablePoint.h
    DraggableLineSegment.cpp DraggableLineSegment.h
    ObjectDragHandler.cpp ObjectDragHandler.h
    InteractionHandler.cpp InteractionHandler.h
    InteractionState.cpp InteractionState.h
    DragHandler.cpp DragHandler.h
    DragWatcher.cpp DragWatcher.h
    ZoomHandler.cpp ZoomHandler.h
    InteractiveXSpline.cpp InteractiveXSpline.h
)

SOURCE_GROUP(Sources FILES ${sources})

ADD_LIBRARY(interaction STATIC ${sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(interaction Qt5::Widgets)
ELSE()
    TARGET_LINK_LIBRARIES(interaction Qt6::Widgets)
ENDIF()
TRANSLATION_SOURCES(scantailor-experimental ${sources})
