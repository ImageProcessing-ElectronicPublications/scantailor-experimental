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

    if (have_content_box)
    {
        double const agwidth = aggregate_hard_size.width();
        double const agheight = aggregate_hard_size.height();
        double const pagew = m_innerRect.width();
        double const pageh = m_innerRect.height();
        double const margins_w = margins.left() + margins.right();
        double const margins_h = margins.top() + margins.bottom();
        if ((match_size_mode.get() == MatchSizeMode::M_SCALE) || (match_size_mode.get() == MatchSizeMode::M_AFFINE))
        {
            double const pagewidthx = pagew;
            double const pagewidthy = pageh * 0.7071067811865475244;
            double const pagewidth = (pagewidthx < pagewidthy) ? pagewidthy : pagewidthx;
            double const pagewm = pagew + margins_w * pagewidth;
            double const pagehm = pageh + margins_h * pagewidth;
            if (match_size_mode.get() == MatchSizeMode::M_SCALE)
            {
                // aggregate_size = content_size * scale + margins * width * scale
                // Solving for scale:
                // scale = aggregate_size / (content_size + margins * width)
                double const x_scale = (pagewm > 0.0) ? (agwidth / pagewm) : 1.0;
                double const y_scale = (pagehm > 0.0) ? (agheight / pagehm) : 1.0;

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
                double const pgx = (pagewm < agwidth) ? agwidth : pagewm;
                double const pgy = (pagehm < agheight) ? agheight : pagehm;
                double const cgx = pgx / (1.0 + margins_w);
                double const cgy = pgy / (1.0 + margins_h * 0.7071067811865475244);
                double const cghx = cgy * 0.7071067811865475244;
                double const cgsize = (cgx < cghx) ? cghx : cgx;
                double const margins_ws = margins_w * cgsize;
                double const margins_hs = margins_h * cgsize;
                double const agx = pgx - margins_ws;
                double const agy = pgy - margins_hs;

                double const x_scale = (agx > 0.0) ? (agx / pagew) : 1.0;
                double const y_scale = (agy > 0.0) ? (agy / pageh) : 1.0;

                m_scaleFactor_X = x_scale;
                m_scaleFactor_Y = y_scale;
            }
            // The rectangle needs to be both shifted and scaled,
            // as that's what AbstractImageTransform::scale() does,
            // which we call in absorbScalingIntoTransform().
            QPointF const p_tl = m_innerRect.topLeft();
            QPointF const p_br = m_innerRect.bottomRight();
            double const p_tl_xs = p_tl.x() * m_scaleFactor_X;
            double const p_tl_ys = p_tl.y() * m_scaleFactor_Y;
            double const p_br_xs = p_br.x() * m_scaleFactor_X;
            double const p_br_ys = p_br.y() * m_scaleFactor_Y;

            QPointF const p_tl_s(p_tl_xs, p_tl_ys);
            QPointF const p_br_s(p_br_xs, p_br_ys);
            m_innerRect = QRectF(p_tl_s, p_br_s);
        }

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
