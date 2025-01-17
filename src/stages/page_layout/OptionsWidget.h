/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef PAGE_LAYOUT_OPTIONSWIDGET_H_
#define PAGE_LAYOUT_OPTIONSWIDGET_H_

#include <memory>
#include <map>
#include <set>
#include <QIcon>
#include "ui_PageLayoutOptionsWidget.h"
#include "FilterOptionsWidget.h"
#include "PageSelectionAccessor.h"
#include "IntrusivePtr.h"
#include "RelativeMargins.h"
#include "MatchSizeMode.h"
#include "Alignment.h"
#include "Framings.h"
#include "PageId.h"

class QToolButton;
class ProjectPages;

namespace page_layout
{

class Settings;

class OptionsWidget :
    public FilterOptionsWidget,
    public Ui::PageLayoutOptionsWidget
{
    Q_OBJECT
public:
    OptionsWidget(
        IntrusivePtr<Settings> const& settings,
        PageSelectionAccessor const& page_selection_accessor);

    virtual ~OptionsWidget();

    bool disconnectAll(void);

    void preUpdateUI(
        PageId const& page_id,
        RelativeMargins const& margins,
        MatchSizeMode const& match_size_mode,
        Alignment const& alignment,
        Framings const& framings
    );

    void postUpdateUI(bool have_content_box);

    bool leftRightLinked() const
    {
        return m_leftRightLinked;
    }

    bool topBottomLinked() const
    {
        return m_topBottomLinked;
    }

    RelativeMargins const& margins() const
    {
        return m_margins;
    }

    MatchSizeMode const& matchSizeMode() const
    {
        return m_matchSizeMode;
    }

    Alignment const& alignment() const
    {
        return m_alignment;
    }

    Framings const& framings() const
    {
        return m_framings;
    }
signals:
    void leftRightLinkToggled(bool linked);

    void topBottomLinkToggled(bool linked);

    void matchSizeModeChanged(MatchSizeMode const& match_size_mode);

    void alignmentChanged(Alignment const& alignment);

    void framingsChanged(Framings const& framings);

    void marginsSetLocally(RelativeMargins const& margins);

    void aggregateHardSizeChanged();
public slots:
    void marginsSetExternally(RelativeMargins const& margins);
private slots:
    void extraWMarginChanged(double val);

    void extraHMarginChanged(double val);

    void horMarginsChanged(double val);

    void vertMarginsChanged(double val);

    void topBottomLinkClicked();

    void leftRightLinkClicked();

    void matchSizeDisabledToggled(bool selected);

    void matchSizeMarginsToggled(bool selected);

    void matchSizeScaleToggled(bool selected);

    void matchSizeAffineToggled(bool selected);

    void alignmentButtonClicked();

    void showApplyFramingsDialog();

    void showApplyMarginsDialog();

    void showApplyAlignmentDialog();

    void applyFramings(std::set<PageId> const& pages);

    void applyMargins(std::set<PageId> const& pages);

    void applyAlignment(std::set<PageId> const& pages);
private:
    typedef std::map<QToolButton*, Alignment> AlignmentByButton;

    void updateMarginsDisplay();

    void updateLinkDisplay(QToolButton* button, bool linked);

    void updateFramingsDisplay();

    void updateSizeDisplay();

    void enableDisableAlignmentButtons();

    IntrusivePtr<Settings> m_ptrSettings;
    PageSelectionAccessor m_pageSelectionAccessor;
    QIcon m_chainIcon;
    QIcon m_brokenChainIcon;
    AlignmentByButton m_alignmentByButton;
    PageId m_pageId;
    RelativeMargins m_margins;
    MatchSizeMode m_matchSizeMode;
    Alignment m_alignment;
    Framings m_framings;
    int m_ignoreMarginChanges;
    int m_ignoreMatchSizeModeChanges;
    bool m_leftRightLinked;
    bool m_topBottomLinked;
};

} // namespace page_layout

#endif
