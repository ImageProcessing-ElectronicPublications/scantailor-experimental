PROJECT("Deskew Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

SET(
    sources
    ImageView.cpp ImageView.h
    DewarpingMode.cpp DewarpingMode.h
    DewarpingView.cpp DewarpingView.h
    RotationThumbnail.cpp RotationThumbnail.h
    DewarpingThumbnail.cpp DewarpingThumbnail.h
    Filter.cpp Filter.h
    OptionsWidget.cpp OptionsWidget.h
    Settings.cpp Settings.h
    Task.cpp Task.h
    CacheDrivenTask.cpp CacheDrivenTask.h
    Dependencies.cpp Dependencies.h
    DistortionType.cpp DistortionType.h
    RotationParams.cpp RotationParams.h
    SourceParams.cpp SourceParams.h
    PerspectiveParams.cpp PerspectiveParams.h
    DewarpingParams.cpp DewarpingParams.h
    Params.cpp Params.h
    ApplyDialog.cpp ApplyDialog.h
    Utils.cpp Utils.h
    orders/OrderByTypeProvider.cpp orders/OrderByTypeProvider.h
    orders/OrderByAngleProvider.cpp orders/OrderByAngleProvider.h
    orders/OrderByAngleAbsProvider.cpp orders/OrderByAngleAbsProvider.h
    orders/OrderByAngleObliqueProvider.cpp orders/OrderByAngleObliqueProvider.h
    orders/OrderByAngleHorProvider.cpp orders/OrderByAngleHorProvider.h
    orders/OrderByAngleVertProvider.cpp orders/OrderByAngleVertProvider.h
)
SOURCE_GROUP("Sources" FILES ${sources})

FILE(GLOB ui_files "ui/*.ui")
SOURCE_GROUP("UI Files" FILES ${ui_files})

IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_WRAP_UI(ui_sources ${ui_files})
ELSE()
    QT6_WRAP_UI(ui_sources ${ui_files})
ENDIF()

ADD_LIBRARY(deskew STATIC ${sources} ${ui_sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(deskew Qt5::Widgets Qt5::Xml)
ELSE()
    TARGET_LINK_LIBRARIES(deskew Qt6::Widgets Qt6::Xml)
ENDIF()

TRANSLATION_SOURCES(scantailor-experimental ${sources} ${ui_files})
