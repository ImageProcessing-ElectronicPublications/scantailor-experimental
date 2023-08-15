/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2009  Joseph Artsimovich <joseph_a@mail.ru>

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

#include "ColorGrayscaleOptions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace output
{

ColorGrayscaleOptions::ColorGrayscaleOptions(QDomElement const& el)
    :  m_wienerCoef(el.attribute("wienerCoef").toDouble()),
       m_wienerWindowSize(el.attribute("wienerWinSize").toInt()),
       m_knndCoef(el.attribute("knnDenoiser").toDouble()),
       m_knndRadius(el.attribute("knnDRadius").toInt()),
       m_screenCoef(el.attribute("screenCoef").toDouble()),
       m_screenWindowSize(el.attribute("screenWinSize").toInt()),
       m_curveCoef(el.attribute("curveCoef").toDouble()),
       m_sqrCoef(el.attribute("sqrCoef").toDouble()),
       m_normalizeCoef(el.attribute("normalizeCoef").toDouble()),
       m_whiteMargins(el.attribute("whiteMargins") == "1")
{
    if (m_wienerCoef < 0.0 || m_wienerCoef > 1.0)
    {
        m_wienerCoef = 0.0;
    }
    if (m_wienerWindowSize < 3)
    {
        m_wienerWindowSize = 5;
    }
    if (m_knndCoef < 0.0 || m_knndCoef > 1.0)
    {
        m_knndCoef = 0.0;
    }
    if (m_knndRadius < 1)
    {
        m_knndRadius = 5;
    }
    if (m_screenCoef < -1.0 || m_screenCoef > 1.0)
    {
        m_screenCoef = 0.0;
    }
    if (m_screenWindowSize < 3)
    {
        m_screenWindowSize = 10;
    }
    if (m_curveCoef < -1.0 || m_curveCoef > 1.0)
    {
        m_curveCoef = 0.0;
    }
    if (m_sqrCoef < -1.0 || m_sqrCoef > 1.0)
    {
        m_sqrCoef = 0.0;
    }
    if (m_normalizeCoef < 0.0 || m_normalizeCoef > 1.0)
    {
        m_normalizeCoef = 0.0;
    }
}

QDomElement
ColorGrayscaleOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("wienerCoef", m_wienerCoef);
    el.setAttribute("wienerWinSize", m_wienerWindowSize);
    el.setAttribute("knnDenoiser", m_knndCoef);
    el.setAttribute("knnDRadius", m_knndRadius);
    el.setAttribute("screenCoef", m_screenCoef);
    el.setAttribute("screenWinSize", m_screenWindowSize);
    el.setAttribute("curveCoef", m_curveCoef);
    el.setAttribute("sqrCoef", m_sqrCoef);
    el.setAttribute("normalizeCoef", m_normalizeCoef);
    el.setAttribute("whiteMargins", m_whiteMargins ? "1" : "0");
    return el;
}

bool
ColorGrayscaleOptions::operator==(ColorGrayscaleOptions const& other) const
{
    if (m_wienerCoef != other.m_wienerCoef)
    {
        return false;
    }
    if (m_wienerWindowSize != other.m_wienerWindowSize)
    {
        return false;
    }
    if (m_knndCoef != other.m_knndCoef)
    {
        return false;
    }
    if (m_knndRadius != other.m_knndRadius)
    {
        return false;
    }
    if (m_screenCoef != other.m_screenCoef)
    {
        return false;
    }
    if (m_screenWindowSize != other.m_screenWindowSize)
    {
        return false;
    }
    if (m_curveCoef != other.m_curveCoef)
    {
        return false;
    }
    if (m_sqrCoef != other.m_sqrCoef)
    {
        return false;
    }
    if (m_normalizeCoef != other.m_normalizeCoef)
    {
        return false;
    }
    if (m_whiteMargins != other.m_whiteMargins)
    {
        return false;
    }

    return true;
}

bool
ColorGrayscaleOptions::operator!=(ColorGrayscaleOptions const& other) const
{
    return !(*this == other);
}

} // namespace output
