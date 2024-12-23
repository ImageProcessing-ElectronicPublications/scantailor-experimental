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
#include "DebugImagesImpl.h"
#include "OptionsWidget.h"
#include "AutoManualMode.h"
#include "Dependencies.h"
#include "Params.h"
#include "Settings.h"
#include "TaskStatus.h"
#include "ContentBoxFinder.h"
#include "FilterUiInterface.h"
#include "ImageView.h"
#include "OrthogonalRotation.h"
#include "imageproc/AbstractImageTransform.h"
#include "imageproc/AffineImageTransform.h"
#include "imageproc/AffineTransformedImage.h"
#include "stages/filtering/Task.h"
#include <QObject>
#include <QTransform>
#include <QDebug>
#include <boost/optional.hpp>
#include <assert.h>

using namespace imageproc;

namespace select_content
{

class Task::UiUpdater : public FilterResult
{
public:
    UiUpdater(IntrusivePtr<Filter> const& filter,
              std::shared_ptr<AcceleratableOperations> const& accel_ops,
              PageId const& page_id, Params const& params,
              std::unique_ptr<DebugImagesImpl> dbg,
              std::shared_ptr<AbstractImageTransform const> const& orig_transform,
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
    Params m_params;
    std::unique_ptr<DebugImagesImpl> m_ptrDbg;
    std::shared_ptr<AbstractImageTransform const> m_ptrOrigTransform;
    AffineTransformedImage m_affineTransformedImage;
    QImage m_downscaledImage;
    bool m_batchProcessing;
};


Task::Task(IntrusivePtr<Filter> const& filter,
           IntrusivePtr<filtering::Task> const& next_task,
           IntrusivePtr<Settings> const& settings,
           PageId const& page_id, bool const batch, bool const debug)
    : m_ptrFilter(filter),
      m_ptrNextTask(next_task),
      m_ptrSettings(settings),
      m_pageId(page_id),
      m_batchProcessing(batch)
{
    if (debug)
    {
        m_ptrDbg.reset(new DebugImagesImpl);
    }
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
    std::shared_ptr<AbstractImageTransform const> const& orig_image_transform)
{
    assert(!orig_image.isNull());
    assert(orig_image_transform);

    status.throwIfCancelled();

    Dependencies const deps(orig_image_transform->fingerprint());

    std::unique_ptr<Params> params(m_ptrSettings->getPageParams(m_pageId));
    if (params.get() && !params->dependencies().matches(deps))
    {
        // Dependency mismatch.
        if (params->mode() == MODE_AUTO)
        {
            params.reset();
        }
        else
        {
            // If the content box was set manually, we don't want to lose it
            // just because the user went back and adjusted the warping grid slightly.
            // We still need to update params->contentSizePx() however.
            params->setContentSizePx(
                params->contentBox().toTransformedRect(*orig_image_transform).size()
            );
            params->setDependencies(deps);
            m_ptrSettings->setPageParams(m_pageId, *params);
        }

    }

    boost::optional<AffineTransformedImage> dewarped;

    if (!params.get())
    {
        dewarped = orig_image_transform->toAffine(
                       orig_image, Qt::transparent, accel_ops
                   );

        QRectF const content_rect(
            ContentBoxFinder::findContentBox(status, accel_ops, *dewarped, m_ptrDbg.get())
        );

        params.reset(
            new Params(
                ContentBox(*orig_image_transform, content_rect),
                content_rect.size(), deps, MODE_AUTO
            )
        );

        m_ptrSettings->setPageParams(m_pageId, *params);
    }

    status.throwIfCancelled();

    if (m_ptrNextTask)
    {
        return m_ptrNextTask->process(
                   status, accel_ops, orig_image, gray_orig_image_factory,
                   orig_image_transform, dewarped, params->contentBox()
               );
    }
    else
    {
        if (!dewarped)
        {
            dewarped = orig_image_transform->toAffine(
                           orig_image, Qt::transparent, accel_ops
                       );
        }

        return FilterResultPtr(
                   new UiUpdater(
                       m_ptrFilter, accel_ops, m_pageId, *params, std::move(m_ptrDbg),
                       orig_image_transform, *dewarped, m_batchProcessing
                   )
               );
    }
}


/*============================ Task::UiUpdater ==========================*/

Task::UiUpdater::UiUpdater(
    IntrusivePtr<Filter> const& filter,
    std::shared_ptr<AcceleratableOperations> const& accel_ops,
    PageId const& page_id,
    Params const& params, std::unique_ptr<DebugImagesImpl> dbg,
    std::shared_ptr<AbstractImageTransform const> const& orig_transform,
    AffineTransformedImage const& affine_transformed_image, bool const batch)
    : m_ptrFilter(filter),
      m_ptrAccelOps(accel_ops),
      m_pageId(page_id),
      m_params(params),
      m_ptrDbg(std::move(dbg)),
      m_ptrOrigTransform(orig_transform),
      m_affineTransformedImage(affine_transformed_image),
      m_downscaledImage(
          ImageViewBase::createDownscaledImage(affine_transformed_image.origImage(), accel_ops)
      ),
      m_batchProcessing(batch)
{
}

void
Task::UiUpdater::updateUI(FilterUiInterface* ui)
{
    // This function is executed from the GUI thread.

    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    opt_widget->postUpdateUI(m_params);
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing)
    {
        return;
    }

    ImageView* view = new ImageView(
        m_ptrAccelOps, m_ptrOrigTransform, m_affineTransformedImage,
        m_downscaledImage, m_params.contentBox()
    );
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP, m_ptrDbg.get());

    QObject::connect(
        view, SIGNAL(manualContentBoxSet(ContentBox const&, QSizeF const&)),
        opt_widget, SLOT(manualContentBoxSet(ContentBox const&, QSizeF const&))
    );
}

} // namespace select_content
