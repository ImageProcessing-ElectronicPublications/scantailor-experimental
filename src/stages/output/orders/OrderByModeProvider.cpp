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
#include "OrderByModeProvider.h"

namespace output
{

OrderByModeProvider::OrderByModeProvider(IntrusivePtr<Settings> const& settings)
    :   m_ptrSettings(settings)
{
}

bool
OrderByModeProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    Params const lhs_params(m_ptrSettings->getParams(lhs_page));
    Params const rhs_params(m_ptrSettings->getParams(rhs_page));

    bool const lhs_valid = !lhs_incomplete;
    bool const rhs_valid = !rhs_incomplete;

    if (lhs_valid != rhs_valid)
    {
        // Invalid (unknown) sizes go to the back.
        return lhs_valid;
    }

    ColorParams const& lhs_colors = lhs_params.colorParams();
    ColorParams const& rhs_colors = rhs_params.colorParams();

    int lhs_mode = (int) lhs_colors.colorMode();
    int rhs_mode = (int) rhs_colors.colorMode();

    if (lhs_mode == rhs_mode)
    {
        return (lhs_page < rhs_page);
    }
    else
    {
        return (lhs_mode < rhs_mode);
    }
}

} // namespace deskew
