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
    : m_curveCoef(el.attribute("curveCoef").toDouble())
    , m_sqrCoef(el.attribute("sqrCoef").toDouble())
    , m_autoLevelSize(el.attribute("autoLevelSize").toInt())
    , m_autoLevelCoef(el.attribute("autoLevelCoef").toDouble())
    , m_balanceSize(el.attribute("balanceSize").toInt())
    , m_balanceCoef(el.attribute("balanceCoef").toDouble())
    , m_overblurSize(el.attribute("overblurSize").toInt())
    , m_overblurCoef(el.attribute("overblurCoef").toDouble())
    , m_retinexSize(el.attribute("retinexSize").toInt())
    , m_retinexCoef(el.attribute("retinexCoef").toDouble())
    , m_subtractbgSize(el.attribute("subtractbgSize").toInt())
    , m_subtractbgCoef(el.attribute("subtractbgCoef").toDouble())
    , m_equalizeSize(el.attribute("equalizeSize").toInt())
    , m_equalizeCoef(el.attribute("equalizeCoef").toDouble())
    , m_wienerSize(el.attribute("wienerSize").toInt())
    , m_wienerCoef(el.attribute("wienerCoef").toDouble())
    , m_knndRadius(el.attribute("knnDRadius").toInt())
    , m_knndCoef(el.attribute("knnDenoiser").toDouble())
    , m_emdRadius(el.attribute("emDRadius").toInt())
    , m_emdCoef(el.attribute("emDenoiser").toDouble())
    , m_cdespeckleRadius(el.attribute("cdespeckleRadius").toInt())
    , m_cdespeckleCoef(el.attribute("cdespeckle").toDouble())
    , m_sigmaSize(el.attribute("sigmaSize").toInt())
    , m_sigmaCoef(el.attribute("sigmaCoef").toDouble())
    , m_blurSize(el.attribute("blurSize").toInt())
    , m_blurCoef(el.attribute("blurCoef").toDouble())
    , m_screenSize(el.attribute("screenSize").toInt())
    , m_screenCoef(el.attribute("screenCoef").toDouble())
    , m_edgedivSize(el.attribute("edgedivSize").toInt())
    , m_edgedivCoef(el.attribute("edgedivCoef").toDouble())
    , m_robustSize(el.attribute("robustSize").toInt())
    , m_robustCoef(el.attribute("robustCoef").toDouble())
    , m_grainSize(el.attribute("grainSize").toInt())
    , m_grainCoef(el.attribute("grainCoef").toDouble())
    , m_comixSize(el.attribute("comixSize").toInt())
    , m_comixCoef(el.attribute("comixCoef").toDouble())
    , m_gravureSize(el.attribute("gravureSize").toInt())
    , m_gravureCoef(el.attribute("gravureCoef").toDouble())
    , m_dots8Size(el.attribute("dots8Size").toInt())
    , m_dots8Coef(el.attribute("dots8Coef").toDouble())
    , m_unPaperIters(el.attribute("unPaperIters").toInt())
    , m_unPaperCoef(el.attribute("unPaper").toDouble())
    , m_normalizeCoef(el.attribute("normalizeCoef").toDouble())
    , m_whiteMargins(el.attribute("whiteMargins") == "1")
    , m_grayScale(el.attribute("grayScale") == "1")
{
    if (m_curveCoef < -1.0 || m_curveCoef > 1.0)
    {
        m_curveCoef = 0.0;
    }
    if (m_sqrCoef < -1.0 || m_sqrCoef > 1.0)
    {
        m_sqrCoef = 0.0;
    }
    if (m_autoLevelSize < 1)
    {
        m_autoLevelSize = 10;
    }
    if (m_autoLevelCoef < -1.0 || m_autoLevelCoef > 1.0)
    {
        m_autoLevelCoef = 0.0;
    }
    if (m_balanceSize < 1)
    {
        m_balanceSize = 23;
    }
    if (m_overblurSize < 1)
    {
        m_overblurSize = 49;
    }
    if (m_retinexSize < 1)
    {
        m_retinexSize = 31;
    }
    if (m_subtractbgSize < 1)
    {
        m_subtractbgSize = 45;
    }
    if (m_equalizeSize < 1)
    {
        m_equalizeSize = 6;
    }
    if (m_wienerSize < 1)
    {
        m_wienerSize = 3;
    }
    if (m_wienerCoef < 0.0 || m_wienerCoef > 1.0)
    {
        m_wienerCoef = 0.0;
    }
    if (m_knndRadius < 1)
    {
        m_knndRadius = 7;
    }
    if (m_knndCoef < 0.0 || m_knndCoef > 1.0)
    {
        m_knndCoef = 0.0;
    }
    if (m_emdRadius < 1)
    {
        m_emdRadius = 9;
    }
    if (m_emdCoef < 0.0 || m_emdCoef > 1.0)
    {
        m_emdCoef = 0.0;
    }
    if (m_cdespeckleRadius < 1)
    {
        m_cdespeckleRadius = 2;
    }
    if (m_cdespeckleCoef < -1.0 || m_cdespeckleCoef > 1.0)
    {
        m_cdespeckleCoef = 0.0;
    }
    if (m_sigmaSize < 1)
    {
        m_sigmaSize = 29;
    }
    if (m_sigmaCoef < -1.0 || m_sigmaCoef > 1.0)
    {
        m_sigmaCoef = 0.0;
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
    if (m_edgedivCoef < -1.0 || m_edgedivCoef > 1.0)
    {
        m_edgedivCoef = 0.0;
    }
    if (m_robustSize < 1)
    {
        m_robustSize = 10;
    }
    if (m_robustCoef < -1.0 || m_robustCoef > 1.0)
    {
        m_robustCoef = 0.0;
    }
    if (m_grainSize < 1)
    {
        m_grainSize = 15;
    }
    if (m_grainCoef < -1.0 || m_grainCoef > 1.0)
    {
        m_grainCoef = 0.0;
    }
    if (m_comixSize < 1)
    {
        m_comixSize = 6;
    }
    if (m_comixCoef < -1.0 || m_comixCoef > 1.0)
    {
        m_comixCoef = 0.0;
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
    if (m_autoLevelCoef != 0.0)
    {
        el.setAttribute("autoLevelSize", m_autoLevelSize);
        el.setAttribute("autoLevelCoef", m_autoLevelCoef);
    }
    if (m_balanceCoef != 0.0)
    {
        el.setAttribute("balanceSize", m_balanceSize);
        el.setAttribute("balanceCoef", m_balanceCoef);
    }
    if (m_overblurCoef != 0.0)
    {
        el.setAttribute("overblurSize", m_overblurSize);
        el.setAttribute("overblurCoef", m_overblurCoef);
    }
    if (m_retinexCoef != 0.0)
    {
        el.setAttribute("retinexSize", m_retinexSize);
        el.setAttribute("retinexCoef", m_retinexCoef);
    }
    if (m_subtractbgCoef != 0.0)
    {
        el.setAttribute("subtractbgSize", m_subtractbgSize);
        el.setAttribute("subtractbgCoef", m_subtractbgCoef);
    }
    if (m_equalizeCoef != 0.0)
    {
        el.setAttribute("equalizeSize", m_equalizeSize);
        el.setAttribute("equalizeCoef", m_equalizeCoef);
    }
    if (m_wienerCoef != 0.0)
    {
        el.setAttribute("wienerSize", m_wienerSize);
        el.setAttribute("wienerCoef", m_wienerCoef);
    }
    if (m_knndCoef != 0.0)
    {
        el.setAttribute("knnDenoiser", m_knndCoef);
        el.setAttribute("knnDRadius", m_knndRadius);
    }
    if (m_emdCoef != 0.0)
    {
        el.setAttribute("emDenoiser", m_emdCoef);
        el.setAttribute("emDRadius", m_emdRadius);
    }
    if (m_cdespeckleCoef != 0.0)
    {
        el.setAttribute("cdespeckle", m_cdespeckleCoef);
        el.setAttribute("cdespeckleRadius", m_cdespeckleRadius);
    }
    if (m_sigmaCoef != 0.0)
    {
        el.setAttribute("sigmaSize", m_sigmaSize);
        el.setAttribute("sigmaCoef", m_sigmaCoef);
    }
    if (m_blurCoef != 0.0)
    {
        el.setAttribute("blurSize", m_blurSize);
        el.setAttribute("blurCoef", m_blurCoef);
    }
    if (m_screenCoef != 0.0)
    {
        el.setAttribute("screenSize", m_screenSize);
        el.setAttribute("screenCoef", m_screenCoef);
    }
    if (m_edgedivCoef != 0.0)
    {
        el.setAttribute("edgedivSize", m_edgedivSize);
        el.setAttribute("edgedivCoef", m_edgedivCoef);
    }
    if (m_robustCoef != 0.0)
    {
        el.setAttribute("robustSize", m_robustSize);
        el.setAttribute("robustCoef", m_robustCoef);
    }
    if (m_grainCoef != 0.0)
    {
        el.setAttribute("grainSize", m_grainSize);
        el.setAttribute("grainCoef", m_grainCoef);
    }
    if (m_comixCoef != 0.0)
    {
        el.setAttribute("comixSize", m_comixSize);
        el.setAttribute("comixCoef", m_comixCoef);
    }
    if (m_gravureCoef != 0.0)
    {
        el.setAttribute("gravureSize", m_gravureSize);
        el.setAttribute("gravureCoef", m_gravureCoef);
    }
    if (m_dots8Coef != 0.0)
    {
        el.setAttribute("dots8Size", m_dots8Size);
        el.setAttribute("dots8Coef", m_dots8Coef);
    }
    if (m_unPaperCoef != 0.0)
    {
        el.setAttribute("unPaper", m_unPaperCoef);
        el.setAttribute("unPaperIters", m_unPaperIters);
    }
    el.setAttribute("normalizeCoef", m_normalizeCoef);
    if (m_whiteMargins)
    {
        el.setAttribute("whiteMargins", "1");
    }
    if (m_grayScale)
    {
        el.setAttribute("grayScale", "1");
    }
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
    if (m_autoLevelCoef != other.m_autoLevelCoef)
    {
        return false;
    }
    if (m_autoLevelSize != other.m_autoLevelSize)
    {
        return false;
    }
    if (m_balanceCoef != other.m_balanceCoef)
    {
        return false;
    }
    if (m_balanceSize != other.m_balanceSize)
    {
        return false;
    }
    if (m_overblurCoef != other.m_overblurCoef)
    {
        return false;
    }
    if (m_overblurSize != other.m_overblurSize)
    {
        return false;
    }
    if (m_retinexCoef != other.m_retinexCoef)
    {
        return false;
    }
    if (m_retinexSize != other.m_retinexSize)
    {
        return false;
    }
    if (m_subtractbgCoef != other.m_subtractbgCoef)
    {
        return false;
    }
    if (m_subtractbgSize != other.m_subtractbgSize)
    {
        return false;
    }
    if (m_equalizeCoef != other.m_equalizeCoef)
    {
        return false;
    }
    if (m_equalizeSize != other.m_equalizeSize)
    {
        return false;
    }
    if (m_wienerCoef != other.m_wienerCoef)
    {
        return false;
    }
    if (m_wienerSize != other.m_wienerSize)
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
    if (m_emdCoef != other.m_emdCoef)
    {
        return false;
    }
    if (m_emdRadius != other.m_emdRadius)
    {
        return false;
    }
    if (m_cdespeckleCoef != other.m_cdespeckleCoef)
    {
        return false;
    }
    if (m_cdespeckleRadius != other.m_cdespeckleRadius)
    {
        return false;
    }
    if (m_sigmaCoef != other.m_sigmaCoef)
    {
        return false;
    }
    if (m_sigmaSize != other.m_sigmaSize)
    {
        return false;
    }
    if (m_blurCoef != other.m_blurCoef)
    {
        return false;
    }
    if (m_blurSize != other.m_blurSize)
    {
        return false;
    }
    if (m_screenCoef != other.m_screenCoef)
    {
        return false;
    }
    if (m_screenSize != other.m_screenSize)
    {
        return false;
    }
    if (m_edgedivCoef != other.m_edgedivCoef)
    {
        return false;
    }
    if (m_edgedivSize != other.m_edgedivSize)
    {
        return false;
    }
    if (m_robustCoef != other.m_robustCoef)
    {
        return false;
    }
    if (m_robustSize != other.m_robustSize)
    {
        return false;
    }
    if (m_grainCoef != other.m_grainCoef)
    {
        return false;
    }
    if (m_grainSize != other.m_grainSize)
    {
        return false;
    }
    if (m_comixCoef != other.m_comixCoef)
    {
        return false;
    }
    if (m_comixSize != other.m_comixSize)
    {
        return false;
    }
    if (m_gravureCoef != other.m_gravureCoef)
    {
        return false;
    }
    if (m_gravureSize != other.m_gravureSize)
    {
        return false;
    }
    if (m_dots8Coef != other.m_dots8Coef)
    {
        return false;
    }
    if (m_dots8Size != other.m_dots8Size)
    {
        return false;
    }
    if (m_unPaperCoef != other.m_unPaperCoef)
    {
        return false;
    }
    if (m_unPaperIters != other.m_unPaperIters)
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
