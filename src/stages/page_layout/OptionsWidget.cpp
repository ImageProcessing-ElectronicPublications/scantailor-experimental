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

#include <assert.h>
#include <boost/foreach.hpp>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QSettings>
#include <QVariant>
#include "OptionsWidget.h"
#include "Settings.h"
#include "ApplyDialog.h"
#include "../../Utils.h"
#include "ScopedIncDec.h"
#include "PageInfo.h"
#include "PageId.h"
#include "imageproc/Constants.h"

using namespace imageproc::constants;

namespace page_layout
{

OptionsWidget::OptionsWidget(
    IntrusivePtr<Settings> const& settings,
    PageSelectionAccessor const& page_selection_accessor)
    : m_ptrSettings(settings),
      m_pageSelectionAccessor(page_selection_accessor),
      m_ignoreMarginChanges(0),
      m_ignoreMatchSizeModeChanges(0),
      m_leftRightLinked(true),
      m_topBottomLinked(true)
{
    {
        QSettings app_settings;
        m_leftRightLinked = app_settings.value("margins/leftRightLinked", true).toBool();
        m_topBottomLinked = app_settings.value("margins/topBottomLinked", true).toBool();
    }

    m_chainIcon.addPixmap(
        QPixmap(QString::fromUtf8(":/icons/stock-vchain-24.png"))
    );
    m_brokenChainIcon.addPixmap(
        QPixmap(QString::fromUtf8(":/icons/stock-vchain-broken-24.png"))
    );

    setupUi(this);

    QIcon warningIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
    warningIconLabel->setPixmap(warningIcon.pixmap(48, 48));

    updateLinkDisplay(topBottomLink, m_topBottomLinked);
    updateLinkDisplay(leftRightLink, m_leftRightLinked);
    enableDisableAlignmentButtons();

    Utils::mapSetValue(
        m_alignmentByButton, alignTopLeftBtn,
        Alignment(Alignment::TOP, Alignment::LEFT)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignTopBtn,
        Alignment(Alignment::TOP, Alignment::HCENTER)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignTopRightBtn,
        Alignment(Alignment::TOP, Alignment::RIGHT)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignLeftBtn,
        Alignment(Alignment::VCENTER, Alignment::LEFT)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignCenterBtn,
        Alignment(Alignment::VCENTER, Alignment::HCENTER)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignRightBtn,
        Alignment(Alignment::VCENTER, Alignment::RIGHT)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignBottomLeftBtn,
        Alignment(Alignment::BOTTOM, Alignment::LEFT)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignBottomBtn,
        Alignment(Alignment::BOTTOM, Alignment::HCENTER)
    );
    Utils::mapSetValue(
        m_alignmentByButton, alignBottomRightBtn,
        Alignment(Alignment::BOTTOM, Alignment::RIGHT)
    );

    connect(
        extraWMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(extraWMarginChanged(double))
    );
    connect(
        extraHMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(extraHMarginChanged(double))
    );
    connect(
        applyFramingsBtn, SIGNAL(clicked()),
        this, SLOT(showApplyFramingsDialog())
    );
    connect(
        topMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(vertMarginsChanged(double))
    );
    connect(
        bottomMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(vertMarginsChanged(double))
    );
    connect(
        leftMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(horMarginsChanged(double))
    );
    connect(
        rightMarginSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(horMarginsChanged(double))
    );
    connect(
        topBottomLink, SIGNAL(clicked()),
        this, SLOT(topBottomLinkClicked())
    );
    connect(
        leftRightLink, SIGNAL(clicked()),
        this, SLOT(leftRightLinkClicked())
    );
    connect(
        applyMarginsBtn, SIGNAL(clicked()),
        this, SLOT(showApplyMarginsDialog())
    );
    connect(
        matchSizeDisabledRb, SIGNAL(toggled(bool)),
        this, SLOT(matchSizeDisabledToggled(bool))
    );
    connect(
        matchSizeMarginsRb, SIGNAL(toggled(bool)),
        this, SLOT(matchSizeMarginsToggled(bool))
    );
    connect(
        matchSizeScaleRb, SIGNAL(toggled(bool)),
        this, SLOT(matchSizeScaleToggled(bool))
    );
    connect(
        applyAlignmentBtn, SIGNAL(clicked()),
        this, SLOT(showApplyAlignmentDialog())
    );

    typedef AlignmentByButton::value_type KeyVal;
    BOOST_FOREACH (KeyVal const& kv, m_alignmentByButton)
    {
        connect(
            kv.first, SIGNAL(clicked()),
            this, SLOT(alignmentButtonClicked())
        );
    }
}

OptionsWidget::~OptionsWidget()
{
}

void
OptionsWidget::preUpdateUI(
    PageId const& page_id,
    RelativeMargins const& margins,
    MatchSizeMode const& match_size_mode,
    Alignment const& alignment,
    Framings const& framings)
{
    m_pageId = page_id;
    m_margins = margins;
    m_matchSizeMode = match_size_mode;
    m_alignment = alignment;
    m_framings = framings;

    for (auto const& kv : m_alignmentByButton)
    {
        if (kv.second == m_alignment)
        {
            kv.first->setChecked(true);
        }
    }

    updateMarginsDisplay();

    {
        ScopedIncDec<int> const guard(m_ignoreMatchSizeModeChanges);

        switch (match_size_mode.get())
        {
        case MatchSizeMode::DISABLED:
            matchSizeDisabledRb->setChecked(true);
            break;
        case MatchSizeMode::GROW_MARGINS:
            matchSizeMarginsRb->setChecked(true);
            break;
        case MatchSizeMode::SCALE:
            matchSizeScaleRb->setChecked(true);
            break;
        }
    }

    enableDisableAlignmentButtons();

    m_leftRightLinked = m_leftRightLinked && margins.horizontalMarginsApproxEqual();
    m_topBottomLinked = m_topBottomLinked && margins.verticalMarginsApproxEqual();
    updateLinkDisplay(topBottomLink, m_topBottomLinked);
    updateLinkDisplay(leftRightLink, m_leftRightLinked);

    marginsGroup->setVisible(true);
    alignmentGroup->setVisible(true);
    marginsGroup->setEnabled(false);
    alignmentGroup->setEnabled(false);
    missingContentBoxGroup->setVisible(false);
}

void
OptionsWidget::postUpdateUI(bool have_content_box)
{
    marginsGroup->setVisible(have_content_box);
    alignmentGroup->setVisible(have_content_box);

    marginsGroup->setEnabled(true);
    alignmentGroup->setEnabled(true);

    updateSizeDisplay();

    missingContentBoxGroup->setVisible(!have_content_box);
}

void
OptionsWidget::marginsSetExternally(RelativeMargins const& margins)
{
    m_margins = margins;

    updateMarginsDisplay();
    updateSizeDisplay();
}

void
OptionsWidget::extraWMarginChanged(double const val)
{
    if (m_ignoreMarginChanges)
    {
        return;
    }

    extraWMarginSpinBox->setValue(val);

    m_framings.setFramingWidth(extraWMarginSpinBox->value() / 100.0);

    updateMarginsDisplay();
    emit framingsChanged(m_framings);
    emit marginsSetLocally(m_margins);
    updateSizeDisplay();
}

void
OptionsWidget::extraHMarginChanged(double const val)
{
    if (m_ignoreMarginChanges)
    {
        return;
    }

    extraHMarginSpinBox->setValue(val);

    m_framings.setFramingHeight(extraHMarginSpinBox->value() / 100.0);

    updateMarginsDisplay();
    emit framingsChanged(m_framings);
    emit marginsSetLocally(m_margins);
    updateSizeDisplay();
}

void
OptionsWidget::horMarginsChanged(double const val)
{
    if (m_ignoreMarginChanges)
    {
        return;
    }

    if (m_leftRightLinked)
    {
        ScopedIncDec<int> const ingore_scope(m_ignoreMarginChanges);
        leftMarginSpinBox->setValue(val);
        rightMarginSpinBox->setValue(val);
    }

    m_margins.setLeft(leftMarginSpinBox->value() / 100.0);
    m_margins.setRight(rightMarginSpinBox->value() / 100.0);

    emit marginsSetLocally(m_margins);
    updateSizeDisplay();
}

void
OptionsWidget::vertMarginsChanged(double const val)
{

    if (m_topBottomLinked)
    {
        ScopedIncDec<int> const ingore_scope(m_ignoreMarginChanges);
        topMarginSpinBox->setValue(val);
        bottomMarginSpinBox->setValue(val);
    }

    m_margins.setTop(topMarginSpinBox->value() / 100.0);
    m_margins.setBottom(bottomMarginSpinBox->value() / 100.0);


    emit marginsSetLocally(m_margins);
    updateSizeDisplay();
}

void
OptionsWidget::topBottomLinkClicked()
{
    m_topBottomLinked = !m_topBottomLinked;
    QSettings().setValue("margins/topBottomLinked", m_topBottomLinked);
    updateLinkDisplay(topBottomLink, m_topBottomLinked);
    topBottomLinkToggled(m_topBottomLinked);
}

void
OptionsWidget::leftRightLinkClicked()
{
    m_leftRightLinked = !m_leftRightLinked;
    QSettings().setValue("margins/leftRightLinked", m_leftRightLinked);
    updateLinkDisplay(leftRightLink, m_leftRightLinked);
    leftRightLinkToggled(m_leftRightLinked);
}

void
OptionsWidget::matchSizeDisabledToggled(bool selected)
{
    if (!selected || m_ignoreMatchSizeModeChanges)
    {
        return;
    }

    m_matchSizeMode.set(MatchSizeMode::DISABLED);

    enableDisableAlignmentButtons();
    emit matchSizeModeChanged(m_matchSizeMode);
    updateSizeDisplay();
}

void
OptionsWidget::matchSizeMarginsToggled(bool selected)
{
    if (!selected || m_ignoreMatchSizeModeChanges)
    {
        return;
    }

    m_matchSizeMode.set(MatchSizeMode::GROW_MARGINS);

    enableDisableAlignmentButtons();
    emit matchSizeModeChanged(m_matchSizeMode);
    updateSizeDisplay();
}

void
OptionsWidget::matchSizeScaleToggled(bool selected)
{
    if (!selected || m_ignoreMatchSizeModeChanges)
    {
        return;
    }

    m_matchSizeMode.set(MatchSizeMode::SCALE);

    enableDisableAlignmentButtons();
    emit matchSizeModeChanged(m_matchSizeMode);
    updateSizeDisplay();
}

void
OptionsWidget::alignmentButtonClicked()
{
    QToolButton* const button = dynamic_cast<QToolButton*>(sender());
    assert(button);

    AlignmentByButton::iterator const it(m_alignmentByButton.find(button));
    assert(it != m_alignmentByButton.end());

    m_alignment = it->second;
    emit alignmentChanged(m_alignment);
}

void
OptionsWidget::showApplyFramingsDialog()
{
    ApplyDialog* dialog = new ApplyDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Apply Framings"));
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyFramings(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::showApplyMarginsDialog()
{
    ApplyDialog* dialog = new ApplyDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Apply Margins"));
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyMargins(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::showApplyAlignmentDialog()
{
    ApplyDialog* dialog = new ApplyDialog(
        this, m_pageId, m_pageSelectionAccessor
    );
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Apply Alignment"));
    connect(
        dialog, SIGNAL(accepted(std::set<PageId> const&)),
        this, SLOT(applyAlignment(std::set<PageId> const&))
    );
    dialog->show();
}

void
OptionsWidget::applyFramings(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    for (PageId const& page_id : pages)
    {
        m_ptrSettings->setPageFramings(page_id, m_framings);
    }

    emit invalidateAllThumbnails();
}

void
OptionsWidget::applyMargins(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    BOOST_FOREACH(PageId const& page_id, pages)
    {
        m_ptrSettings->setHardMargins(page_id, m_margins);
    }

    emit aggregateHardSizeChanged();
    emit invalidateAllThumbnails();
}

void
OptionsWidget::applyAlignment(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    for (PageId const& page_id : pages)
    {
        m_ptrSettings->setPageAlignment(page_id, m_alignment);
        m_ptrSettings->setMatchSizeMode(page_id, m_matchSizeMode);
    }

    emit invalidateAllThumbnails();
}

void
OptionsWidget::updateMarginsDisplay()
{
    ScopedIncDec<int> const ignore_scope(m_ignoreMarginChanges);

    extraWMarginSpinBox->setValue(m_framings.getFramingWidth() * 100.0);
    extraHMarginSpinBox->setValue(m_framings.getFramingHeight() * 100.0);
    topMarginSpinBox->setValue(m_margins.top() * 100.0);
    bottomMarginSpinBox->setValue(m_margins.bottom() * 100.0);
    leftMarginSpinBox->setValue(m_margins.left() * 100.0);
    rightMarginSpinBox->setValue(m_margins.right() * 100.0);
}

void
OptionsWidget::updateSizeDisplay()
{
    QSizeF const agg_size = m_ptrSettings->getAggregateHardSize();
    float const exwidth = m_framings.getFramingWidth();
    float const exheight = m_framings.getFramingHeight();
    float agwidth = agg_size.width();
    float agheight = agg_size.height();
    if(m_matchSizeMode == MatchSizeMode::DISABLED)
    {
        QSizeF const content_size = m_ptrSettings->getContentSize(m_pageId);
        RelativeMargins const hard_magrins = m_ptrSettings->getHardMargins(m_pageId);
        double const pagewidthx = content_size.width();
        double const pagewidthy = content_size.height() * 0.7071067811865475244;
        double const pagewidth = (pagewidthx < pagewidthy) ? pagewidthy : pagewidthx;
        agwidth = content_size.width() + pagewidth * (hard_magrins.left() + hard_magrins.right());
        agheight = content_size.height() + pagewidth * (hard_magrins.top() + hard_magrins.bottom());
    }
    int const outwidth = (int) (agwidth * (1.0f + exwidth) + 0.5f);
    int const outheight = (int) (agheight * (1.0f + exheight) + 0.5f);
    labelExtraWout->setText(tr(" = %1").arg(outwidth));
    labelExtraHout->setText(tr(" = %1").arg(outheight));
}

void
OptionsWidget::updateLinkDisplay(QToolButton* button, bool const linked)
{
    button->setIcon(linked ? m_chainIcon : m_brokenChainIcon);
}

void
OptionsWidget::enableDisableAlignmentButtons()
{
    bool const enabled = !matchSizeDisabledRb->isChecked();

    alignTopLeftBtn->setEnabled(enabled);
    alignTopBtn->setEnabled(enabled);
    alignTopRightBtn->setEnabled(enabled);
    alignLeftBtn->setEnabled(enabled);
    alignCenterBtn->setEnabled(enabled);
    alignRightBtn->setEnabled(enabled);
    alignBottomLeftBtn->setEnabled(enabled);
    alignBottomBtn->setEnabled(enabled);
    alignBottomRightBtn->setEnabled(enabled);
}

} // namespace page_layout

