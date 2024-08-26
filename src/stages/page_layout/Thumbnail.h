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

#ifndef PAGE_LAYOUT_THUMBNAIL_H_
#define PAGE_LAYOUT_THUMBNAIL_H_

#include <QTransform>
#include <QRectF>
#include "ThumbnailBase.h"
#include "Params.h"
#include "PageLayout.h"
#include "Framings.h"
#include "IntrusivePtr.h"

class ThumbnailPixmapCache;
class PageId;

namespace imageproc
{
class AbstractImageTransform;
}

namespace page_layout
{

class Thumbnail : public ThumbnailBase
{
public:
    Thumbnail(IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
              QSizeF const& max_display_size,
              PageId const& page_id,
              Params const& params,
              imageproc::AbstractImageTransform const& full_size_image_transform,
              PageLayout const& page_layout,
              Framings const& framings);

    virtual void paintOverImage(
        QPainter& painter,
        QTransform const& transformed_to_display,
        QTransform const& thumb_to_display);
private:
    Params m_params;
    PageLayout m_pageLayout;
    Framings m_framings;
};

} // namespace page_layout

#endif
