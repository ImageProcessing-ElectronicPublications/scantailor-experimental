PROJECT(imageproc)

SET(
    sources
    imageproc_config.h
    Constants.h Constants.cpp
    BadAllocIfNull.cpp BadAllocIfNull.h
    BinaryImage.cpp BinaryImage.h
    BinaryThreshold.cpp BinaryThreshold.h
    SlicedHistogram.cpp SlicedHistogram.h
    BWColor.h BWPixelProxy.h
    ConnComp.h Connectivity.h
    BitOps.cpp BitOps.h
    SeedFill.cpp SeedFill.h
    ConnCompEraser.cpp ConnCompEraser.h
    ConnCompEraserExt.cpp ConnCompEraserExt.h
    GrayImage.cpp GrayImage.h
    Grayscale.cpp Grayscale.h
    RasterOp.h RasterOpGeneric.h
    UpscaleIntegerTimes.cpp UpscaleIntegerTimes.h
    ReduceThreshold.cpp ReduceThreshold.h
    Shear.cpp Shear.h
    SkewFinder.cpp SkewFinder.h
    OrthogonalRotation.cpp OrthogonalRotation.h
    Scale.cpp Scale.h
    AffineTransform.cpp AffineTransform.h
    AbstractImageTransform.h
    AffineImageTransform.cpp AffineImageTransform.h
    AffineTransformedImage.cpp AffineTransformedImage.h
    Morphology.cpp Morphology.h
    IntegralImage.h
    Binarize.cpp Binarize.h
    PolygonUtils.cpp PolygonUtils.h
    PolygonRasterizer.cpp PolygonRasterizer.h
    HoughLineDetector.cpp HoughLineDetector.h
    HoughTransform.h
    GaussBlur.cpp GaussBlur.h
    Sobel.h
    MorphGradientDetect.cpp MorphGradientDetect.h
    PolynomialLine.cpp PolynomialLine.h
    PolynomialSurface.cpp PolynomialSurface.h
    SavGolKernel.cpp SavGolKernel.h
    SavGolFilter.cpp SavGolFilter.h
    ColorFilter.cpp ColorFilter.h
    ImageMetrics.cpp ImageMetrics.h
    DrawOver.cpp DrawOver.h
    AdjustBrightness.cpp AdjustBrightness.h
    SEDM.cpp SEDM.h
    ConnectivityMap.cpp ConnectivityMap.h
    InfluenceMap.cpp InfluenceMap.h
    MaxWhitespaceFinder.cpp MaxWhitespaceFinder.h
    RastLineFinder.cpp RastLineFinder.h
    ColorInterpolation.cpp ColorInterpolation.h
    LocalMinMaxGeneric.h
    SeedFillGeneric.cpp SeedFillGeneric.h
    WatershedSegmentation.cpp WatershedSegmentation.h
    TraverseBorders.h
    FindPeaksGeneric.h
    PlugHoles.h
    ColorMixer.h
    ColorForId.h
)

SOURCE_GROUP(Sources FILES ${sources})

ADD_LIBRARY(imageproc STATIC ${sources})
IF(QT_DEFAULT_MAJOR_VERSION EQUAL 5)
    TARGET_LINK_LIBRARIES(imageproc foundation Qt5::Core Qt5::Gui)
ELSE()
    TARGET_LINK_LIBRARIES(imageproc foundation Qt6::Core Qt6::Gui)
ENDIF()
IF(NOT STE_NO_TESTS STREQUAL "ON")
    ADD_SUBDIRECTORY(tests)
ENDIF()
