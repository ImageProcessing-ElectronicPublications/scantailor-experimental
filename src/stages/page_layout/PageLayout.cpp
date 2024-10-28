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

#include <algorithm>
#include <QTransform>
#include <QtGlobal>
#include <QMarginsF>
#include "PageLayout.h"
#include "MatchSizeMode.h"
#include "Alignment.h"
#include "Framings.h"
#include "RelativeMargins.h"
#include "Utils.h"
#include "imageproc/AbstractImageTransform.h"

using namespace imageproc;

namespace page_layout
{

PageLayout::PageLayout(
    QRectF const& unscaled_content_rect,
    QSizeF const& aggregate_hard_size,
    MatchSizeMode const& match_size_mode,
    Alignment const& alignment,
    Framings const& framings,
    RelativeMargins const& margins)
{
    m_innerRect = unscaled_content_rect;
    m_scaleFactor_X = 1.0;
    m_scaleFactor_Y = 1.0;

    // An empty unscaled_content_rect is a special case to indicate
    // a missing content box. In this case, we want the geometry
    // we would get with zero hard margins and MatchSizeMode::M_GROW_MARGINS.
    bool const have_content_box = !unscaled_content_rect.isEmpty();

    if (have_content_box && (match_size_mode.get() == MatchSizeMode::M_SCALE || match_size_mode.get() == MatchSizeMode::M_AFFINE))
    {
        // aggregate_size = content_size * scale + margins * width * scale
        // Solving for scale:
        // scale = aggregate_size / (content_size + margins * width)
        double const agwidth = aggregate_hard_size.width();
        double const agheight = aggregate_hard_size.height();
        double const aghx = agheight * 0.7071067811865475244;
        double const agsize = (agwidth < aghx) ? agwidth : aghx;
        double const agx = (agwidth - (margins.left() + margins.right()) * agsize);
        double const agy = (agheight - (margins.top() + margins.bottom()) * agsize);
        double const x_scale = (agx > 0.0) ? (agx / m_innerRect.width()) : 1.0;
        double const y_scale = (agy > 0.0) ? (agy / m_innerRect.height()) : 1.0;
        if (match_size_mode.get() == MatchSizeMode::M_SCALE)
        {
            if (x_scale > 1.0 && y_scale > 1.0)
            {
                m_scaleFactor_X = std::min(x_scale, y_scale);
            }
            else if (x_scale < 1.0 && y_scale < 1.0)
            {
                m_scaleFactor_X = std::max(x_scale, y_scale);
            }
            m_scaleFactor_Y = m_scaleFactor_X;
        }
        else
        {
            m_scaleFactor_X = x_scale;
            m_scaleFactor_Y = y_scale;
        }

        // The rectangle needs to be both shifted and scaled,
        // as that's what AbstractImageTransform::scale() does,
        // which we call in absorbScalingIntoTransform().
        QPointF const p_tl = m_innerRect.topLeft();
        QPointF const p_br = m_innerRect.bottomRight();
        QPointF const p_tl_s(p_tl.x() * m_scaleFactor_X, p_tl.y() * m_scaleFactor_Y);
        QPointF const p_br_s(p_br.x() * m_scaleFactor_X, p_br.y() * m_scaleFactor_Y);
        m_innerRect = QRectF(p_tl_s, p_br_s);
    }

    if (have_content_box)
    {
        m_middleRect = margins.extendContentRect(m_innerRect);
    }
    else
    {
        m_middleRect = m_innerRect;
    }

    QMarginsF const soft_margins(
        Utils::calcSoftMarginsPx(
            m_middleRect.size(), aggregate_hard_size, match_size_mode, alignment
        )
    );

    m_outerRect = m_middleRect.adjusted(
                      -soft_margins.left(), -soft_margins.top(),
                      soft_margins.right(), soft_margins.bottom()
                  );
}

QRectF const
PageLayout::extraRect(Framings const& framings) const
{
    QRectF const base = outerRect();
    float const basew = base.width();
    float const baseh = base.height();
    float const extraw = basew * framings.getFramingWidth() * 0.5f;
    float const extrah = baseh * framings.getFramingHeight() * 0.5f;
    QRectF const extra = m_outerRect.adjusted(-extraw, -extrah, extraw, extrah);

    return extra;
}

void
PageLayout::absorbScalingIntoTransform(AbstractImageTransform& transform) const
{
    if ((m_scaleFactor_X != 1.0) || (m_scaleFactor_Y != 1.0))
    {
        transform.scale(m_scaleFactor_X, m_scaleFactor_Y);
    }
}

} // namespace page_layout
