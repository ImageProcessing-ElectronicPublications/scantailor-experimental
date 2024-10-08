PROJECT("Page Layout Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    Utils.cpp Utils.h
    ImageView.cpp ImageView.h
    Filter.cpp Filter.h
    OptionsWidget.cpp OptionsWidget.h
    Task.cpp Task.h
    CacheDrivenTask.cpp CacheDrivenTask.h
    Params.cpp Params.h
    Settings.cpp Settings.h
    Thumbnail.cpp Thumbnail.h
    ApplyDialog.cpp ApplyDialog.h
    Alignment.cpp Alignment.h
    MatchSizeMode.cpp MatchSizeMode.h
    Framings.cpp Framings.h
    PageLayout.cpp PageLayout.h
    orders/OrderByWidthProvider.cpp orders/OrderByWidthProvider.h
    orders/OrderByHeightProvider.cpp orders/OrderByHeightProvider.h
    orders/OrderByRatioProvider.cpp orders/OrderByRatioProvider.h
    orders/OrderByAreaProvider.cpp orders/OrderByAreaProvider.h
)
SOURCE_GROUP("Sources" FILES ${sources})

FILE(GLOB ui_files "ui/*.ui")
SOURCE_GROUP("UI Files" FILES ${ui_files})

IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_WRAP_UI(ui_sources ${ui_files})
ELSE()
    QT6_WRAP_UI(ui_sources ${ui_files})
ENDIF()

ADD_LIBRARY(page_layout STATIC ${sources} ${ui_sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(page_layout Qt5::Widgets Qt5::Xml)
ELSE()
    TARGET_LINK_LIBRARIES(page_layout Qt6::Widgets Qt6::Xml)
ENDIF()

TRANSLATION_SOURCES(scantailor-experimental ${sources} ${ui_files})
