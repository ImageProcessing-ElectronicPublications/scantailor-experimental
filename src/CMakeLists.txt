PROJECT("Application")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

ADD_SUBDIRECTORY(ui)
INCLUDE_DIRECTORIES("${toplevel_ui_sources_BINARY_DIR}")

# crash_reporter is included unconditionally to collect translation sources from there.
ADD_SUBDIRECTORY(crash_reporter)
ADD_SUBDIRECTORY(dewarping)
ADD_SUBDIRECTORY(foundation)
ADD_SUBDIRECTORY(math)
ADD_SUBDIRECTORY(imageproc)
ADD_SUBDIRECTORY(interaction)
ADD_SUBDIRECTORY(zones)
ADD_SUBDIRECTORY(acceleration)
IF(NOT STE_NO_TESTS STREQUAL "ON")
    ADD_SUBDIRECTORY(tests)
ENDIF()

ADD_SUBDIRECTORY(stages)

SET(resource_files resources/resources.qrc resources/icons/COPYING)
IF(WIN32)
    LIST(APPEND resource_files resources/win32/resources.rc)
ENDIF()
SOURCE_GROUP("Resources" FILES ${resource_files})

FILE(WRITE "${CMAKE_BINARY_DIR}/src/resources/license.qrc" "<RCC><qresource prefix=\"/\"><file>license.html</file></qresource></RCC>")
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    QT5_ADD_RESOURCES(resource_sources resources/resources.qrc "${CMAKE_BINARY_DIR}/src/resources/license.qrc")
ELSE()
    QT6_ADD_RESOURCES(resource_sources resources/resources.qrc "${CMAKE_BINARY_DIR}/src/resources/license.qrc")
ENDIF()
SOURCE_GROUP("Generated" FILES ${resource_sources})

SET(
    common_sources
    BackgroundExecutor.cpp BackgroundExecutor.h
    OpenGLSupport.cpp OpenGLSupport.h
    PixmapRenderer.cpp PixmapRenderer.h
    BubbleAnimation.cpp BubbleAnimation.h
    ProcessingIndicationWidget.cpp ProcessingIndicationWidget.h
    NonOwningWidget.cpp NonOwningWidget.h
    ContentBox.cpp ContentBox.h
    SmartFilenameOrdering.cpp SmartFilenameOrdering.h
    AbstractRelinker.h
    RelinkablePath.cpp RelinkablePath.h
    ImageInfo.cpp ImageInfo.h
    ImageFileInfo.h
    ImageMetadata.cpp ImageMetadata.h
    RecentProjects.cpp RecentProjects.h
    OutOfMemoryHandler.cpp OutOfMemoryHandler.h
    CommandLine.cpp CommandLine.h
    PageSelectionAccessor.cpp PageSelectionAccessor.h
    PageSelectionProvider.h
    ContentSpanFinder.cpp ContentSpanFinder.h
    ImagePixmapUnion.h
    ImageViewBase.cpp ImageViewBase.h
    BasicImageView.cpp BasicImageView.h
    DebugImageView.cpp DebugImageView.h
    VectorFieldImageView.cpp VectorFieldImageView.h
    TabbedDebugImages.cpp TabbedDebugImages.h
    ThumbnailLoadResult.h
    ThumbnailPixmapCache.cpp ThumbnailPixmapCache.h
    ThumbnailBase.cpp ThumbnailBase.h
    ThumbnailFactory.cpp ThumbnailFactory.h
    IncompleteThumbnail.cpp IncompleteThumbnail.h
    ContentBoxPropagator.cpp ContentBoxPropagator.h
    PageOrientationPropagator.cpp PageOrientationPropagator.h
    DebugImagesImpl.cpp DebugImagesImpl.h
    DebugViewFactory.h
    ImageId.cpp ImageId.h
    PageId.cpp PageId.h
    PageInfo.cpp PageInfo.h
    BackgroundTask.cpp BackgroundTask.h
    ProcessingTaskQueue.cpp ProcessingTaskQueue.h
    PageSequence.cpp PageSequence.h
    StageSequence.cpp StageSequence.h
    ProjectPages.cpp ProjectPages.h
    ImageMetadataLoader.cpp ImageMetadataLoader.h
    TiffReader.cpp TiffReader.h
    TiffWriter.cpp TiffWriter.h
    PngMetadataLoader.cpp PngMetadataLoader.h
    TiffMetadataLoader.cpp TiffMetadataLoader.h
    JpegMetadataLoader.cpp JpegMetadataLoader.h
    ImageLoader.cpp ImageLoader.h
    ErrorWidget.cpp ErrorWidget.h
    OrthogonalRotation.cpp OrthogonalRotation.h
    WorkerThreadPool.cpp WorkerThreadPool.h
    LoadFileTask.cpp LoadFileTask.h
    FilterOptionsWidget.h
    TaskStatus.h FilterUiInterface.h
    ProjectReader.cpp ProjectReader.h
    ProjectWriter.cpp ProjectWriter.h
    AtomicFileOverwriter.cpp AtomicFileOverwriter.h
    EstimateBackground.cpp EstimateBackground.h
    Despeckle.cpp Despeckle.h
    FileNameDisambiguator.cpp FileNameDisambiguator.h
    OutputFileNameGenerator.cpp OutputFileNameGenerator.h
    PageRange.cpp PageRange.h
    SelectedPage.cpp SelectedPage.h
    Utils.cpp Utils.h
    PageView.h
    AutoManualMode.h
    AbstractCommand.h
    AbstractFilter.h
    BeforeOrAfter.h
    FilterResult.h
    CompositeCacheDrivenTask.h
    ChangedStateItemDelegate.h
    PageOrderProvider.h
    PageOrderOption.h
    PayloadEvent.h
    filter_dc/AbstractFilterDataCollector.h
    filter_dc/ThumbnailCollector.h
    filter_dc/ContentBoxCollector.h
    filter_dc/PageOrientationCollector.h
    version.h
    config.h.in
    ${common_ui_files}
)

