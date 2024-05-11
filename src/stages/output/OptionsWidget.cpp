/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/foreach.hpp>
#include <QtGlobal>
#include <QVariant>
#include <QColorDialog>
#include <QToolTip>
#include <QString>
#include <QCursor>
#include <QPoint>
#include <QSize>
#include <Qt>
#include <QDebug>
#include "OptionsWidget.h"
#include "ApplyColorsDialog.h"
#include "Settings.h"
#include "Params.h"
#include "dewarping/DistortionModel.h"
#include "DespeckleLevel.h"
#include "ZoneSet.h"
#include "PictureZoneComparator.h"
#include "FillZoneComparator.h"
#include "../../Utils.h"
#include "ScopedIncDec.h"
#include "config.h"

using namespace dewarping;

namespace output
{

OptionsWidget::OptionsWidget(
    IntrusivePtr<Settings> const& settings,
    PageSelectionAccessor const& page_selection_accessor)
    :   m_ptrSettings(settings),
        m_pageSelectionAccessor(page_selection_accessor),
        m_despeckleLevel(DESPECKLE_NORMAL),
        m_despeckleFactor(despeckleLevelToFactor(DESPECKLE_NORMAL)),
        m_lastTab(TAB_OUTPUT),
        m_ignoreThresholdChanges(0),
        m_ignoreDespeckleLevelChanges(0),
        m_ignoreScaleChanges(0),
        m_colorFilterCurrent(F_WIENER)
{
    setupUi(this);

    colorModeSelector->addItem(tr("Black and White"), ColorParams::BLACK_AND_WHITE);
    colorModeSelector->addItem(tr("Color / Grayscale"), ColorParams::COLOR_GRAYSCALE);
    colorModeSelector->addItem(tr("Mixed"), ColorParams::MIXED);

    thresholdMethodSelector->addItem(tr("Otsu"), OTSU);
    thresholdMethodSelector->addItem(tr("Mean"), MEANDELTA);
    thresholdMethodSelector->addItem(tr("Dots8"), DOTS8);
    thresholdMethodSelector->addItem(tr("Niblack"), NIBLACK);
    thresholdMethodSelector->addItem(tr("Gatos"), GATOS);
    thresholdMethodSelector->addItem(tr("Sauvola"), SAUVOLA);
    thresholdMethodSelector->addItem(tr("Wolf"), WOLF);
    thresholdMethodSelector->addItem(tr("Bradley"), BRADLEY);
    thresholdMethodSelector->addItem(tr("Singh"), SINGH);
    thresholdMethodSelector->addItem(tr("WAN"), WAN);
    thresholdMethodSelector->addItem(tr("EdgePlus"), EDGEPLUS);
    thresholdMethodSelector->addItem(tr("BlurDiv"), BLURDIV);
    thresholdMethodSelector->addItem(tr("EdgeDiv"), EDGEDIV);
    thresholdMethodSelector->addItem(tr("Robust"), ROBUST);
    thresholdMethodSelector->addItem(tr("MultiScale"), MSCALE);

    colorFilterSelector->addItem(tr("Wiener denoiser"), F_WIENER);
    colorFilterSelector->addItem(tr("KNN denoiser"), F_KNND);
    colorFilterSelector->addItem(tr("Despeckle"), F_DESPECKLE);
    colorFilterSelector->addItem(tr("Blur/Sharpen"), F_BLUR);
    colorFilterSelector->addItem(tr("Screen"), F_SCREEN);
    colorFilterSelector->addItem(tr("Engraving"), F_ENGRAVING);
    colorFilterSelector->addItem(tr("UnPaper"), F_UNPAPER);

    kmeansColorSpaceSelector->addItem(tr("HSV"), HSV);
    kmeansColorSpaceSelector->addItem(tr("HSL"), HSL);
    kmeansColorSpaceSelector->addItem(tr("YCbCr"), YCBCR);

    thresholdSlider->setToolTip(QString::number(thresholdSlider->value()));

    updateColorsDisplay();
    updateScaleDisplay();

    connect(
        scalingPanelEmpty, SIGNAL(clicked(bool)),
        this, SLOT(scalingPanelToggled(bool))
    );
    connect(
        scalingPanel, SIGNAL(clicked(bool)),
        this, SLOT(scalingPanelToggled(bool))
    );
    connect(
        scale1xBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) scaleChanged(1.0);
    }
    );
    connect(
        scale15xBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) scaleChanged(1.5);
    }
    );
    connect(
        scale2xBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) scaleChanged(2.0);
    }
    );
    connect(
        scale3xBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) scaleChanged(3.0);
    }
    );
    connect(
        scale4xBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) scaleChanged(4.0);
    }
    );
    connect(
        scaleFactor, SIGNAL(valueChanged(double)),
        this, SLOT(scaleFactorChanged(double))
    );
    connect(
        filtersPanelEmpty, SIGNAL(clicked(bool)),
        this, SLOT(filtersPanelToggled(bool))
    );
    connect(
        filtersPanel, SIGNAL(clicked(bool)),
        this, SLOT(filtersPanelToggled(bool))
    );
    connect(
        curveCoef, SIGNAL(valueChanged(double)),
        this, SLOT(curveCoefChanged(double))
    );
    connect(
        sqrCoef, SIGNAL(valueChanged(double)),
        this, SLOT(sqrCoefChanged(double))
    );
    connect(
        colorFilterSelector, SIGNAL(currentIndexChanged(int)),
        this, SLOT(colorFilterChanged(int))
    );
    connect(
        colorFilterSize, SIGNAL(valueChanged(int)),
        this, SLOT(colorFilterSizeChanged(int))
    );
    connect(
        colorFilterCoef, SIGNAL(valueChanged(double)),
        this, SLOT(colorFilterCoefChanged(double))
    );
    connect(
        normalizeCoef, SIGNAL(valueChanged(double)),
        this, SLOT(normalizeCoefChanged(double))
    );
    connect(
        whiteMarginsCB, SIGNAL(clicked(bool)),
        this, SLOT(whiteMarginsToggled(bool))
    );
    connect(
        applyColorsFiltersButton, SIGNAL(clicked()),
        this, SLOT(applyColorsFiltersButtonClicked())
    );
    connect(
        modePanelEmpty, SIGNAL(clicked(bool)),
        this, SLOT(modePanelToggled(bool))
    );
    connect(
        modePanel, SIGNAL(clicked(bool)),
        this, SLOT(modePanelToggled(bool))
    );
    connect(
        colorModeSelector, SIGNAL(currentIndexChanged(int)),
        this, SLOT(colorModeChanged(int))
    );
    connect(
        grayScaleCB, SIGNAL(clicked(bool)),
        this, SLOT(grayScaleToggled(bool))
    );
    connect(
        thresholdMethodSelector, SIGNAL(currentIndexChanged(int)),
        this, SLOT(thresholdMethodChanged(int))
    );
    connect(
        morphologyCB, SIGNAL(clicked(bool)),
        this, SLOT(morphologyToggled(bool))
    );
    connect(
        negateCB, SIGNAL(clicked(bool)),
        this, SLOT(negateToggled(bool))
    );
    connect(
        dimmingColoredCoef, SIGNAL(valueChanged(double)),
        this, SLOT(dimmingColoredCoefChanged(double))
    );
    connect(
        lighterThresholdButton, SIGNAL(clicked(bool)),
        this, SLOT(setLighterThreshold())
    );
    connect(
        darkerThresholdButton, SIGNAL(clicked(bool)),
        this, SLOT(setDarkerThreshold())
    );
    connect(
        neutralThresholdBtn, SIGNAL(clicked()),
        this, SLOT(setNeutralThreshold())
    );
    connect(
        thresholdSlider, SIGNAL(valueChanged(int)),
        this, SLOT(bwThresholdChanged())
    );
    connect(
        thresholdSlider, SIGNAL(sliderReleased()),
        this, SLOT(bwThresholdChanged())
    );
    connect(
        thresholdWindowSize, SIGNAL(valueChanged(int)),
        this, SLOT(thresholdWindowSizeChanged(int))
    );
    connect(
        thresholdCoef, SIGNAL(valueChanged(double)),
        this, SLOT(thresholdCoefChanged(double))
    );
    connect(
        autoPictureCoef, SIGNAL(valueChanged(double)),
        this, SLOT(autoPictureCoefChanged(double))
    );
    connect(
        autoPictureOffCB, SIGNAL(clicked(bool)),
        this, SLOT(autoPictureOffToggled(bool))
    );
    connect(
        applyColorsModeButton, SIGNAL(clicked()),
        this, SLOT(applyColorsModeButtonClicked())
    );
    connect(
        kmeansPanelEmpty, SIGNAL(clicked(bool)),
        this, SLOT(kmeansPanelToggled(bool))
    );
    connect(
        kmeansPanel, SIGNAL(clicked(bool)),
        this, SLOT(kmeansPanelToggled(bool))
    );
    connect(
        kmeansCount, SIGNAL(valueChanged(int)),
        this, SLOT(kmeansCountChanged(int))
    );
    connect(
        kmeansCount, SIGNAL(valueChanged(int)),
        this, SLOT(kmeansCountChanged(int))
    );
    connect(
        kmeansValueStart, SIGNAL(valueChanged(int)),
        this, SLOT(kmeansValueStartChanged(int))
    );
    connect(
        kmeansSat, SIGNAL(valueChanged(double)),
        this, SLOT(kmeansSatChanged(double))
    );
    connect(
        kmeansNorm, SIGNAL(valueChanged(double)),
        this, SLOT(kmeansNormChanged(double))
    );
    connect(
        kmeansBG, SIGNAL(valueChanged(double)),
        this, SLOT(kmeansBGChanged(double))
    );
    connect(
        coloredMaskCoef, SIGNAL(valueChanged(double)),
        this, SLOT(coloredMaskCoefChanged(double))
    );
    connect(
        kmeansColorSpaceSelector, SIGNAL(currentIndexChanged(int)),
        this, SLOT(kmeansColorSpaceChanged(int))
    );
    connect(
        kmeansMorphology, SIGNAL(valueChanged(int)),
        this, SLOT(kmeansMorphologyChanged(int))
    );
    connect(
        applyKmeansButton, SIGNAL(clicked()),
        this, SLOT(applyKmeansButtonClicked())
    );
    connect(
        despecklePanelEmpty, SIGNAL(clicked(bool)),
        this, SLOT(despecklePanelToggled(bool))
    );
    connect(
        despecklePanel, SIGNAL(clicked(bool)),
        this, SLOT(despecklePanelToggled(bool))
    );
    connect(
        despeckleOffBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) despeckleLevelSelected(DESPECKLE_OFF);
    }
    );
    connect(
        despeckleCautiousBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) despeckleLevelSelected(DESPECKLE_CAUTIOUS);
    }
    );
    connect(
        despeckleNormalBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) despeckleLevelSelected(DESPECKLE_NORMAL);
    }
    );
    connect(
        despeckleAggressiveBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) despeckleLevelSelected(DESPECKLE_AGGRESSIVE);
    }
    );
    connect(
        despeckleCustomBtn, &QAbstractButton::toggled,
        [this](bool checked)
    {
        if (checked) despeckleLevelSelected(DESPECKLE_CUSTOM);
    }
    );
    connect(
        despeckleFactor, SIGNAL(valueChanged(double)),
        this, SLOT(despeckleFactorChanged(double))
    );
    connect(
        applyDespeckleButton, SIGNAL(clicked()),
        this, SLOT(applyDespeckleButtonClicked())
    );

    thresholdSlider->setMinimum(-50);
    thresholdSlider->setMaximum(50);
    thresholLabel->setText(QString::number(thresholdSlider->value()));
}

