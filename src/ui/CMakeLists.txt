PROJECT(toplevel_ui_sources) # Don't rename - it's referenced elsewhere.

FILE(GLOB ui_files "*.ui")
SOURCE_GROUP("UI Files" FILES ${ui_files})

IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_WRAP_UI(ui_sources ${ui_files})
ELSE()
    QT6_WRAP_UI(ui_sources ${ui_files})
ENDIF()
ADD_CUSTOM_TARGET(toplevel_ui_sources SOURCES ${ui_sources})

TRANSLATION_SOURCES(scantailor-experimental ${ui_files})
