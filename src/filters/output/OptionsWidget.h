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

class Settings;

class OptionsWidget
    : public FilterOptionsWidget, private Ui::OutputOptionsWidget
{
    Q_OBJECT
public:
    OptionsWidget(IntrusivePtr<Settings> const& settings,
                  PageSelectionAccessor const& page_selection_accessor);

    virtual ~OptionsWidget();

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
    void wienerCoefChanged(double value);

    void wienerWindowSizeChanged(int value);

    void knndCoefChanged(double value);

    void knndRadiusChanged(int value);

    void blurCoefChanged(double value);

    void blurWindowSizeChanged(int value);

    void screenCoefChanged(double value);

    void screenWindowSizeChanged(int value);

    void curveCoefChanged(double value);

    void sqrCoefChanged(double value);

    void normalizeCoefChanged(double value);

    void whiteMarginsToggled(bool checked);

    void applyColorsFiltersButtonClicked();

    void applyColorsFiltersConfirmed(std::set<PageId> const& pages);

    void colorModeChanged(int idx);

    void setLighterThreshold();

    void setDarkerThreshold();

    void setNeutralThreshold();

    void thresholdMethodChanged(int idx);

    void dimmingColoredCoefChanged(double value);

    void thresholdWindowSizeChanged(int value);

    void thresholdCoefChanged(double value);

    void bwThresholdChanged();

    void applyColorsModeButtonClicked();

    void applyColorsModeConfirmed(std::set<PageId> const& pages);

    void despeckleFactorChanged(double value);

    void applyDespeckleButtonClicked();

    void applyDespeckleConfirmed(std::set<PageId> const& pages);
private:
    void despeckleLevelSelected(DespeckleLevel level);

    void scaleChanged(double scale);

    void reloadIfNecessary();

    void updateColorsDisplay();

    void updateScaleDisplay();

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
};

} // namespace output

#endif
