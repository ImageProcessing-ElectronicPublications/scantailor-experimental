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

#include "CacheDrivenTask.h"
#include "PageInfo.h"
#include "ThumbnailBase.h"
#include "filter_dc/ThumbnailCollector.h"
#include "stages/page_layout/CacheDrivenTask.h"

namespace filtering
{

CacheDrivenTask::CacheDrivenTask(
	IntrusivePtr<page_layout::CacheDrivenTask> const& next_task)
	: m_ptrNextTask(next_task)
{
}

CacheDrivenTask::~CacheDrivenTask()
{
}

void
CacheDrivenTask::process(
    PageInfo const& page_info, AbstractFilterDataCollector* collector,
    std::shared_ptr<imageproc::AbstractImageTransform const> const& full_size_image_transform,
    ContentBox const& content_box)
{
	if (m_ptrNextTask)
	{
		m_ptrNextTask->process(
			page_info, collector, full_size_image_transform, content_box
		);
		return;
	}

    if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector))
    {
        thumb_col->processThumbnail(
            std::unique_ptr<QGraphicsItem>(
                new ThumbnailBase(
                    thumb_col->thumbnailCache(),
                    thumb_col->maxLogicalThumbSize(),
                    page_info.id(),
                    *full_size_image_transform
                )
            )
        );
    }
}

} // filtering
