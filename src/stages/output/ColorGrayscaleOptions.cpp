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
    :  m_curveCoef(el.attribute("curveCoef").toDouble()),
       m_sqrCoef(el.attribute("sqrCoef").toDouble()),
       m_wienerSize(el.attribute("wienerSize").toInt()),
       m_wienerCoef(el.attribute("wienerCoef").toDouble()),
       m_autoLevelSize(el.attribute("autoLevelSize").toInt()),
       m_autoLevelCoef(el.attribute("autoLevelCoef").toDouble()),
       m_knndRadius(el.attribute("knnDRadius").toInt()),
       m_knndCoef(el.attribute("knnDenoiser").toDouble()),
       m_cdespeckleRadius(el.attribute("cdespeckleRadius").toInt()),
       m_cdespeckleCoef(el.attribute("cdespeckle").toDouble()),
       m_blurSize(el.attribute("blurSize").toInt()),
       m_blurCoef(el.attribute("blurCoef").toDouble()),
       m_screenSize(el.attribute("screenSize").toInt()),
       m_screenCoef(el.attribute("screenCoef").toDouble()),
       m_edgedivSize(el.attribute("edgedivSize").toInt()),
       m_edgedivCoef(el.attribute("edgedivCoef").toDouble()),
       m_gravureSize(el.attribute("gravureSize").toInt()),
       m_gravureCoef(el.attribute("gravureCoef").toDouble()),
       m_dots8Size(el.attribute("dots8Size").toInt()),
       m_dots8Coef(el.attribute("dots8Coef").toDouble()),
       m_unPaperIters(el.attribute("unPaperIters").toInt()),
       m_unPaperCoef(el.attribute("unPaper").toDouble()),
       m_normalizeCoef(el.attribute("normalizeCoef").toDouble()),
       m_whiteMargins(el.attribute("whiteMargins") == "1"),
       m_grayScale(el.attribute("grayScale") == "1")
{
    if (m_curveCoef < -1.0 || m_curveCoef > 1.0)
    {
        m_curveCoef = 0.0;
    }
    if (m_sqrCoef < -1.0 || m_sqrCoef > 1.0)
    {
        m_sqrCoef = 0.0;
    }
    if (m_wienerSize < 1)
    {
        m_wienerSize = 3;
    }
    if (m_wienerCoef < 0.0 || m_wienerCoef > 1.0)
    {
        m_wienerCoef = 0.0;
    }
    if (m_autoLevelSize < 1)
    {
        m_autoLevelSize = 10;
    }
    if (m_autoLevelCoef < -1.0 || m_autoLevelCoef > 1.0)
    {
        m_autoLevelCoef = 0.0;
    }
    if (m_knndRadius < 1)
    {
        m_knndRadius = 7;
    }
    if (m_knndCoef < 0.0 || m_knndCoef > 1.0)
    {
        m_knndCoef = 0.0;
    }
    if (m_cdespeckleRadius < 1)
    {
        m_cdespeckleRadius = 2;
    }
    if (m_cdespeckleCoef < -1.0 || m_cdespeckleCoef > 1.0)
    {
        m_cdespeckleCoef = 0.0;
    }
    if (m_blurSize < 1)
    {
        m_blurSize = 1;
    }
    if (m_blurCoef < -2.0 || m_blurCoef > 1.0)
    {
        m_blurCoef = 0.0;
    }
    if (m_screenSize < 1)
    {
        m_screenSize = 5;
    }
    if (m_screenCoef < -1.0 || m_screenCoef > 1.0)
    {
        m_screenCoef = 0.0;
    }
    if (m_edgedivSize < 1)
    {
        m_edgedivSize = 13;
    }
    if (m_edgedivCoef < 0.0 || m_edgedivCoef > 1.0)
    {
        m_edgedivCoef = 0.0;
    }
    if (m_gravureSize < 1)
    {
        m_gravureSize = 15;
    }
    if (m_gravureCoef < -1.0 || m_gravureCoef > 1.0)
    {
        m_gravureCoef = 0.0;
    }
    if (m_dots8Size < 1)
    {
        m_dots8Size = 17;
    }
    if (m_dots8Coef < -1.0 || m_dots8Coef > 1.0)
    {
        m_dots8Coef = 0.0;
    }
    if (m_unPaperIters < 1)
    {
        m_unPaperIters = 4;
    }
    if (m_unPaperCoef < 0.0 || m_unPaperCoef > 1.0)
    {
        m_unPaperCoef = 0.0;
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
    el.setAttribute("curveCoef", m_curveCoef);
    el.setAttribute("sqrCoef", m_sqrCoef);
    el.setAttribute("wienerSize", m_wienerSize);
    el.setAttribute("wienerCoef", m_wienerCoef);
    el.setAttribute("autoLevelSize", m_autoLevelSize);
    el.setAttribute("autoLevelCoef", m_autoLevelCoef);
    el.setAttribute("knnDenoiser", m_knndCoef);
    el.setAttribute("knnDRadius", m_knndRadius);
    el.setAttribute("cdespeckle", m_cdespeckleCoef);
    el.setAttribute("cdespeckleRadius", m_cdespeckleRadius);
    el.setAttribute("blurSize", m_blurSize);
    el.setAttribute("blurCoef", m_blurCoef);
    el.setAttribute("screenSize", m_screenSize);
    el.setAttribute("screenCoef", m_screenCoef);
    el.setAttribute("edgedivSize", m_edgedivSize);
    el.setAttribute("edgedivCoef", m_edgedivCoef);
    el.setAttribute("gravureSize", m_gravureSize);
    el.setAttribute("gravureCoef", m_gravureCoef);
    el.setAttribute("dots8Size", m_dots8Size);
    el.setAttribute("dots8Coef", m_dots8Coef);
    el.setAttribute("unPaper", m_unPaperCoef);
    el.setAttribute("unPaperIters", m_unPaperIters);
    el.setAttribute("normalizeCoef", m_normalizeCoef);
    el.setAttribute("whiteMargins", m_whiteMargins ? "1" : "0");
    el.setAttribute("grayScale", m_grayScale ? "1" : "0");
    return el;
}

bool
ColorGrayscaleOptions::operator==(ColorGrayscaleOptions const& other) const
{
    if (m_curveCoef != other.m_curveCoef)
    {
        return false;
    }
    if (m_sqrCoef != other.m_sqrCoef)
    {
        return false;
    }
    if ((m_wienerCoef != other.m_wienerCoef) || (m_wienerSize != other.m_wienerSize))
    {
        return false;
    }
    if ((m_autoLevelCoef != other.m_autoLevelCoef) || (m_autoLevelSize != other.m_autoLevelSize))
    {
        return false;
    }
    if ((m_knndCoef != other.m_knndCoef) || (m_knndRadius != other.m_knndRadius))
    {
        return false;
    }
    if ((m_cdespeckleCoef != other.m_cdespeckleCoef) || (m_cdespeckleRadius != other.m_cdespeckleRadius))
    {
        return false;
    }
    if ((m_blurCoef != other.m_blurCoef) || (m_blurSize != other.m_blurSize))
    {
        return false;
    }
    if ((m_screenCoef != other.m_screenCoef) || (m_screenSize != other.m_screenSize))
    {
        return false;
    }
    if ((m_edgedivCoef != other.m_edgedivCoef) || (m_edgedivSize != other.m_edgedivSize))
    {
        return false;
    }
    if ((m_gravureCoef != other.m_gravureCoef) || (m_gravureSize != other.m_gravureSize))
    {
        return false;
    }
    if ((m_dots8Coef != other.m_dots8Coef) || (m_dots8Size != other.m_dots8Size))
    {
        return false;
    }
    if ((m_unPaperCoef != other.m_unPaperCoef) || (m_unPaperIters != other.m_unPaperIters))
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
    if (m_grayScale != other.m_grayScale)
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
