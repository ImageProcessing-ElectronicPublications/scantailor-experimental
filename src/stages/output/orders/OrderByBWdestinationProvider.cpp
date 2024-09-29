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
#include "../Params.h"
#include "OrderByBWdestinationProvider.h"

namespace output
{

OrderByBWdestinationProvider::OrderByBWdestinationProvider(IntrusivePtr<Settings> const& settings)
    :   m_ptrSettings(settings)
{
}

bool
OrderByBWdestinationProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    Params const lhs_params(m_ptrSettings->getParams(lhs_page));
    Params const rhs_params(m_ptrSettings->getParams(rhs_page));

    MetricsOptions const& lhs_metrics = lhs_params.getMetricsOptions();
    MetricsOptions const& rhs_metrics = rhs_params.getMetricsOptions();

    double lhs_bw = lhs_metrics.getMetricBWdestination();
    double rhs_bw = rhs_metrics.getMetricBWdestination();

    bool const lhs_valid = !lhs_incomplete;
    bool const rhs_valid = !rhs_incomplete;

    if (lhs_valid != rhs_valid)
    {
        // Invalid (unknown) sizes go to the back.
        return lhs_valid;
    }

    return (lhs_bw < rhs_bw);
}

} // namespace deskew
