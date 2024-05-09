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
        : m_wienerCoef(0.0),
          m_wienerWindowSize(5),
          m_knndCoef(0.0),
          m_knndRadius(5),
          m_cdespeckleCoef(0.0),
          m_cdespeckleRadius(5),
          m_blurCoef(0.0),
          m_blurWindowSize(5),
          m_screenCoef(0.0),
          m_screenWindowSize(10),
          m_curveCoef(0.5),
          m_sqrCoef(0.0),
          m_gravureCoef(0.0),
          m_gravureWindowSize(10),
          m_unPaperCoef(0.0),
          m_unPaperIters(5),
          m_normalizeCoef(0.5),
          m_whiteMargins(false),
          m_grayScale(false) {}

    ColorGrayscaleOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double wienerCoef() const
    {
        return m_wienerCoef;
    }
    void setWienerCoef(double val)
    {
        m_wienerCoef = val;
    }
    int wienerWindowSize() const
    {
        return m_wienerWindowSize;
    }
    void setWienerWindowSize(int val)
    {
        m_wienerWindowSize = val;
    }

    double knndCoef() const
    {
        return m_knndCoef;
    }
    void setKnndCoef(double val)
    {
        m_knndCoef = val;
    }
    int knndRadius() const
    {
        return m_knndRadius;
    }
    void setKnndRadius(int val)
    {
        m_knndRadius = val;
    }

    double cdespeckleCoef() const
    {
        return m_cdespeckleCoef;
    }
    void setCdespeckleCoef(double val)
    {
        m_cdespeckleCoef = val;
    }
    int cdespeckleRadius() const
    {
        return m_cdespeckleRadius;
    }
    void setCdespeckleRadius(int val)
    {
        m_cdespeckleRadius = val;
    }

    double blurCoef() const
    {
        return m_blurCoef;
    }
    void setBlurCoef(double val)
    {
        m_blurCoef = val;
    }
    int blurWindowSize() const
    {
        return m_blurWindowSize;
    }
    void setBlurWindowSize(int val)
    {
        m_blurWindowSize = val;
    }

    double screenCoef() const
    {
        return m_screenCoef;
    }
    void setScreenCoef(double val)
    {
        m_screenCoef = val;
    }
    int screenWindowSize() const
    {
        return m_screenWindowSize;
    }
    void setScreenWindowSize(int val)
    {
        m_screenWindowSize = val;
    }

    double curveCoef() const
    {
        return m_curveCoef;
    }
    void setCurveCoef(double val)
    {
        m_curveCoef = val;
    }

    double sqrCoef() const
    {
        return m_sqrCoef;
    }
    void setSqrCoef(double val)
    {
        m_sqrCoef = val;
    }

    double gravureCoef() const
    {
        return m_gravureCoef;
    }
    void setGravureCoef(double val)
    {
        m_gravureCoef = val;
    }
    int gravureWindowSize() const
    {
        return m_gravureWindowSize;
    }
    void setGravureWindowSize(int val)
    {
        m_gravureWindowSize = val;
    }

    double unPaperCoef() const
    {
        return m_unPaperCoef;
    }
    void setUnPaperCoef(double val)
    {
        m_unPaperCoef = val;
    }
    int unPaperIters() const
    {
        return m_unPaperIters;
    }
    void setUnPaperIters(int val)
    {
        m_unPaperIters = val;
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

    bool getflgGrayScale() const
    {
        return m_grayScale;
    }
    void setflgGrayScale(bool val)
    {
        m_grayScale = val;
    }

    bool operator==(ColorGrayscaleOptions const& other) const;

    bool operator!=(ColorGrayscaleOptions const& other) const;

private:
    double m_wienerCoef;
    int m_wienerWindowSize;
    double m_knndCoef;
    int m_knndRadius;
    double m_cdespeckleCoef;
    int m_cdespeckleRadius;
    double m_blurCoef;
    int m_blurWindowSize;
    double m_screenCoef;
    int m_screenWindowSize;
    double m_curveCoef;
    double m_sqrCoef;
    double m_gravureCoef;
    int m_gravureWindowSize;
    double m_unPaperCoef;
    int m_unPaperIters;
    double m_normalizeCoef;
    bool m_whiteMargins;
    bool m_grayScale;
};

} // namespace output

#endif