SET(
    gui_only_sources
    Application.cpp Application.h
    SkinnedButton.cpp SkinnedButton.h
    StageListView.cpp StageListView.h
    RelinkablePathVisualization.cpp RelinkablePathVisualization.h
    RelinkingModel.cpp RelinkingModel.h
    RelinkingSortingModel.cpp RelinkingSortingModel.h
    RelinkingListView.cpp RelinkingListView.h
    RelinkingDialog.cpp RelinkingDialog.h
    SettingsDialog.cpp SettingsDialog.h
    LoadFilesStatusDialog.cpp LoadFilesStatusDialog.h
    ProjectCreationContext.cpp ProjectCreationContext.h
    ProjectOpeningContext.cpp ProjectOpeningContext.h
    OutOfMemoryDialog.cpp OutOfMemoryDialog.h
    ThumbnailSequence.cpp ThumbnailSequence.h
    ProjectFilesDialog.cpp ProjectFilesDialog.h
    NewOpenProjectPanel.cpp NewOpenProjectPanel.h
    SystemLoadWidget.cpp SystemLoadWidget.h
    MainWindow.cpp MainWindow.h
    main.cpp
)

SET(
    cli_only_sources
    ConsoleBatch.cpp ConsoleBatch.h
    main-cli.cpp
)

SOURCE_GROUP("Sources" FILES ${common_sources} ${gui_only_sources} ${cli_only_sources})
SOURCE_GROUP("Special Headers" FILES version.h config.h.in)

IF(CMAKE_COMPILER_IS_GNUCXX)
    # There seems to be a bug in either gcc or boost that makes page sorting either crash
    # or go into infinite loop.  It's reproducible on a very simple test case on at least
    # gcc 4.4.3 and gcc 4.4.4.  It's not reproducible on gcc 4.5.1.
    # Adding -fno-strict-aliasing seems to fix it.
    SET_SOURCE_FILES_PROPERTIES(
        ThumbnailSequence.cpp PROPERTIES
        COMPILE_FLAGS "-fno-strict-aliasing"
    )
ENDIF()

SET(win32_resource_file "")
IF(WIN32)
    SET(rc_file "${CMAKE_CURRENT_SOURCE_DIR}/resources/win32/resources.rc")
    FILE(GLOB win32_resources resources/win32/*.ico)
    SET_SOURCE_FILES_PROPERTIES(
        "${rc_file}" PROPERTIES
        OBJECT_DEPENDS ${win32_resources}
    )
    SET(win32_resource_file "${rc_file}")
ENDIF(WIN32)

ADD_LIBRARY(stcore STATIC ${common_sources} ${common_ui_sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(stcore Qt5::Widgets Qt5::Xml ${MAYBE_QT_OPENGL_MODULE})
ELSE()
    TARGET_LINK_LIBRARIES(stcore Qt6::Widgets Qt6::Xml ${MAYBE_QT_OPENGL_MODULE})
ENDIF()
ADD_DEPENDENCIES(stcore toplevel_ui_sources)

ADD_EXECUTABLE(scantailor-experimental WIN32 ${gui_only_sources} ${resource_sources} ${win32_resource_file})
TARGET_LINK_LIBRARIES(
    scantailor-experimental
    acceleration page_layout output
    fix_orientation page_split deskew select_content stcore
    dewarping zones interaction imageproc math foundation
    ${EXTRA_LIBS}
)
SET_TARGET_PROPERTIES(
    scantailor-experimental PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

ADD_EXECUTABLE(scantailor-experimental-cli ${cli_only_sources} ${common_ui_sources})
TARGET_LINK_LIBRARIES(
    scantailor-experimental-cli
    acceleration page_layout output
    fix_orientation page_split deskew select_content stcore
    dewarping zones interaction imageproc math foundation
    ${EXTRA_LIBS}
)
SET_TARGET_PROPERTIES(
    scantailor-experimental-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