OptionsWidget::~OptionsWidget()
{
}

void
OptionsWidget::preUpdateUI(PageId const& page_id)
{
    Params const params(m_ptrSettings->getParams(page_id));
    m_pageId = page_id;
    m_colorParams = params.colorParams();
    m_despeckleLevel = params.despeckleLevel();
    m_despeckleFactor = params.despeckleFactor();
    m_thisPageOutputSize.reset();
    updateColorsDisplay();
    updateScaleDisplay();
}

void
OptionsWidget::postUpdateUI(QSize const& output_size)
{
    m_thisPageOutputSize = output_size;
    updateScaleDisplay();
}

void
OptionsWidget::tabChanged(ImageViewTab const tab)
{
    m_lastTab = tab;
    updateColorsDisplay();
    reloadIfNecessary();
}

void
OptionsWidget::scalingPanelToggled(bool const checked)
{
    scalingPanelEmpty->setVisible(!checked);
    scalingPanelEmpty->setChecked(checked);
    scalingPanel->setVisible(checked);
    scalingPanel->setChecked(checked);
}

void
OptionsWidget::scaleChanged(double const scale)
{
    if (m_ignoreScaleChanges)
    {
        return;
    }

    m_ptrSettings->setScalingFactor(scale);
    updateScaleDisplay();

    emit invalidateAllThumbnails();
    emit reloadRequested();
}

