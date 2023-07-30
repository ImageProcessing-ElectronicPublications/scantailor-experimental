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

#ifndef OUTPUT_COLOR_GRAYSCALE_OPTIONS_H_
#define OUTPUT_COLOR_GRAYSCALE_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

class ColorGrayscaleOptions
{
public:
    ColorGrayscaleOptions()
        : m_whiteMargins(false),
          m_curveCoef(0.5),
          m_normalizeCoef(0.5) {}

    ColorGrayscaleOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double curveCoef() const
    {
        return m_curveCoef;
    }

    void setCurveCoef(double val)
    {
        m_curveCoef = val;
    }

    double normalizeCoef() const
    {
        return m_normalizeCoef;
    }

    void setNormalizeCoef(double val)
    {
        m_normalizeCoef = val;
    }

    bool whiteMargins() const
    {
        return m_whiteMargins;
    }

    void setWhiteMargins(bool val)
    {
        m_whiteMargins = val;
    }

    bool operator==(ColorGrayscaleOptions const& other) const;

    bool operator!=(ColorGrayscaleOptions const& other) const;
private:
    bool m_whiteMargins;
    double m_curveCoef;
    double m_normalizeCoef;
};

} // namespace output

#endif
