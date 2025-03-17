/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef OUTPUT_OPTIONSWIDGET_H_
#define OUTPUT_OPTIONSWIDGET_H_

#include <set>
#include <boost/optional.hpp>
#include <QSize>
#include "ui_OutputOptionsWidget.h"
#include "FilterOptionsWidget.h"
#include "IntrusivePtr.h"
#include "PageId.h"
#include "PageSelectionAccessor.h"
#include "ColorParams.h"
#include "DespeckleLevel.h"
#include "ImageViewTab.h"

namespace output
{

enum FilterColorAdditional { F_AUTOLEVEL, F_BALANCE, F_OVERBLUR, F_RETINEX, F_SUBTRACTBG, F_EQUALIZE, F_WIENER, F_KNND, F_DESPECKLE, F_SIGMA, F_BLUR, F_SCREEN, F_EDGEDIV, F_ROBUST, F_COMIX, F_ENGRAVING, F_DOTS8, F_UNPAPER };

class Settings;

class OptionsWidget
    : public FilterOptionsWidget, private Ui::OutputOptionsWidget
{
    Q_OBJECT
public:
    OptionsWidget(IntrusivePtr<Settings> const& settings,
                  PageSelectionAccessor const& page_selection_accessor);

    virtual ~OptionsWidget();

    bool disconnectAll(void);

    void preUpdateUI(PageId const& page_id);

    void postUpdateUI(QSize const& output_size);

    ImageViewTab lastTab() const
    {
        return m_lastTab;
    }
signals:
    void despeckleLevelChanged(double factor, bool* handled);
public slots:
    void tabChanged(ImageViewTab tab);
private slots:
    void scalingPanelToggled(bool checked);

    void scaleFactorChanged(double value);

    void filtersPanelToggled(bool checked);

    void curveCoefChanged(double value);

    void sqrCoefChanged(double value);

    void colorFilterGet();

    void colorFilterChanged(int idx);

    void colorFilterSizeChanged(int value);

    void colorFilterCoefChanged(double value);

    void normalizeCoefChanged(double value);

    void whiteMarginsToggled(bool checked);

    void applyColorsFiltersButtonClicked();

    void applyColorsFiltersConfirmed(std::set<PageId> const& pages);

    void modePanelToggled(bool checked);

    void grayScaleToggled(bool checked);

    void colorModeChanged(int idx);

    void setLighterThreshold();

    void setDarkerThreshold();

    void setNeutralThreshold();

    void thresholdMethodChanged(int idx);

    void morphologyToggled(bool checked);

    void negateToggled(bool checked);

    void dimmingColoredCoefChanged(double value);

    void bwThresholdChanged();

    void thresholdBoundLowerChanged(int value);

    void thresholdBoundUpperChanged(int value);

    void thresholdRadiusChanged(int value);

    void thresholdCoefChanged(double value);

    void autoPictureCoefChanged(double value);

    void autoPictureOffToggled(bool checked);

    void pictureToDots8Toggled(bool checked);

    void applyColorsModeButtonClicked();

    void applyColorsModeConfirmed(std::set<PageId> const& pages);

    void kmeansPanelToggled(bool checked);

    void kmeansCountChanged(int value);

    void kmeansValueStartChanged(int value);

    void kmeansSatChanged(double value);

    void kmeansNormChanged(double value);

    void kmeansBGChanged(double value);

    void coloredMaskCoefChanged(double value);

    void kmeansColorSpaceChanged(int idx);

    void kmeansFindBlackToggled(bool checked);

    void kmeansFindWhiteToggled(bool checked);

    void applyKmeansButtonClicked();

    void applyKmeansConfirmed(std::set<PageId> const& pages);

    void despecklePanelToggled(bool checked);

    void despeckleFactorChanged(double value);

    void applyDespeckleButtonClicked();

    void applyDespeckleConfirmed(std::set<PageId> const& pages);

    void metricsPanelToggled(bool checked);

private:
    void despeckleLevelSelected(DespeckleLevel level);

    void scaleChanged(double scale);

    void reloadIfNecessary();

    void updateColorsDisplay();

    void updateScaleDisplay();

    void updateMetricsDisplay();

    IntrusivePtr<Settings> m_ptrSettings;
    PageSelectionAccessor m_pageSelectionAccessor;
    PageId m_pageId;
    ColorParams m_colorParams;
    DespeckleLevel m_despeckleLevel;
    double m_despeckleFactor;
    boost::optional<QSize> m_thisPageOutputSize;
    ImageViewTab m_lastTab;
    int m_ignoreThresholdChanges;
    int m_ignoreDespeckleLevelChanges;
    int m_ignoreScaleChanges;
    int m_colorFilterCurrent;
};

} // namespace output

#endif
