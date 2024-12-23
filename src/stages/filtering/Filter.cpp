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

#include "Filter.h"
#include "FilterUiInterface.h"
#include "OptionsWidget.h"
#include "PageId.h"
#include "Task.h"
#include "CacheDrivenTask.h"
#include <QCoreApplication>
#include <QString>
#include <QDomElement>
#include "CommandLine.h"

namespace filtering
{

Filter::Filter(PageSelectionAccessor const& page_selection_accessor)
{
	if (CommandLine::get().isGui())
	{
		m_ptrOptionsWidget.reset(
			new OptionsWidget()
		);
    }
}

Filter::~Filter()
{
}

QString
Filter::getName() const
{
	return QCoreApplication::translate("filtering::Filter", "Filtering");
}

PageView
Filter::getView() const
{
    return PAGE_VIEW;
}

void
Filter::performRelinking(AbstractRelinker const& relinker)
{
}

void
Filter::preUpdateUI(FilterUiInterface* ui, PageId const& page_id)
{
    if (m_ptrOptionsWidget.get())
    {
        ui->setOptionsWidget(m_ptrOptionsWidget.get(), ui->KEEP_OWNERSHIP);
    }
}

QDomElement
Filter::saveSettings(
	ProjectWriter const& writer, QDomDocument& doc) const
{
    QDomElement filter_el(doc.createElement("filtering"));
    return filter_el;
}

void
Filter::loadSettings(
    ProjectReader const& reader, QDomElement const& filters_el)
{
}

IntrusivePtr<Task>
Filter::createTask(
    PageId const& page_id,
    IntrusivePtr<page_layout::Task> const& next_task,
    bool batch, bool debug)
{
    return IntrusivePtr<Task>(
        new Task(
            IntrusivePtr<Filter>(this), next_task,
            page_id, batch, debug
        )
    );
}

IntrusivePtr<CacheDrivenTask>
Filter::createCacheDrivenTask(
    IntrusivePtr<page_layout::CacheDrivenTask> const& next_task)
{
	return IntrusivePtr<CacheDrivenTask>(
		new CacheDrivenTask(next_task)
	);
}

} // filtering
