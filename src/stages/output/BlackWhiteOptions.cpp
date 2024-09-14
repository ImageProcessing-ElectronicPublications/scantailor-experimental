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

#include "BlackWhiteOptions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace output
{

BlackWhiteOptions::BlackWhiteOptions()
    :   m_thresholdMethod(T_OTSU),
        m_morphology(true),
        m_negate(false),
        m_dimmingColoredCoef(0.0),
        m_thresholdAdjustment(0),
        m_boundLower(0),
        m_boundUpper(255),
        m_thresholdRadius(50),
        m_thresholdCoef(0.3),
        m_autoPictureCoef(0.0),
        m_autoPictureOff(false)
{
}

BlackWhiteOptions::BlackWhiteOptions(QDomElement const& el)
    :   m_thresholdMethod(parseThresholdMethod(el.attribute("thresholdMethod"))),
        m_morphology(el.attribute("morphology") == "1"),
        m_negate(el.attribute("negate") == "1"),
        m_dimmingColoredCoef(el.attribute("dimmingColoredCoef").toDouble()),
        m_thresholdAdjustment(el.attribute("thresholdAdj").toInt()),
        m_boundLower(el.attribute("boundLower").toInt()),
        m_boundUpper(el.attribute("boundUpper").toInt()),
        m_thresholdRadius(el.attribute("thresholdRadius").toInt()),
        m_thresholdCoef(el.attribute("thresholdCoef").toDouble()),
        m_autoPictureCoef(el.attribute("autoPictureCoef").toDouble()),
        m_autoPictureOff(el.attribute("autoPictureOff") == "1")
{
    if (m_dimmingColoredCoef < -1.0 || m_dimmingColoredCoef > 2.0)
    {
        m_dimmingColoredCoef = 0.0;
    }
    if (m_boundUpper <= m_boundLower)
    {
        m_boundLower = 0;
        m_boundUpper = 255;
    }
    if (m_thresholdRadius < 1)
    {
        m_thresholdRadius = 50;
    }
    if (m_thresholdCoef < 0.0)
    {
        m_thresholdCoef = 0.0;
    }
}

QDomElement
BlackWhiteOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("thresholdMethod", formatThresholdMethod(m_thresholdMethod));
    el.setAttribute("morphology", m_morphology ? "1" : "0");
    if (m_negate)
    {
        el.setAttribute("negate", "1");
    }
    if (m_dimmingColoredCoef != 0.0)
    {
        el.setAttribute("dimmingColoredCoef", m_dimmingColoredCoef);
    }
    if (m_thresholdAdjustment != 0)
    {
        el.setAttribute("thresholdAdj", m_thresholdAdjustment);
    }
    if (m_boundLower > 0)
    {
        el.setAttribute("boundLower", m_boundLower);
    }
    if (m_boundUpper < 255)
    {
        el.setAttribute("boundUpper", m_boundUpper);
    }
    el.setAttribute("thresholdRadius", m_thresholdRadius);
    el.setAttribute("thresholdCoef", m_thresholdCoef);
    if (m_autoPictureCoef != 0.0)
    {
        el.setAttribute("autoPictureCoef", m_autoPictureCoef);
    }
    if (m_autoPictureOff)
    {
        el.setAttribute("autoPictureOff", "1");
    }
    return el;
}

bool
BlackWhiteOptions::operator==(BlackWhiteOptions const& other) const
{
    if (m_thresholdMethod != other.m_thresholdMethod)
    {
        return false;
    }
    if (m_morphology != other.m_morphology)
    {
        return false;
    }
    if (m_negate != other.m_negate)
    {
        return false;
    }
    if (m_dimmingColoredCoef != other.m_dimmingColoredCoef)
    {
        return false;
    }
    if (m_thresholdAdjustment != other.m_thresholdAdjustment)
    {
        return false;
    }
    if (m_boundLower != other.m_boundLower)
    {
        return false;
    }
    if (m_boundUpper != other.m_boundUpper)
    {
        return false;
    }
    if (m_thresholdRadius != other.m_thresholdRadius)
    {
        return false;
    }
    if (m_thresholdCoef != other.m_thresholdCoef)
    {
        return false;
    }
    if (m_autoPictureCoef != other.m_autoPictureCoef)
    {
        return false;
    }
    if (m_autoPictureOff != other.m_autoPictureOff)
    {
        return false;
    }

    return true;
}

bool
BlackWhiteOptions::operator!=(BlackWhiteOptions const& other) const
{
    return !(*this == other);
}

ThresholdFilter
BlackWhiteOptions::parseThresholdMethod(QString const& str)
{
    if (str == "mean")
    {
        return T_MEANDELTA;
    }
    else if (str == "dots8")
    {
        return T_DOTS8;
    }
    else if (str == "niblack")
    {
        return T_NIBLACK;
    }
    else if (str == "gatos")
    {
        return T_GATOS;
    }
    else if (str == "sauvola")
    {
        return T_SAUVOLA;
    }
    else if (str == "wolf")
    {
        return T_WOLF;
    }
    else if (str == "bradley")
    {
        return T_BRADLEY;
    }
    else if (str == "grad")
    {
        return T_GRAD;
    }
    else if (str == "singh")
    {
        return T_SINGH;
    }
    else if (str == "WAN")
    {
        return T_WAN;
    }
    else if (str == "edgeplus")
    {
        return T_EDGEPLUS;
    }
    else if (str == "blurdiv")
    {
        return T_BLURDIV;
    }
    else if (str == "edgediv")
    {
        return T_EDGEDIV;
    }
    else if (str == "edgeadapt")
    {
        return T_EDGEADAPT;
    }
    else if (str == "robust")
    {
        return T_ROBUST;
    }
    else if (str == "multiscale")
    {
        return T_MSCALE;
    }
    else if (str == "engraving")
    {
        return T_ENGRAVING;
    }
    else
    {
        return T_OTSU;
    }
}

QString
BlackWhiteOptions::formatThresholdMethod(ThresholdFilter type)
{
    QString str = "";
    switch (type)
    {
    case T_OTSU:
        str = "otsu";
        break;
    case T_MEANDELTA:
        str = "mean";
        break;
    case T_DOTS8:
        str = "dots8";
        break;
    case T_NIBLACK:
        str = "niblack";
        break;
    case T_GATOS:
        str = "gatos";
        break;
    case T_SAUVOLA:
        str = "sauvola";
        break;
    case T_WOLF:
        str = "wolf";
        break;
    case T_BRADLEY:
        str = "bradley";
        break;
    case T_GRAD:
        str = "grad";
        break;
    case T_SINGH:
        str = "singh";
        break;
    case T_WAN:
        str = "WAN";
        break;
    case T_EDGEPLUS:
        str = "edgeplus";
        break;
    case T_BLURDIV:
        str = "blurdiv";
        break;
    case T_EDGEDIV:
        str = "edgediv";
        break;
    case T_EDGEADAPT:
        str = "edgeadapt";
        break;
    case T_ROBUST:
        str = "robust";
        break;
    case T_MSCALE:
        str = "multiscale";
        break;
    case T_ENGRAVING:
        str = "engraving";
        break;
    }
    return str;
}

} // namespace output