void
OptionsWidget::scaleFactorChanged(double const value)
{
    m_ptrSettings->setScalingFactor(value);
    updateScaleDisplay();

    emit invalidateAllThumbnails();
    emit reloadRequested();
}

void
OptionsWidget::filtersPanelToggled(bool const checked)
{
    filtersPanelEmpty->setVisible(!checked);
    filtersPanelEmpty->setChecked(checked);
    filtersPanel->setVisible(checked);
    filtersPanel->setChecked(checked);
}

void
OptionsWidget::curveCoefChanged(double value)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    color_options.setCurveCoef(value);
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::sqrCoefChanged(double value)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    color_options.setSqrCoef(value);
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::colorFilterGet()
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    colorFilterSize->blockSignals(true);
    colorFilterCoef->blockSignals(true);
    switch (m_colorFilterCurrent)
    {
    case F_WIENER:
        colorFilterSize->setValue(color_options.wienerSize());
        colorFilterCoef->setValue(color_options.wienerCoef());
        break;
    case F_KNND:
        colorFilterSize->setValue(color_options.knndRadius());
        colorFilterCoef->setValue(color_options.knndCoef());
        break;
    case F_DESPECKLE:
        colorFilterSize->setValue(color_options.cdespeckleRadius());
        colorFilterCoef->setValue(color_options.cdespeckleCoef());
        break;
    case F_BLUR:
        colorFilterSize->setValue(color_options.blurSize());
        colorFilterCoef->setValue(color_options.blurCoef());
        break;
    case F_SCREEN:
        colorFilterSize->setValue(color_options.screenSize());
        colorFilterCoef->setValue(color_options.screenCoef());
        break;
    case F_ENGRAVING:
        colorFilterSize->setValue(color_options.gravureSize());
        colorFilterCoef->setValue(color_options.gravureCoef());
        break;
    case F_UNPAPER:
        colorFilterSize->setValue(color_options.unPaperIters());
        colorFilterCoef->setValue(color_options.unPaperCoef());
        break;
    }
    colorFilterSize->blockSignals(false);
    colorFilterCoef->blockSignals(false);
}

