/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>
    Copyright (C)  Vadim Kuznetsov ()DikBSD <dikbsd@gmail.com>

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

#include <memory>
#include <assert.h>
#include <QSizeF>
#include "../Params.h"
#include "../PageLayout.h"
#include "OrderBySplitPosProvider.h"

namespace page_split
{

OrderBySplitPosProvider::OrderBySplitPosProvider(IntrusivePtr<Settings> const& settings)
    :   m_ptrSettings(settings)
{
}

bool
OrderBySplitPosProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    if (lhs_incomplete != rhs_incomplete)
    {
        // Pages with question mark go to the bottom.
        return rhs_incomplete;
    }
    else if (lhs_incomplete)
    {
        assert(rhs_incomplete);
        // Two pages with question marks are ordered naturally.
        return lhs_page < rhs_page;
    }

    assert(lhs_incomplete == false);
    assert(rhs_incomplete == false);

    Settings::Record const lhs_record(m_ptrSettings->getPageRecord(lhs_page.imageId()));
    Settings::Record const rhs_record(m_ptrSettings->getPageRecord(rhs_page.imageId()));

    Params const* lhs_params = lhs_record.params();
    Params const* rhs_params = rhs_record.params();

    double lhs_layout_pos = 0.5;
    if (lhs_params)
    {
        lhs_layout_pos = lhs_params->pageLayout().getSplitPosition();
    }

    double rhs_layout_pos = 0.5;
    if (rhs_params)
    {
        rhs_layout_pos = rhs_params->pageLayout().getSplitPosition();
    }

    return lhs_layout_pos < rhs_layout_pos;
}

} // namespace page_split
