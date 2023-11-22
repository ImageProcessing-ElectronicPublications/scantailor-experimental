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

#include "BlackKmeansOptions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace output
{

BlackKmeansOptions::BlackKmeansOptions()
    :   m_kmeansCount(0),
        m_kmeansMorphology(0),
        m_kmeansSat(0.0),
        m_kmeansNorm(0.0),
        m_kmeansBG(0.0),
        m_coloredMaskCoef(0.0)
{
}

BlackKmeansOptions::BlackKmeansOptions(QDomElement const& el)
    :   m_kmeansCount(el.attribute("kmeans").toInt()),
        m_kmeansMorphology(el.attribute("kmeansMorphology").toInt()),
        m_kmeansSat(el.attribute("kmeansSat").toDouble()),
        m_kmeansNorm(el.attribute("kmeansNorm").toDouble()),
        m_kmeansBG(el.attribute("kmeansBG").toDouble()),
        m_coloredMaskCoef(el.attribute("coloredMaskCoef").toDouble())
{
    if (m_kmeansCount < 0)
    {
        m_kmeansCount = 0;
    }
    if (m_kmeansSat < 0.0 || m_kmeansSat > 1.0)
    {
        m_kmeansSat = 0.0;
    }
    if (m_kmeansNorm < 0.0 || m_kmeansNorm > 1.0)
    {
        m_kmeansNorm = 0.0;
    }
    if (m_kmeansBG < 0.0 || m_kmeansBG > 1.0)
    {
        m_kmeansBG = 0.0;
    }
    if (m_coloredMaskCoef < 0.0 || m_coloredMaskCoef > 1.0)
    {
        m_coloredMaskCoef = 0.0;
    }
}

QDomElement
BlackKmeansOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("kmeans", m_kmeansCount);
    el.setAttribute("kmeansMorphology", m_kmeansMorphology);
    el.setAttribute("kmeansSat", m_kmeansSat);
    el.setAttribute("kmeansNorm", m_kmeansNorm);
    el.setAttribute("kmeansBG", m_kmeansBG);
    el.setAttribute("coloredMaskCoef", m_coloredMaskCoef);
    return el;
}

bool
BlackKmeansOptions::operator==(BlackKmeansOptions const& other) const
{
    if (m_kmeansCount != other.m_kmeansCount)
    {
        return false;
    }
    if (m_kmeansMorphology != other.m_kmeansMorphology)
    {
        return false;
    }
    if (m_kmeansSat != other.m_kmeansSat)
    {
        return false;
    }
    if (m_kmeansNorm != other.m_kmeansNorm)
    {
        return false;
    }
    if (m_kmeansBG != other.m_kmeansBG)
    {
        return false;
    }
    if (m_coloredMaskCoef != other.m_coloredMaskCoef)
    {
        return false;
    }

    return true;
}

bool
BlackKmeansOptions::operator!=(BlackKmeansOptions const& other) const
{
    return !(*this == other);
}

} // namespace output
