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

#include <QString>
#include "DespeckleLevel.h"

namespace output
{

int despeckleLevelToFactor(DespeckleLevel const level)
{
    int despeckle_factor = 0;

    switch (level)
    {
    case DESPECKLE_OFF:
        despeckle_factor = 0;
        break;
    case DESPECKLE_CAUTIOUS:
        despeckle_factor = 67;
        break;
    case DESPECKLE_NORMAL:
        despeckle_factor = 100;
        break;
    case DESPECKLE_AGGRESSIVE:
        despeckle_factor = 150;
        break;
    default:
        despeckle_factor = 0;
        break;
    }

    return despeckle_factor;
}

DespeckleLevel despeckleFactorToLevel(int const despeckle_factor)
{
    DespeckleLevel level = DESPECKLE_CUSTOM;

    switch (despeckle_factor)
    {
    case 0:
        level = DESPECKLE_OFF;
        break;
    case 67:
        level = DESPECKLE_CAUTIOUS;
        break;
    case 100:
        level = DESPECKLE_NORMAL;
        break;
    case 150:
        level = DESPECKLE_AGGRESSIVE;
        break;
    default:
        level = DESPECKLE_CUSTOM;
        break;
    }

    return level;
}

QString despeckleLevelToString(DespeckleLevel const level)
{
    QString level_string = QString();

    switch (level)
    {
    case DESPECKLE_OFF:
        level_string =  QString("off");
        break;
    case DESPECKLE_CAUTIOUS:
        level_string =  QString("cautious");
        break;
    case DESPECKLE_NORMAL:
        level_string =  QString("normal");
        break;
    case DESPECKLE_AGGRESSIVE:
        level_string =  QString("aggressive");
        break;
    case DESPECKLE_CUSTOM:
        level_string =  QString("custom");
        break;
    }

    return level_string;
}

DespeckleLevel despeckleLevelFromString(QString const& str)
{
    DespeckleLevel level = DESPECKLE_CUSTOM;

    if (str == "off")
    {
        level = DESPECKLE_OFF;
    }
    else if (str == "cautious")
    {
        level = DESPECKLE_CAUTIOUS;
    }
    else if (str == "normal")
    {
        level = DESPECKLE_NORMAL;
    }
    else if (str == "aggressive")
    {
        level = DESPECKLE_AGGRESSIVE;
    }
    else if (str == "custom")
    {
        level = DESPECKLE_CUSTOM;
    }

    return level;
}

} // namespace output
