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
#include "OrderByAngleHorProvider.h"

namespace deskew
{

OrderByAngleHorProvider::OrderByAngleHorProvider(IntrusivePtr<Settings> const& settings)
    :   m_ptrSettings(settings)
{
}

bool
OrderByAngleHorProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    std::unique_ptr<Params> const lhs_params(m_ptrSettings->getPageParams(lhs_page));
    std::unique_ptr<Params> const rhs_params(m_ptrSettings->getPageParams(rhs_page));

    bool const lhs_valid = !lhs_incomplete;
    bool const rhs_valid = !rhs_incomplete;

    if (lhs_valid != rhs_valid)
    {
        // Invalid (unknown) sizes go to the back.
        return lhs_valid;
    }

    double lhs_angle = 0.0;
    if (lhs_params.get())
    {
        switch (lhs_params->distortionType().get())
        {
        case DistortionType::NONE:
            lhs_angle = 0.0;
            break;
        case DistortionType::ROTATION:
            lhs_angle = 0.0;
            break;
        case DistortionType::PERSPECTIVE:
            lhs_angle = -lhs_params->perspectiveParams().getAngleHor();
            break;
        case DistortionType::WARP:
            lhs_angle = -lhs_params->dewarpingParams().getAngleHor();
            break;
        } // switch
    }

    double rhs_angle = 0.0;
    if (rhs_params.get())
    {
        switch (rhs_params->distortionType().get())
        {
        case DistortionType::NONE:
            rhs_angle = 0.0;
            break;
        case DistortionType::ROTATION:
            rhs_angle = 0.0;
            break;
        case DistortionType::PERSPECTIVE:
            rhs_angle = -rhs_params->perspectiveParams().getAngleHor();
            break;
        case DistortionType::WARP:
            rhs_angle = -rhs_params->dewarpingParams().getAngleHor();
            break;
        } // switch
    }

    if (lhs_angle == rhs_angle)
    {
        return (lhs_page < rhs_page);
    }
    else
    {
        return (lhs_angle < rhs_angle);
    }
}

} // namespace deskew
