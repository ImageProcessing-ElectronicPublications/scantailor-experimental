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

#include "Task.h"
#include "Filter.h"
#include "TaskStatus.h"
#include "FilterUiInterface.h"
#include "ImageViewBase.h"
#include "BasicImageView.h"
#include "stages/page_layout/Task.h"
#include "imageproc/AffineTransformedImage.h"

using namespace imageproc;

namespace filtering
{

class Task::UiUpdater : public FilterResult
{
public:
	UiUpdater(IntrusivePtr<Filter> const& filter,
		      std::shared_ptr<AcceleratableOperations> const& accel_ops,
			  PageId const& page_id,
		      AffineTransformedImage const& affine_transformed_image,
		      bool batch);

	virtual void updateUI(FilterUiInterface* ui);

	virtual IntrusivePtr<AbstractFilter> filter()
	{
		return m_ptrFilter;
	}

private:
	IntrusivePtr<Filter> m_ptrFilter;
	std::shared_ptr<AcceleratableOperations> m_ptrAccelOps;
	PageId m_pageId;
	AffineTransformedImage m_affineTransformedImage;
	QImage m_downscaledImage;
	bool m_batchProcessing;
};

Task::Task(IntrusivePtr<Filter> const& filter,
		   IntrusivePtr<page_layout::Task> const& next_task,
		   PageId const& page_id, bool batch, bool debug)
	: m_ptrFilter(filter)
	, m_ptrNextTask(next_task)
    , m_pageId(page_id)
    , m_batchProcessing(batch)
{
}

Task::~Task()
{
}

FilterResultPtr
Task::process(
	TaskStatus const& status,
	std::shared_ptr<AcceleratableOperations> const& accel_ops,
	QImage const& orig_image,
	CachingFactory<imageproc::GrayImage> const& gray_orig_image_factory,
	std::shared_ptr<imageproc::AbstractImageTransform const> const& orig_image_transform,
	boost::optional<imageproc::AffineTransformedImage> pre_transformed_image,
	ContentBox const& content_box)
{
	status.throwIfCancelled();

	if (m_ptrNextTask)
	{
		return m_ptrNextTask->process(
			status, accel_ops, orig_image, gray_orig_image_factory,
			orig_image_transform, pre_transformed_image, content_box
		);
	}
	else
	{
		if (!pre_transformed_image)
		{
			pre_transformed_image = orig_image_transform->toAffine(
				orig_image, Qt::transparent, accel_ops
			);
		}

		return FilterResultPtr(
			new UiUpdater(
				m_ptrFilter, accel_ops, m_pageId,
				*pre_transformed_image, m_batchProcessing
			)
		);
	}
}

/*============================ Task::UiUpdater ==========================*/

Task::UiUpdater::UiUpdater(
	IntrusivePtr<Filter> const& filter,
	std::shared_ptr<AcceleratableOperations> const& accel_ops,
	PageId const& page_id,
	AffineTransformedImage const& affine_transformed_image,
	bool batch)
	: m_ptrFilter(filter)
	, m_ptrAccelOps(accel_ops)
	, m_pageId(page_id)
	, m_affineTransformedImage(affine_transformed_image)
	, m_downscaledImage(
		ImageViewBase::createDownscaledImage(
			affine_transformed_image.origImage(),
			accel_ops))
	, m_batchProcessing(batch)
{
}

void
Task::UiUpdater::updateUI(FilterUiInterface* ui)
{
	if (m_batchProcessing)
	{
		return;
	}

	ui->invalidateThumbnail(m_pageId);

	ImageViewBase* view = new BasicImageView(
		m_ptrAccelOps, m_affineTransformedImage, m_downscaledImage
	);

	ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

} // filtering
