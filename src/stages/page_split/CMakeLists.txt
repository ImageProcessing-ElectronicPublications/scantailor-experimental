PROJECT("Page Split Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    SplitLineObject.h
    ImageView.cpp ImageView.h
    Thumbnail.cpp Thumbnail.h
    Params.cpp Params.h
    Dependencies.cpp Dependencies.h
    PageLayout.cpp PageLayout.h
    PageLayoutEstimator.cpp PageLayoutEstimator.h
    VertLineFinder.cpp VertLineFinder.h
    Filter.cpp Filter.h
    OptionsWidget.cpp OptionsWidget.h
    SplitModeDialog.cpp SplitModeDialog.h
    Settings.cpp Settings.h
    Task.cpp Task.h
    CacheDrivenTask.cpp CacheDrivenTask.h
    LayoutType.cpp LayoutType.h
    UnremoveButton.cpp UnremoveButton.h
    orders/OrderBySplitTypeProvider.cpp orders/OrderBySplitTypeProvider.h
    orders/OrderBySplitPosProvider.cpp orders/OrderBySplitPosProvider.h
    orders/OrderBySplitOffsetProvider.cpp orders/OrderBySplitOffsetProvider.h
)
SOURCE_GROUP("Sources" FILES ${sources})

FILE(GLOB ui_files "ui/*.ui")
SOURCE_GROUP("UI Files" FILES ${ui_files})

IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_WRAP_UI(ui_sources ${ui_files})
ELSE()
    QT6_WRAP_UI(ui_sources ${ui_files})
ENDIF()

ADD_LIBRARY(page_split STATIC ${sources} ${ui_sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(page_split Qt5::Widgets Qt5::Xml)
ELSE()
    TARGET_LINK_LIBRARIES(page_split Qt6::Widgets Qt6::Xml)
ENDIF()

TRANSLATION_SOURCES(scantailor-experimental ${sources} ${ui_files})
