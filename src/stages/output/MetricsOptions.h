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

#ifndef OUTPUT_METRICS_OPTIONS_H_
#define OUTPUT_METRICS_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

class MetricsOptions
{
public:
    MetricsOptions();

    MetricsOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double getMetricMSEfilters() const
    {
        return m_metricMSEfilters;
    }
    void setMetricMSEfilters(double val)
    {
        m_metricMSEfilters = val;
    }

    double getMetricBWorigin() const
    {
        return m_metricBWorigin;
    }
    void setMetricBWorigin(double val)
    {
        m_metricBWorigin = val;
    }

    double getMetricBWfilters() const
    {
        return m_metricBWfilters;
    }
    void setMetricBWfilters(double val)
    {
        m_metricBWfilters = val;
    }

    double getMetricBWthreshold() const
    {
        return m_metricBWthreshold;
    }
    void setMetricBWthreshold(double val)
    {
        m_metricBWthreshold = val;
    }

    double getMetricBWdestination() const
    {
        return m_metricBWdestination;
    }
    void setMetricBWdestination(double val)
    {
        m_metricBWdestination = val;
    }

    bool operator==(MetricsOptions const& other) const;

    bool operator!=(MetricsOptions const& other) const;
private:
    double m_metricMSEfilters;
    double m_metricBWorigin;
    double m_metricBWfilters;
    double m_metricBWthreshold;
    double m_metricBWdestination;
};

} // namespace output

#endif
