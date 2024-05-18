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
        : m_curveCoef(0.5),
          m_sqrCoef(0.0),
          m_wienerSize(3),
          m_wienerCoef(0.0),
          m_autoLevelSize(10),
          m_autoLevelCoef(0.0),
          m_knndRadius(7),
          m_knndCoef(0.0),
          m_cdespeckleRadius(2),
          m_cdespeckleCoef(0.0),
          m_blurSize(1),
          m_blurCoef(0.0),
          m_screenSize(5),
          m_screenCoef(0.0),
          m_edgedivSize(13),
          m_edgedivCoef(0.0),
          m_gravureSize(15),
          m_gravureCoef(0.0),
          m_dots8Size(17),
          m_dots8Coef(0.0),
          m_unPaperIters(4),
          m_unPaperCoef(0.0),
          m_normalizeCoef(0.5),
          m_whiteMargins(false),
          m_grayScale(false) {}

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

    double sqrCoef() const
    {
        return m_sqrCoef;
    }
    void setSqrCoef(double val)
    {
        m_sqrCoef = val;
    }

    int wienerSize() const
    {
        return m_wienerSize;
    }
    void setWienerSize(int val)
    {
        m_wienerSize = val;
    }
   double wienerCoef() const
    {
        return m_wienerCoef;
    }
    void setWienerCoef(double val)
    {
        m_wienerCoef = val;
    }

    int autoLevelSize() const
    {
        return m_autoLevelSize;
    }
    void setAutoLevelSize(int val)
    {
        m_autoLevelSize = val;
    }
    double autoLevelCoef() const
    {
        return m_autoLevelCoef;
    }
    void setAutoLevelCoef(double val)
    {
        m_autoLevelCoef = val;
    }

    int knndRadius() const
    {
        return m_knndRadius;
    }
    void setKnndRadius(int val)
    {
        m_knndRadius = val;
    }
    double knndCoef() const
    {
        return m_knndCoef;
    }
    void setKnndCoef(double val)
    {
        m_knndCoef = val;
    }

    int cdespeckleRadius() const
    {
        return m_cdespeckleRadius;
    }
    void setCdespeckleRadius(int val)
    {
        m_cdespeckleRadius = val;
    }
    double cdespeckleCoef() const
    {
        return m_cdespeckleCoef;
    }
    void setCdespeckleCoef(double val)
    {
        m_cdespeckleCoef = val;
    }

    int blurSize() const
    {
        return m_blurSize;
    }
    void setBlurSize(int val)
    {
        m_blurSize = val;
    }
    double blurCoef() const
    {
        return m_blurCoef;
    }
    void setBlurCoef(double val)
    {
        m_blurCoef = val;
    }

    int screenSize() const
    {
        return m_screenSize;
    }
    void setScreenSize(int val)
    {
        m_screenSize = val;
    }
    double screenCoef() const
    {
        return m_screenCoef;
    }
    void setScreenCoef(double val)
    {
        m_screenCoef = val;
    }

    int edgedivSize() const
    {
        return m_edgedivSize;
    }
    void setEdgedivSize(int val)
    {
        m_edgedivSize = val;
    }
    double edgedivCoef() const
    {
        return m_edgedivCoef;
    }
    void setEdgedivCoef(double val)
    {
        m_edgedivCoef = val;
    }

    int gravureSize() const
    {
        return m_gravureSize;
    }
    void setGravureSize(int val)
    {
        m_gravureSize = val;
    }
    double gravureCoef() const
    {
        return m_gravureCoef;
    }
    void setGravureCoef(double val)
    {
        m_gravureCoef = val;
    }

    int dots8Size() const
    {
        return m_dots8Size;
    }
    void setDots8Size(int val)
    {
        m_dots8Size = val;
    }
    double dots8Coef() const
    {
        return m_dots8Coef;
    }
    void setDots8Coef(double val)
    {
        m_dots8Coef = val;
    }

    int unPaperIters() const
    {
        return m_unPaperIters;
    }
    void setUnPaperIters(int val)
    {
        m_unPaperIters = val;
    }
    double unPaperCoef() const
    {
        return m_unPaperCoef;
    }
    void setUnPaperCoef(double val)
    {
        m_unPaperCoef = val;
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
    double m_curveCoef;
    double m_sqrCoef;
    int m_wienerSize;
    double m_wienerCoef;
    int m_autoLevelSize;
    double m_autoLevelCoef;
    int m_knndRadius;
    double m_knndCoef;
    int m_cdespeckleRadius;
    double m_cdespeckleCoef;
    int m_blurSize;
    double m_blurCoef;
    int m_screenSize;
    double m_screenCoef;
    int m_edgedivSize;
    double m_edgedivCoef;
    int m_gravureSize;
    double m_gravureCoef;
    int m_dots8Size;
    double m_dots8Coef;
    int m_unPaperIters;
    double m_unPaperCoef;
    double m_normalizeCoef;
    bool m_whiteMargins;
    bool m_grayScale;
};

} // namespace output

#endif
