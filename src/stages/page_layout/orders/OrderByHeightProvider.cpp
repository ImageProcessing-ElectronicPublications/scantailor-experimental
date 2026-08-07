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

#include <memory>
#include <QSizeF>
#include "../Params.h"
#include "RelativeMargins.h"
#include "OrderByHeightProvider.h"

namespace page_layout
{

OrderByHeightProvider::OrderByHeightProvider(IntrusivePtr<Settings> const& settings)
    : m_ptrSettings(settings)
{
}

bool
OrderByHeightProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    std::unique_ptr<Params> const lhs_params(m_ptrSettings->getPageParams(lhs_page));
    std::unique_ptr<Params> const rhs_params(m_ptrSettings->getPageParams(rhs_page));

    QSizeF lhs_size;
    float lhs_scale_h = 1.0f;
    if (lhs_params.get())
    {
        lhs_size = lhs_params->hardMargins().extendContentSize(lhs_params->contentSize());
        lhs_scale_h = lhs_params->matchSizeMode().getSubScaleHeight();
    }
    QSizeF rhs_size;
    float rhs_scale_h = 1.0f;
    if (rhs_params.get())
    {
        rhs_size = rhs_params->hardMargins().extendContentSize(rhs_params->contentSize());
        rhs_scale_h = rhs_params->matchSizeMode().getSubScaleHeight();
    }

    bool const lhs_valid = !lhs_incomplete && lhs_size.isValid();
    bool const rhs_valid = !rhs_incomplete && rhs_size.isValid();

    if (lhs_valid != rhs_valid)
    {
        // Invalid (unknown) sizes go to the back.
        return lhs_valid;
    }

    float const lk = 1.0f + lhs_scale_h * (lhs_size.height() - 1.0f);
    float const rk = 1.0f + rhs_scale_h * (rhs_size.height() - 1.0f);

    return (lk < rk);
}

} // namespace page_layout
