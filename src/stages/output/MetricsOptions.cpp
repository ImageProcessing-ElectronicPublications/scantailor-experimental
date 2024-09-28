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

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include "MetricsOptions.h"

namespace output
{

MetricsOptions::MetricsOptions()
    :   m_metricMSEfilters(0.0),
        m_metricBWorigin(0.0),
        m_metricBWfilters(0.0),
        m_metricBWthreshold(0.0),
        m_metricBWdestination(0.0)
{
}

MetricsOptions::MetricsOptions(QDomElement const& el)
    :   m_metricMSEfilters(el.attribute("MSEfilters").toDouble()),
        m_metricBWorigin(el.attribute("BWorigin").toDouble()),
        m_metricBWfilters(el.attribute("BWfilters").toDouble()),
        m_metricBWthreshold(el.attribute("BWthreshold").toDouble()),
        m_metricBWdestination(el.attribute("BWdestination").toDouble())
{}

QDomElement
MetricsOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    if (m_metricMSEfilters > 0.0)
    {
        el.setAttribute("MSEfilters", m_metricMSEfilters);
    }
    if (m_metricBWorigin > 0.0)
    {
        el.setAttribute("BWorigin", m_metricBWorigin);
    }
    if (m_metricBWfilters > 0.0)
    {
        el.setAttribute("BWfilters", m_metricBWfilters);
    }
    if (m_metricBWthreshold > 0.0)
    {
        el.setAttribute("BWthreshold", m_metricBWthreshold);
    }
    if (m_metricBWdestination > 0.0)
    {
        el.setAttribute("BWdestination", m_metricBWdestination);
    }
    return el;
}

bool
MetricsOptions::operator==(MetricsOptions const& other) const
{
    if (m_metricMSEfilters != other.m_metricMSEfilters)
    {
        return false;
    }
    if (m_metricBWorigin != other.m_metricBWorigin)
    {
        return false;
    }
    if (m_metricBWfilters != other.m_metricBWfilters)
    {
        return false;
    }
    if (m_metricBWthreshold != other.m_metricBWthreshold)
    {
        return false;
    }
    if (m_metricBWdestination != other.m_metricBWdestination)
    {
        return false;
    }

    return true;
}

} // namespace output
