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

#ifndef FILTERING_CACHEDRIVENTASK_H_
#define FILTERING_CACHEDRIVENTASK_H_

#include "NonCopyable.h"
#include "RefCountable.h"
#include "IntrusivePtr.h"
#include "ContentBox.h"
#include <memory>

class PageInfo;
class AbstractFilterDataCollector;

namespace imageproc
{
class AbstractImageTransform;
}

namespace page_layout
{
class CacheDrivenTask;
}

namespace filtering
{

class CacheDrivenTask : public RefCountable
{
	DECLARE_NON_COPYABLE(CacheDrivenTask)
public:
	CacheDrivenTask(IntrusivePtr<page_layout::CacheDrivenTask> const& next_task);

    virtual ~CacheDrivenTask();

    void process(
        PageInfo const& page_info, AbstractFilterDataCollector* collector,
        std::shared_ptr<imageproc::AbstractImageTransform const> const& full_size_image_transform,
        ContentBox const& content_box);
private:
    IntrusivePtr<page_layout::CacheDrivenTask> m_ptrNextTask;
};

} // filtering

#endif