void
OptionsWidget::colorFilterChanged(int const idx)
{
    m_colorFilterCurrent = colorFilterSelector->itemData(idx).toInt();
    colorFilterGet();
}

void
OptionsWidget::colorFilterSizeChanged(int value)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    switch (m_colorFilterCurrent)
    {
    case F_WIENER:
        color_options.setWienerSize(value);
        break;
    case F_KNND:
        color_options.setKnndRadius(value);
        break;
    case F_DESPECKLE:
        color_options.setCdespeckleRadius(value);
        break;
    case F_BLUR:
        color_options.setBlurSize(value);
        break;
    case F_SCREEN:
        color_options.setScreenSize(value);
        break;
    case F_ENGRAVING:
        color_options.setGravureSize(value);
        break;
    case F_UNPAPER:
        color_options.setUnPaperIters(value);
        break;
    }
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::colorFilterCoefChanged(double value)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    switch (m_colorFilterCurrent)
    {
    case F_WIENER:
        color_options.setWienerCoef(value);
        break;
    case F_KNND:
        color_options.setKnndCoef(value);
        break;
    case F_DESPECKLE:
        color_options.setCdespeckleCoef(value);
        break;
    case F_BLUR:
        color_options.setBlurCoef(value);
        break;
    case F_SCREEN:
        color_options.setScreenCoef(value);
        break;
    case F_ENGRAVING:
        color_options.setGravureCoef(value);
        break;
    case F_UNPAPER:
        color_options.setUnPaperCoef(value);
        break;
    }
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::normalizeCoefChanged(double value)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    color_options.setNormalizeCoef(value);
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::whiteMarginsToggled(bool const checked)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    color_options.setWhiteMargins(checked);
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::applyColorsFiltersButtonClicked()
{
    ApplyColorsDialog* dialog = new ApplyColorsDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyColorsFiltersConfirmed(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::applyColorsFiltersConfirmed(std::set<PageId> const& pages)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    BOOST_FOREACH(PageId const& page_id, pages)
    {
//        m_ptrSettings->setColorParams(page_id, m_colorParams);
        m_ptrSettings->setColorGrayscaleOptions(page_id, color_options);
        emit invalidateThumbnail(page_id);
    }

    if (pages.find(m_pageId) != pages.end())
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::modePanelToggled(bool const checked)
{
    modePanelEmpty->setVisible(!checked);
    modePanelEmpty->setChecked(checked);
    modePanel->setVisible(checked);
    modePanel->setChecked(checked);
}

void
OptionsWidget::colorModeChanged(int const idx)
{
    int const mode = colorModeSelector->itemData(idx).toInt();
    m_colorParams.setColorMode((ColorParams::ColorMode)mode);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    updateColorsDisplay();
    emit reloadRequested();
}

void
OptionsWidget::grayScaleToggled(bool const checked)
{
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    color_options.setflgGrayScale(checked);
    m_colorParams.setColorGrayscaleOptions(color_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::thresholdMethodChanged(int idx)
{
    ThresholdFilter const method = (ThresholdFilter) thresholdMethodSelector->itemData(idx).toInt();
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setThresholdMethod(method);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
//    updateColorsDisplay();
    emit reloadRequested();
}

void
OptionsWidget::morphologyToggled(bool const checked)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setMorphology(checked);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::negateToggled(bool const checked)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setNegate(checked);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::setLighterThreshold()
{
    thresholdSlider->setValue(thresholdSlider->value() - 1);
}

void
OptionsWidget::setDarkerThreshold()
{
    thresholdSlider->setValue(thresholdSlider->value() + 1);
}

void
OptionsWidget::setNeutralThreshold()
{
    thresholdSlider->setValue(0);
}

void
OptionsWidget::bwThresholdChanged()
{
    int const value = thresholdSlider->value();
    QString const tooltip_text(QString::number(value));
    thresholdSlider->setToolTip(tooltip_text);

    thresholLabel->setText(QString::number(value));

    if (m_ignoreThresholdChanges)
    {
        return;
    }

    // Show the tooltip immediately.
    QPoint const center(thresholdSlider->rect().center());
    QPoint tooltip_pos(thresholdSlider->mapFromGlobal(QCursor::pos()));
    tooltip_pos.setY(center.y());
    tooltip_pos.setX(qBound(0, tooltip_pos.x(), thresholdSlider->width()));
    tooltip_pos = thresholdSlider->mapToGlobal(tooltip_pos);
    QToolTip::showText(tooltip_pos, tooltip_text, thresholdSlider);

    if (thresholdSlider->isSliderDown())
    {
        // Wait for it to be released.
        // We could have just disabled tracking, but in that case we wouldn't
        // be able to show tooltips with a precise value.
        return;
    }

    BlackWhiteOptions opt(m_colorParams.blackWhiteOptions());
    if (opt.thresholdAdjustment() == value)
    {
        // Didn't change.
        return;
    }

    opt.setThresholdAdjustment(value);
    m_colorParams.setBlackWhiteOptions(opt);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();

    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::dimmingColoredCoefChanged(double value)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setDimmingColoredCoef(value);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::thresholdWindowSizeChanged(int value)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setThresholdWindowSize(value);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::thresholdCoefChanged(double value)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setThresholdCoef(value);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::autoPictureCoefChanged(double value)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setAutoPictureCoef(value);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::autoPictureOffToggled(bool const checked)
{
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    black_white_options.setAutoPictureOff(checked);
    m_colorParams.setBlackWhiteOptions(black_white_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::applyColorsModeButtonClicked()
{
    ApplyColorsDialog* dialog = new ApplyColorsDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyColorsModeConfirmed(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::applyColorsModeConfirmed(std::set<PageId> const& pages)
{
    ColorParams::ColorMode mode(m_colorParams.colorMode());
    BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
    BOOST_FOREACH(PageId const& page_id, pages)
    {
//        m_ptrSettings->setColorParams(page_id, m_colorParams);
        m_ptrSettings->setColorMode(page_id, mode);
        m_ptrSettings->setBlackWhiteOptions(page_id, black_white_options);
        emit invalidateThumbnail(page_id);
    }

    if (pages.find(m_pageId) != pages.end())
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::kmeansPanelToggled(bool const checked)
{
    kmeansPanelEmpty->setVisible(!checked);
    kmeansPanelEmpty->setChecked(checked);
    kmeansPanel->setVisible(checked);
    kmeansPanel->setChecked(checked);
}

void
OptionsWidget::kmeansCountChanged(int value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansCount(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansValueStartChanged(int value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansValueStart(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansMorphologyChanged(int value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansMorphology(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansSatChanged(double value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansSat(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansNormChanged(double value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansNorm(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansBGChanged(double value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansBG(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::coloredMaskCoefChanged(double value)
{
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setColoredMaskCoef(value);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::kmeansColorSpaceChanged(int idx)
{
    KmeansColorSpace const color_space = (KmeansColorSpace) kmeansColorSpaceSelector->itemData(idx).toInt();
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    black_kmeans_options.setKmeansColorSpace(color_space);
    m_colorParams.setBlackKmeansOptions(black_kmeans_options);
    m_ptrSettings->setColorParams(m_pageId, m_colorParams);
    emit reloadRequested();
}

void
OptionsWidget::applyKmeansButtonClicked()
{
    ApplyColorsDialog* dialog = new ApplyColorsDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyKmeansConfirmed(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::applyKmeansConfirmed(std::set<PageId> const& pages)
{
    ColorParams::ColorMode mode(m_colorParams.colorMode());
    BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
    BOOST_FOREACH(PageId const& page_id, pages)
    {
//        m_ptrSettings->setColorParams(page_id, m_colorParams);
        m_ptrSettings->setBlackKmeansOptions(page_id, black_kmeans_options);
        emit invalidateThumbnail(page_id);
    }

    if (pages.find(m_pageId) != pages.end())
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::despecklePanelToggled(bool const checked)
{
    despecklePanelEmpty->setVisible(!checked);
    despecklePanelEmpty->setChecked(checked);
    despecklePanel->setVisible(checked);
    despecklePanel->setChecked(checked);
}

void
OptionsWidget::despeckleLevelSelected(DespeckleLevel const level)
{
    if (m_ignoreDespeckleLevelChanges)
    {
        return;
    }

    double despeckle_factor_bak = m_despeckleFactor;
    if (level == DESPECKLE_CUSTOM)
    {
        despeckleFactor->setEnabled( true );
    }
    else
    {
        m_despeckleFactor = despeckleLevelToFactor(level);
        despeckleFactor->setEnabled( false );
    }
    m_despeckleLevel = level;
    if (m_despeckleFactor != despeckle_factor_bak)
    {
        despeckleFactor->setValue(m_despeckleFactor);
    }

    m_ptrSettings->setDespeckleLevel(m_pageId, level);
    m_ptrSettings->setDespeckleFactor(m_pageId, m_despeckleFactor);
    emit reloadRequested();
}

void
OptionsWidget::despeckleFactorChanged(double value)
{
    m_despeckleFactor = value;
    m_ptrSettings->setDespeckleFactor(m_pageId, m_despeckleFactor);

    bool handled = false;
    emit despeckleLevelChanged(m_despeckleFactor, &handled);

    if (handled)
    {
        // This means we are on the "Despeckling" tab.
        emit invalidateThumbnail(m_pageId);
    }
    emit reloadRequested();
}

void
OptionsWidget::applyDespeckleButtonClicked()
{
    ApplyColorsDialog* dialog = new ApplyColorsDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Apply Despeckling Level"));
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyDespeckleConfirmed(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::applyDespeckleConfirmed(std::set<PageId> const& pages)
{
    BOOST_FOREACH(PageId const& page_id, pages)
    {
        m_ptrSettings->setDespeckleLevel(page_id, m_despeckleLevel);
        m_ptrSettings->setDespeckleFactor(page_id, m_despeckleFactor);
        emit invalidateThumbnail(page_id);
    }

    if (pages.find(m_pageId) != pages.end())
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::reloadIfNecessary()
{
    ZoneSet saved_picture_zones;
    ZoneSet saved_fill_zones;
    dewarping::DistortionModel saved_distortion_model;
    DespeckleLevel saved_despeckle_level = DESPECKLE_CAUTIOUS;

    std::unique_ptr<OutputParams> output_params(m_ptrSettings->getOutputParams(m_pageId));
    if (output_params.get())
    {
        saved_picture_zones = output_params->pictureZones();
        saved_fill_zones = output_params->fillZones();
        saved_despeckle_level = output_params->outputImageParams().despeckleLevel();
    }

    if (!PictureZoneComparator::equal(saved_picture_zones, m_ptrSettings->pictureZonesForPage(m_pageId)))
    {
        emit reloadRequested();
        return;
    }
    else if (!FillZoneComparator::equal(saved_fill_zones, m_ptrSettings->fillZonesForPage(m_pageId)))
    {
        emit reloadRequested();
        return;
    }

    Params const params(m_ptrSettings->getParams(m_pageId));

    if (saved_despeckle_level != params.despeckleLevel())
    {
        emit reloadRequested();
        return;
    }
}

void
OptionsWidget::updateColorsDisplay()
{
    colorModeSelector->blockSignals(true);

    ColorParams::ColorMode const color_mode = m_colorParams.colorMode();
    int const color_mode_idx = colorModeSelector->findData(color_mode);
    colorModeSelector->setCurrentIndex(color_mode_idx);

    bool color_grayscale_options_visible = false;
    bool bw_options_visible = false;
    bool mixed_options_visible = false;
    switch (color_mode)
    {
    case ColorParams::BLACK_AND_WHITE:
        bw_options_visible = true;
        break;
    case ColorParams::COLOR_GRAYSCALE:
        color_grayscale_options_visible = true;
        break;
    case ColorParams::MIXED:
        bw_options_visible = true;
        mixed_options_visible = true;
        break;
    }

    colorMarginOptions->setVisible(color_grayscale_options_visible);
    mixedOptions->setVisible(mixed_options_visible);
    ColorGrayscaleOptions color_options(m_colorParams.colorGrayscaleOptions());
    curveCoef->setValue(color_options.curveCoef());
    sqrCoef->setValue(color_options.sqrCoef());
    colorFilterGet();
    normalizeCoef->setValue(color_options.normalizeCoef());
    if (color_grayscale_options_visible)
    {
        whiteMarginsCB->setChecked(color_options.whiteMargins());
    }
    grayScaleCB->setChecked(color_options.getflgGrayScale());

    bwOptions->setVisible(bw_options_visible);
    if (bw_options_visible)
    {
        BlackWhiteOptions black_white_options(m_colorParams.blackWhiteOptions());
        BlackKmeansOptions black_kmeans_options(m_colorParams.blackKmeansOptions());
        thresholdMethodSelector->setCurrentIndex((int) black_white_options.thresholdMethod());
        morphologyCB->setChecked(black_white_options.morphology());
        negateCB->setChecked(black_white_options.negate());
        dimmingColoredCoef->setValue(black_white_options.dimmingColoredCoef());
        thresholdSlider->setValue(black_white_options.thresholdAdjustment());
        thresholdWindowSize->setValue(black_white_options.thresholdWindowSize());
        thresholdCoef->setValue(black_white_options.thresholdCoef());

        if ((black_white_options.thresholdMethod() == OTSU) || (black_white_options.thresholdMethod() == MEANDELTA) || (black_white_options.thresholdMethod() == DOTS8))
        {
            thresholdWindowSize->setEnabled( false );
            thresholdCoef->setEnabled( false );
        }
        else
        {
            thresholdWindowSize->setEnabled( true );
            thresholdCoef->setEnabled( true );
        }
        if (mixed_options_visible)
        {
            autoPictureCoef->setValue(black_white_options.autoPictureCoef());
            autoPictureOffCB->setChecked(black_white_options.autoPictureOff());
        }

        kmeansPanelToggled(kmeansPanelEmpty->isChecked());
        kmeansCount->setValue(black_kmeans_options.kmeansCount());
        kmeansValueStart->setValue(black_kmeans_options.kmeansValueStart());
        kmeansSat->setValue(black_kmeans_options.kmeansSat());
        kmeansNorm->setValue(black_kmeans_options.kmeansNorm());
        kmeansBG->setValue(black_kmeans_options.kmeansBG());
        coloredMaskCoef->setValue(black_kmeans_options.coloredMaskCoef());
        kmeansColorSpaceSelector->setCurrentIndex((int) black_kmeans_options.kmeansColorSpace());
        kmeansMorphology->setValue(black_kmeans_options.kmeansMorphology());

        coloredMaskCoef->setEnabled( false );
        if (black_kmeans_options.kmeansCount() > 0)
        {
            kmeansValueStart->setEnabled( true );
            kmeansSat->setEnabled( true );
            kmeansNorm->setEnabled( true );
            kmeansBG->setEnabled( true );
            if (black_white_options.dimmingColoredCoef() > 0.0)
            {
                coloredMaskCoef->setEnabled( true );
            }
            kmeansColorSpaceSelector->setEnabled( true );
            kmeansMorphology->setEnabled( true );
        }
        else
        {
            kmeansValueStart->setEnabled( false );
            kmeansSat->setEnabled( false );
            kmeansNorm->setEnabled( false );
            kmeansBG->setEnabled( false );
            kmeansColorSpaceSelector->setEnabled( false );
            kmeansMorphology->setEnabled( false );
        }

        despecklePanelToggled(despecklePanelEmpty->isChecked());
        ScopedIncDec<int> const despeckle_guard(m_ignoreDespeckleLevelChanges);

        switch (m_despeckleLevel)
        {
        case DESPECKLE_OFF:
            despeckleOffBtn->setChecked(true);
            break;
        case DESPECKLE_CAUTIOUS:
            despeckleCautiousBtn->setChecked(true);
            break;
        case DESPECKLE_NORMAL:
            despeckleNormalBtn->setChecked(true);
            break;
        case DESPECKLE_AGGRESSIVE:
            despeckleAggressiveBtn->setChecked(true);
            break;
        case DESPECKLE_CUSTOM:
            despeckleCustomBtn->setChecked(true);
            break;
        }
        if (m_despeckleLevel == DESPECKLE_CUSTOM)
        {
            despeckleFactor->setEnabled( true );
        }
        else
        {
            m_despeckleFactor = despeckleLevelToFactor(m_despeckleLevel);
            despeckleFactor->setEnabled( false );
        }
        despeckleFactor->setValue(m_despeckleFactor);

        ScopedIncDec<int> const threshold_guard(m_ignoreThresholdChanges);
    }
    else
    {
        kmeansPanelEmpty->setVisible(bw_options_visible);
        kmeansPanel->setVisible(bw_options_visible);
        despecklePanelEmpty->setVisible(bw_options_visible);
        despecklePanel->setVisible(bw_options_visible);
    }

    colorModeSelector->blockSignals(false);
}

void
OptionsWidget::updateScaleDisplay()
{
    ScopedIncDec<int> const guard(m_ignoreScaleChanges);

    double const scale = m_ptrSettings->scalingFactor();
    if (scale < 1.25)
    {
        scale1xBtn->setChecked(true);
    }
    else if (scale < 1.75)
    {
        scale15xBtn->setChecked(true);
    }
    else if (scale < 2.5)
    {
        scale2xBtn->setChecked(true);
    }
    else if (scale < 3.5)
    {
        scale3xBtn->setChecked(true);
    }
    else
    {
        scale4xBtn->setChecked(true);
    }
    scaleFactor->setValue(scale);

    if (m_thisPageOutputSize)
    {
        int const width = m_thisPageOutputSize->width();
        int const height = m_thisPageOutputSize->height();
        scaleLabel->setText(tr("This page: %1 x %2 px").arg(width).arg(height));
    }
    else
    {
        scaleLabel->setText(QString());
    }
}

} // namespace output
