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
#include <math.h>
#include <assert.h>
#include <boost/bind/bind.hpp>
#include <Qt>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#include <QMarginsF>
#include <QRect>
#include <QSize>
#include <QString>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QDebug>
#include "ImageView.h"
#include "OptionsWidget.h"
#include "RelativeMargins.h"
#include "Settings.h"
#include "ContentBox.h"
#include "ImagePresentation.h"
#include "PageLayout.h"
#include "Utils.h"
#include "imageproc/AffineTransformedImage.h"

using namespace imageproc;

namespace page_layout
{

ImageView::ImageView(
    std::shared_ptr<AcceleratableOperations> const& accel_ops,
    IntrusivePtr<Settings> const& settings,
    PageId const& page_id,
    std::shared_ptr<AbstractImageTransform const> const& orig_transform,
    AffineTransformedImage const& affine_transformed_image,
    ImagePixmapUnion const& downscaled_image,
    ContentBox const& content_box,
    OptionsWidget const& opt_widget)
    :   ImageViewBase(
            accel_ops, affine_transformed_image.origImage(), downscaled_image,
            ImagePresentation(
                affine_transformed_image.xform().transform(),
                affine_transformed_image.xform().transformedCropArea()
            ),
            QMarginsF(0.0, 0.0, 0.0, 0.0)
        ),
        m_dragHandler(*this),
        m_zoomHandler(*this),
        m_ptrSettings(settings),
        m_pageId(page_id),
        m_unscaledAffineTransform(affine_transformed_image.xform()),
        m_unscaledContentRect(content_box.toTransformedRect(*orig_transform)),
        m_affineImageContentTopLeft(
            m_unscaledAffineTransform.transform().inverted().map(
                m_unscaledContentRect.topLeft()
            )
        ),
        m_aggregateHardSize(settings->getAggregateHardSize()),
        m_committedAggregateHardSize(m_aggregateHardSize),
        m_matchSizeMode(opt_widget.matchSizeMode()),
        m_alignment(opt_widget.alignment()),
        m_framings(opt_widget.framings()),
        m_leftRightLinked(opt_widget.leftRightLinked()),
        m_topBottomLinked(opt_widget.topBottomLinked())
{
    setMouseTracking(true);

    interactionState().setDefaultStatusTip(
        tr("Resize margins by dragging any of the solid lines.")
    );

    // Setup interaction stuff.
    static int const masks_by_edge[] = { TOP, RIGHT, BOTTOM, LEFT };
    static int const masks_by_corner[] = { TOP|LEFT, TOP|RIGHT, BOTTOM|RIGHT, BOTTOM|LEFT };
    for (int i = 0; i < 4; ++i)
    {
        // Proximity priority - inner rect higher than middle, corners higher than edges.
        m_innerCorners [i].setProximityPriorityCallback([]()
        {
            return 4;
        });
        m_innerEdges   [i].setProximityPriorityCallback([]()
        {
            return 3;
        });
        m_middleCorners[i].setProximityPriorityCallback([]()
        {
            return 2;
        });
        m_middleEdges  [i].setProximityPriorityCallback([]()
        {
            return 1;
        });

        // Proximity.
        m_innerCorners[i].setProximityCallback(
            boost::bind(&ImageView::cornerProximity, this, masks_by_corner[i], &m_innerRect, boost::placeholders::_1)
        );
        m_middleCorners[i].setProximityCallback(
            boost::bind(&ImageView::cornerProximity, this, masks_by_corner[i], &m_middleRect, boost::placeholders::_1)
        );
        m_innerEdges[i].setProximityCallback(
            boost::bind(&ImageView::edgeProximity, this, masks_by_edge[i], &m_innerRect, boost::placeholders::_1)
        );
        m_middleEdges[i].setProximityCallback(
            boost::bind(&ImageView::edgeProximity, this, masks_by_edge[i], &m_middleRect, boost::placeholders::_1)
        );

        // Drag initiation.
        m_innerCorners[i].setDragInitiatedCallback(
            boost::bind(&ImageView::dragInitiated, this, boost::placeholders::_1)
        );
        m_middleCorners[i].setDragInitiatedCallback(
            boost::bind(&ImageView::dragInitiated, this, boost::placeholders::_1)
        );
        m_innerEdges[i].setDragInitiatedCallback(
            boost::bind(&ImageView::dragInitiated, this, boost::placeholders::_1)
        );
        m_middleEdges[i].setDragInitiatedCallback(
            boost::bind(&ImageView::dragInitiated, this, boost::placeholders::_1)
        );

        // Drag continuation.
        m_innerCorners[i].setDragContinuationCallback(
            boost::bind(&ImageView::innerRectDragContinuation, this, masks_by_corner[i], boost::placeholders::_1)
        );
        m_middleCorners[i].setDragContinuationCallback(
            boost::bind(&ImageView::middleRectDragContinuation, this, masks_by_corner[i], boost::placeholders::_1)
        );
        m_innerEdges[i].setDragContinuationCallback(
            boost::bind(&ImageView::innerRectDragContinuation, this, masks_by_edge[i], boost::placeholders::_1)
        );
        m_middleEdges[i].setDragContinuationCallback(
            boost::bind(&ImageView::middleRectDragContinuation, this, masks_by_edge[i], boost::placeholders::_1)
        );

        // Drag finishing.
        m_innerCorners[i].setDragFinishedCallback(
            boost::bind(&ImageView::dragFinished, this)
        );
        m_middleCorners[i].setDragFinishedCallback(
            boost::bind(&ImageView::dragFinished, this)
        );
        m_innerEdges[i].setDragFinishedCallback(
            boost::bind(&ImageView::dragFinished, this)
        );
        m_middleEdges[i].setDragFinishedCallback(
            boost::bind(&ImageView::dragFinished, this)
        );

        m_innerCornerHandlers[i].setObject(&m_innerCorners[i]);
        m_middleCornerHandlers[i].setObject(&m_middleCorners[i]);
        m_innerEdgeHandlers[i].setObject(&m_innerEdges[i]);
        m_middleEdgeHandlers[i].setObject(&m_middleEdges[i]);

        Qt::CursorShape corner_cursor = (i & 1) ? Qt::SizeBDiagCursor : Qt::SizeFDiagCursor;
        m_innerCornerHandlers[i].setProximityCursor(corner_cursor);
        m_innerCornerHandlers[i].setInteractionCursor(corner_cursor);
        m_middleCornerHandlers[i].setProximityCursor(corner_cursor);
        m_middleCornerHandlers[i].setInteractionCursor(corner_cursor);

        Qt::CursorShape edge_cursor = (i & 1) ? Qt::SizeHorCursor : Qt::SizeVerCursor;
        m_innerEdgeHandlers[i].setProximityCursor(edge_cursor);
        m_innerEdgeHandlers[i].setInteractionCursor(edge_cursor);
        m_middleEdgeHandlers[i].setProximityCursor(edge_cursor);
        m_middleEdgeHandlers[i].setInteractionCursor(edge_cursor);

        makeLastFollower(m_innerCornerHandlers[i]);
        makeLastFollower(m_innerEdgeHandlers[i]);
        makeLastFollower(m_middleCornerHandlers[i]);
        makeLastFollower(m_middleEdgeHandlers[i]);
    }

    rootInteractionHandler().makeLastFollower(*this);
    rootInteractionHandler().makeLastFollower(m_dragHandler);
    rootInteractionHandler().makeLastFollower(m_zoomHandler);

    recalcBoxesAndFit(opt_widget.margins());
}

ImageView::~ImageView()
{
}

void
ImageView::marginsSetExternally(RelativeMargins const& margins)
{
    AggregateSizeChanged const changed = commitHardMargins(margins);

    recalcBoxesAndFit(margins);

    invalidateThumbnails(changed);
}

void
ImageView::leftRightLinkToggled(bool const linked)
{
    m_leftRightLinked = linked;
    if (linked)
    {
        RelativeMargins margins(calcHardMargins());
        if (margins.left() != margins.right())
        {
            double const new_margin = std::min(
                                          margins.left(), margins.right()
                                      );
            margins.setLeft(new_margin);
            margins.setRight(new_margin);

            AggregateSizeChanged const changed =
                commitHardMargins(margins);

            recalcBoxesAndFit(margins);
            emit marginsSetLocally(margins);

            invalidateThumbnails(changed);
        }
    }
}

void
ImageView::topBottomLinkToggled(bool const linked)
{
    m_topBottomLinked = linked;
    if (linked)
    {
        RelativeMargins margins(calcHardMargins());
        if (margins.top() != margins.bottom())
        {
            double const new_margin = std::min(
                                          margins.top(), margins.bottom()
                                      );
            margins.setTop(new_margin);
            margins.setBottom(new_margin);

            AggregateSizeChanged const changed =
                commitHardMargins(margins);

            recalcBoxesAndFit(margins);
            emit marginsSetLocally(margins);

            invalidateThumbnails(changed);
        }
    }
}

void
ImageView::matchSizeModeChanged(MatchSizeMode const& match_size_mode)
{
    m_matchSizeMode = match_size_mode;

    Settings::AggregateSizeChanged const size_changed =
        m_ptrSettings->setMatchSizeMode(m_pageId, match_size_mode);

    if (size_changed == Settings::AGGREGATE_SIZE_CHANGED)
    {
        // Need to update both m_aggregateHardSize and m_committedAggregateHardSize,
        // as the former is used by recalcBoxesAndFit() below.
        m_aggregateHardSize = m_ptrSettings->getAggregateHardSize();
        m_committedAggregateHardSize = m_aggregateHardSize;
    }

    recalcBoxesAndFit(calcHardMargins());

    if (size_changed == Settings::AGGREGATE_SIZE_CHANGED)
    {
        emit invalidateAllThumbnails();
    }
    else
    {
        emit invalidateThumbnail(m_pageId);
    }
}

void
ImageView::alignmentChanged(Alignment const& alignment)
{
    m_alignment = alignment;
    m_ptrSettings->setPageAlignment(m_pageId, alignment);
    recalcBoxesAndFit(calcHardMargins());
    emit invalidateThumbnail(m_pageId);
}

void
ImageView::framingsChanged(Framings const& framings)
{
    m_framings = framings;
    m_ptrSettings->setPageFramings(m_pageId, framings);
    emit invalidateThumbnail(m_pageId);
}

void
ImageView::aggregateHardSizeChanged()
{
    m_aggregateHardSize = m_ptrSettings->getAggregateHardSize();
    m_committedAggregateHardSize = m_aggregateHardSize;
    recalcOuterRect();
    updatePresentationTransform(FIT);
}

void
ImageView::onPaint(QPainter& painter, InteractionState const& interaction)
{
    Q_UNUSED(interaction);
    
    QRectF centerRectV = QRectF((m_extraRect.right() + m_extraRect.left()) * 0.5f, m_extraRect.top(), 0.0f, (m_extraRect.bottom() - m_extraRect.top()));
    QRectF centerRectH = QRectF(m_extraRect.left(), (m_extraRect.bottom() + m_extraRect.top()) * 0.5f, (m_extraRect.right() - m_extraRect.left()), 0.0f);

    QPainterPath outer_outline;
//    outer_outline.addPolygon(m_outerRect);
    outer_outline.addPolygon(m_extraRect);

    QPainterPath content_outline;
    content_outline.addPolygon(m_innerRect);

    painter.setRenderHint(QPainter::Antialiasing, false);

    QColor const bg_color(Utils::backgroundColorForMatchSizeMode(m_matchSizeMode));
    QColor const border_color(Utils::borderColorForMatchSizeMode(m_matchSizeMode));

    painter.setPen(Qt::NoPen);
    painter.setBrush(bg_color);
    painter.drawPath(outer_outline.subtracted(content_outline));

    QPen pen(border_color);
    pen.setCosmetic(true);
    pen.setWidthF(2.0);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(centerRectV);
    painter.drawRect(centerRectH);
    painter.drawRect(m_middleRect);
    painter.drawRect(m_innerRect);
    painter.drawRect(m_extraRect);

    if (m_matchSizeMode.get() != MatchSizeMode::M_DISABLED)
    {
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(m_outerRect);
    }
}

Proximity
ImageView::cornerProximity(
    int const edge_mask, QRectF const* box, QPointF const& mouse_pos) const
{
    QRectF const r(virtualToWidget().mapRect(*box));
    QPointF pt;

    if (edge_mask & TOP)
    {
        pt.setY(r.top());
    }
    else if (edge_mask & BOTTOM)
    {
        pt.setY(r.bottom());
    }

    if (edge_mask & LEFT)
    {
        pt.setX(r.left());
    }
    else if (edge_mask & RIGHT)
    {
        pt.setX(r.right());
    }

    return Proximity(pt, mouse_pos);
}

Proximity
ImageView::edgeProximity(
    int const edge_mask, QRectF const* box, QPointF const& mouse_pos) const
{
    QRectF const r(virtualToWidget().mapRect(*box));
    QLineF line;

    switch (edge_mask)
    {
    case TOP:
        line.setP1(r.topLeft());
        line.setP2(r.topRight());
        break;
    case BOTTOM:
        line.setP1(r.bottomLeft());
        line.setP2(r.bottomRight());
        break;
    case LEFT:
        line.setP1(r.topLeft());
        line.setP2(r.bottomLeft());
        break;
    case RIGHT:
        line.setP1(r.topRight());
        line.setP2(r.bottomRight());
        break;
    default:
        assert(!"Unreachable");
    }

    return Proximity::pointAndLineSegment(mouse_pos, line);
}

void
ImageView::dragInitiated(QPointF const& mouse_pos)
{
    m_beforeResizing.middleWidgetRect = virtualToWidget().mapRect(m_middleRect);
    m_beforeResizing.virtToWidget = virtualToWidget();
    m_beforeResizing.widgetToVirt = widgetToVirtual();
    m_beforeResizing.mousePos = mouse_pos;
    m_beforeResizing.focalPoint = getWidgetFocalPoint();
}

void
ImageView::innerRectDragContinuation(int edge_mask, QPointF const& mouse_pos)
{
    // What really happens when we resize the inner box is resizing
    // the middle box in the opposite direction and moving the scene
    // on screen so that the object being dragged is still under mouse.

    QPointF const delta(mouse_pos - m_beforeResizing.mousePos);
    double left_adjust = 0;
    double right_adjust = 0;
    double top_adjust = 0;
    double bottom_adjust = 0;

    if (edge_mask & LEFT)
    {
        left_adjust = delta.x();
        if (m_leftRightLinked)
        {
            right_adjust = -left_adjust;
        }
    }
    else if (edge_mask & RIGHT)
    {
        right_adjust = delta.x();
        if (m_leftRightLinked)
        {
            left_adjust = -right_adjust;
        }
    }
    if (edge_mask & TOP)
    {
        top_adjust = delta.y();
        if (m_topBottomLinked)
        {
            bottom_adjust = -top_adjust;
        }
    }
    else if (edge_mask & BOTTOM)
    {
        bottom_adjust = delta.y();
        if (m_topBottomLinked)
        {
            top_adjust = -bottom_adjust;
        }
    }

    QRectF widget_rect(m_beforeResizing.middleWidgetRect);
    widget_rect.adjust(-left_adjust, -top_adjust, -right_adjust, -bottom_adjust);

    m_middleRect = m_beforeResizing.widgetToVirt.mapRect(widget_rect);
    forceNonNegativeHardMargins(m_middleRect);
    widget_rect = m_beforeResizing.virtToWidget.mapRect(m_middleRect);

    double effective_dx = 0;
    double effective_dy = 0;

    QRectF const& old_widget_rect = m_beforeResizing.middleWidgetRect;
    if (edge_mask & LEFT)
    {
        effective_dx = old_widget_rect.left() - widget_rect.left();
    }
    else if (edge_mask & RIGHT)
    {
        effective_dx = old_widget_rect.right() - widget_rect.right();
    }
    if (edge_mask & TOP)
    {
        effective_dy = old_widget_rect.top() - widget_rect.top();
    }
    else if (edge_mask & BOTTOM)
    {
        effective_dy = old_widget_rect.bottom()- widget_rect.bottom();
    }

    // Updating the focal point is what makes the image move
    // as we drag an inner edge.
    QPointF fp(m_beforeResizing.focalPoint);
    fp += QPointF(effective_dx, effective_dy);
    setWidgetFocalPoint(fp);

    m_aggregateHardSize = m_ptrSettings->getAggregateHardSize(
                              m_pageId, m_middleRect.size(), m_matchSizeMode
                          );

    recalcOuterRect();

    updatePresentationTransform(DONT_FIT);

    emit marginsSetLocally(calcHardMargins());
}

void
ImageView::middleRectDragContinuation(int const edge_mask, QPointF const& mouse_pos)
{
    QPointF const delta(mouse_pos - m_beforeResizing.mousePos);
    double left_adjust = 0;
    double right_adjust = 0;
    double top_adjust = 0;
    double bottom_adjust = 0;

    QRectF const bounds(maxViewportRect());
    QRectF const old_middle_rect(m_beforeResizing.middleWidgetRect);

    if (edge_mask & LEFT)
    {
        left_adjust = delta.x();
        if (old_middle_rect.left() + left_adjust < bounds.left())
        {
            left_adjust = bounds.left() - old_middle_rect.left();
        }
        if (m_leftRightLinked)
        {
            right_adjust = -left_adjust;
        }
    }
    else if (edge_mask & RIGHT)
    {
        right_adjust = delta.x();
        if (old_middle_rect.right() + right_adjust > bounds.right())
        {
            right_adjust = bounds.right() - old_middle_rect.right();
        }
        if (m_leftRightLinked)
        {
            left_adjust = -right_adjust;
        }
    }
    if (edge_mask & TOP)
    {
        top_adjust = delta.y();
        if (old_middle_rect.top() + top_adjust < bounds.top())
        {
            top_adjust = bounds.top() - old_middle_rect.top();
        }
        if (m_topBottomLinked)
        {
            bottom_adjust = -top_adjust;
        }
    }
    else if (edge_mask & BOTTOM)
    {
        bottom_adjust = delta.y();
        if (old_middle_rect.bottom() + bottom_adjust > bounds.bottom())
        {
            bottom_adjust = bounds.bottom() - old_middle_rect.bottom();
        }
        if (m_topBottomLinked)
        {
            top_adjust = -bottom_adjust;
        }
    }

    {
        QRectF widget_rect(old_middle_rect);
        widget_rect.adjust(left_adjust, top_adjust, right_adjust, bottom_adjust);

        m_middleRect = m_beforeResizing.widgetToVirt.mapRect(widget_rect);
        forceNonNegativeHardMargins(m_middleRect); // invalidates widget_rect
    }

    m_aggregateHardSize = m_ptrSettings->getAggregateHardSize(
                              m_pageId, m_middleRect.size(), m_matchSizeMode
                          );

    recalcOuterRect();

    updatePresentationTransform(DONT_FIT);

    emit marginsSetLocally(calcHardMargins());
}

void
ImageView::dragFinished()
{
    RelativeMargins const margins(calcHardMargins());
    AggregateSizeChanged const agg_size_changed(commitHardMargins(margins));

    if ((m_matchSizeMode == MatchSizeMode::M_SCALE) || (m_matchSizeMode == MatchSizeMode::M_AFFINE))
    {
        // In this mode, adjusting the margins affects scaling applied to the image itself.
        recalcBoxesAndFit(margins);
    }
    else
    {
        QRectF const extended_viewport(maxViewportRect().adjusted(-0.5, -0.5, 0.5, 0.5));
        if (extended_viewport.contains(m_beforeResizing.middleWidgetRect))
        {
            updatePresentationTransform(FIT);
        }
        else
        {
            updatePresentationTransform(DONT_FIT);
        }
    }

    invalidateThumbnails(agg_size_changed);
}

/**
 * Updates m_innerRect, m_middleRect and m_outerRect based on \p margins,
 * m_matchSizeMode, m_aggregateHardSize and m_alignmen. Also updates the
 * displayed area.
 */
void
ImageView::recalcBoxesAndFit(RelativeMargins const& margins)
{
    PageLayout const page_layout(
        m_unscaledContentRect,
        m_aggregateHardSize,
        m_matchSizeMode,
        m_alignment,
        m_framings,
        margins
    );

    m_innerRect = page_layout.innerRect();
    m_middleRect = page_layout.middleRect();
    m_outerRect = page_layout.outerRect();
    m_extraRect = page_layout.extraRect(m_framings);

    AffineImageTransform scaled_transform(m_unscaledAffineTransform);
    page_layout.absorbScalingIntoTransform(scaled_transform);
    scaled_transform.translateSoThatPointBecomes(
        scaled_transform.transform().map(m_affineImageContentTopLeft),
        m_innerRect.topLeft()
    );

    setZoomLevel(1.0);
    updateTransformAndFixFocalPoint(
        ImagePresentation(scaled_transform.transform(), m_extraRect),
        CENTER_IF_FITS
    );
}

/**
 * Updates the virtual image area to be displayed by ImageViewBase,
 * optionally ensuring that this area completely fits into the view.
 *
 * \note virtualToImage() and imageToVirtual() are not affected by this.
 */
void
ImageView::updatePresentationTransform(FitMode const fit_mode)
{
    if (fit_mode == DONT_FIT)
    {
        updateTransformPreservingScale(ImagePresentation(imageToVirtual(), m_extraRect));
    }
    else
    {
        setZoomLevel(1.0);
        updateTransformAndFixFocalPoint(
            ImagePresentation(imageToVirtual(), m_extraRect), CENTER_IF_FITS
        );
    }
}

void
ImageView::forceNonNegativeHardMargins(QRectF& middle_rect) const
{
    if (middle_rect.left() > m_innerRect.left())
    {
        middle_rect.setLeft(m_innerRect.left());
    }
    if (middle_rect.right() < m_innerRect.right())
    {
        middle_rect.setRight(m_innerRect.right());
    }
    if (middle_rect.top() > m_innerRect.top())
    {
        middle_rect.setTop(m_innerRect.top());
    }
    if (middle_rect.bottom() < m_innerRect.bottom())
    {
        middle_rect.setBottom(m_innerRect.bottom());
    }
}

/**
 * \brief Calculates hard margins from m_innerRect and m_middleRect.
 */
RelativeMargins
ImageView::calcHardMargins() const
{
    double pagewidth = m_innerRect.width();
    double const pagewidthheight = m_innerRect.height() * 0.7071067811865475244;
    pagewidth = (pagewidth < pagewidthheight) ? pagewidthheight : pagewidth;
    double const scale = 1.0 / pagewidth;
    return RelativeMargins(
               (m_innerRect.left() - m_middleRect.left()) * scale,
               (m_innerRect.top() - m_middleRect.top()) * scale,
               (m_middleRect.right() - m_innerRect.right()) * scale,
               (m_middleRect.bottom() - m_innerRect.bottom()) * scale
           );
}

/**
 * \brief Recalculates m_outerRect based on m_middleRect, m_aggregateHardSize
 *        and m_alignment.
 */
void
ImageView::recalcOuterRect()
{
    QMarginsF const soft_margins(
        Utils::calcSoftMarginsPx(
            m_middleRect.size(), m_aggregateHardSize, m_matchSizeMode, m_alignment
        )
    );

    m_outerRect = m_middleRect.adjusted(
                      -soft_margins.left(), -soft_margins.top(),
                      soft_margins.right(), soft_margins.bottom()
                  );

    float const basew = m_outerRect.width();
    float const baseh = m_outerRect.height();
    float const extraw = basew * m_framings.getFramingWidth() * 0.5f;
    float const extrah = baseh * m_framings.getFramingHeight() * 0.5f;
    m_extraRect = m_outerRect.adjusted(-extraw, -extrah, extraw, extrah);
}

ImageView::AggregateSizeChanged
ImageView::commitHardMargins(RelativeMargins const& margins)
{
    m_ptrSettings->setHardMargins(m_pageId, margins);
    m_aggregateHardSize = m_ptrSettings->getAggregateHardSize();

    AggregateSizeChanged changed = AGGREGATE_SIZE_UNCHANGED;
    if (m_committedAggregateHardSize != m_aggregateHardSize)
    {
        changed = AGGREGATE_SIZE_CHANGED;
    }

    m_committedAggregateHardSize = m_aggregateHardSize;

    return changed;
}

void
ImageView::invalidateThumbnails(AggregateSizeChanged const agg_size_changed)
{
    if (agg_size_changed == AGGREGATE_SIZE_CHANGED)
    {
        emit invalidateAllThumbnails();
    }
    else
    {
        emit invalidateThumbnail(m_pageId);
    }
}

} // namespace page_layout
