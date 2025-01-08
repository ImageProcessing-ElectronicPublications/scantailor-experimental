/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "FilterOptionsWidget.h"

bool FilterOptionsWidget::disconnectAll(void)
{
    bool result = true;
    
    if(!disconnect(this, SIGNAL(FilterOptionsWidget::reloadRequested()), 0, 0)) result = false;
    if(!disconnect(this, SIGNAL(FilterOptionsWidget::invalidateThumbnail(PageId const&)), 0, 0)) result = false;
    if(!disconnect(this, SIGNAL(FilterOptionsWidget::invalidateThumbnail(PageInfo const&)), 0, 0)) result = false;
    if(!disconnect(this, SIGNAL(FilterOptionsWidget::invalidateAllThumbnails()), 0, 0)) result = false;
    if(!disconnect(this, SIGNAL(FilterOptionsWidget::goToPage(PageId const&)), 0, 0)) result = false;
    
    return result;
}
