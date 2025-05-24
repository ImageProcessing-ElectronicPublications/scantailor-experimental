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
        : m_curveCoef(0.5)
        , m_sqrCoef(0.0)
        , m_RISundefectSize(2)
        , m_RISundefectCoef(0.0)
        , m_autoLevelSize(10)
        , m_autoLevelCoef(0.0)
        , m_balanceSize(23)
        , m_balanceCoef(0.0)
        , m_overblurSize(49)
        , m_overblurCoef(0.0)
        , m_retinexSize(31)
        , m_retinexCoef(0.0)
        , m_subtractbgSize(45)
        , m_subtractbgCoef(0.0)
        , m_equalizeSize(6)
        , m_equalizeCoef(0.0)
        , m_wienerSize(3)
        , m_wienerCoef(0.0)
        , m_knndRadius(7)
        , m_knndCoef(0.0)
        , m_emdRadius(9)
        , m_emdCoef(0.0)
        , m_cdespeckleRadius(2)
        , m_cdespeckleCoef(0.0)
        , m_sigmaSize(29)
        , m_sigmaCoef(0.0)
        , m_blurSize(1)
        , m_blurCoef(0.0)
        , m_screenSize(5)
        , m_screenCoef(0.0)
        , m_edgedivSize(13)
        , m_edgedivCoef(0.0)
        , m_robustSize(10)
        , m_robustCoef(0.0)
        , m_grainSize(15)
        , m_grainCoef(0.0)
        , m_comixSize(6)
        , m_comixCoef(0.0)
        , m_gravureSize(15)
        , m_gravureCoef(0.0)
        , m_dots8Size(17)
        , m_dots8Coef(0.0)
        , m_unPaperIters(4)
        , m_unPaperCoef(0.0)
        , m_normalizeCoef(0.5)
        , m_whiteMargins(false)
        , m_grayScale(false)
    {}

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

    int RISundefectSize() const
    {
        return m_RISundefectSize;
    }
    void setRISundefectSize(int val)
    {
        m_RISundefectSize = val;
    }
    double RISundefectCoef() const
    {
        return m_RISundefectCoef;
    }
    void setRISundefectCoef(double val)
    {
        m_RISundefectCoef = val;
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

    int balanceSize() const
    {
        return m_balanceSize;
    }
    void setBalanceSize(int val)
    {
        m_balanceSize = val;
    }
    double balanceCoef() const
    {
        return m_balanceCoef;
    }
    void setBalanceCoef(double val)
    {
        m_balanceCoef = val;
    }

    int overblurSize() const
    {
        return m_overblurSize;
    }
    void setOverblurSize(int val)
    {
        m_overblurSize = val;
    }
    double overblurCoef() const
    {
        return m_overblurCoef;
    }
    void setOverblurCoef(double val)
    {
        m_overblurCoef = val;
    }

    int retinexSize() const
    {
        return m_retinexSize;
    }
    void setRetinexSize(int val)
    {
        m_retinexSize = val;
    }
    double retinexCoef() const
    {
        return m_retinexCoef;
    }
    void setRetinexCoef(double val)
    {
        m_retinexCoef = val;
    }

    int subtractbgSize() const
    {
        return m_subtractbgSize;
    }
    void setSubtractbgSize(int val)
    {
        m_subtractbgSize = val;
    }
    double subtractbgCoef() const
    {
        return m_subtractbgCoef;
    }
    void setSubtractbgCoef(double val)
    {
        m_subtractbgCoef = val;
    }

    int equalizeSize() const
    {
        return m_equalizeSize;
    }
    void setEqualizeSize(int val)
    {
        m_equalizeSize = val;
    }
    double equalizeCoef() const
    {
        return m_equalizeCoef;
    }
    void setEqualizeCoef(double val)
    {
        m_equalizeCoef = val;
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

    int emdRadius() const
    {
        return m_emdRadius;
    }
    void setEmdRadius(int val)
    {
        m_emdRadius = val;
    }
    double emdCoef() const
    {
        return m_emdCoef;
    }
    void setEmdCoef(double val)
    {
        m_emdCoef = val;
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

    int sigmaSize() const
    {
        return m_sigmaSize;
    }
    void setSigmaSize(int val)
    {
        m_sigmaSize = val;
    }
    double sigmaCoef() const
    {
        return m_sigmaCoef;
    }
    void setSigmaCoef(double val)
    {
        m_sigmaCoef = val;
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

    int robustSize() const
    {
        return m_robustSize;
    }
    void setRobustSize(int val)
    {
        m_robustSize = val;
    }
    double robustCoef() const
    {
        return m_robustCoef;
    }
    void setRobustCoef(double val)
    {
        m_robustCoef = val;
    }

    int grainSize() const
    {
        return m_grainSize;
    }
    void setGrainSize(int val)
    {
        m_grainSize = val;
    }
    double grainCoef() const
    {
        return m_grainCoef;
    }
    void setGrainCoef(double val)
    {
        m_grainCoef = val;
    }

    int comixSize() const
    {
        return m_comixSize;
    }
    void setComixSize(int val)
    {
        m_comixSize = val;
    }
    double comixCoef() const
    {
        return m_comixCoef;
    }
    void setComixCoef(double val)
    {
        m_comixCoef = val;
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
    int m_RISundefectSize;
    double m_RISundefectCoef;
    int m_autoLevelSize;
    double m_autoLevelCoef;
    int m_balanceSize;
    double m_balanceCoef;
    int m_overblurSize;
    double m_overblurCoef;
    int m_retinexSize;
    double m_retinexCoef;
    int m_subtractbgSize;
    double m_subtractbgCoef;
    int m_equalizeSize;
    double m_equalizeCoef;
    int m_wienerSize;
    double m_wienerCoef;
    int m_knndRadius;
    double m_knndCoef;
    int m_emdRadius;
    double m_emdCoef;
    int m_cdespeckleRadius;
    double m_cdespeckleCoef;
    int m_sigmaSize;
    double m_sigmaCoef;
    int m_blurSize;
    double m_blurCoef;
    int m_screenSize;
    double m_screenCoef;
    int m_edgedivSize;
    double m_edgedivCoef;
    int m_robustSize;
    double m_robustCoef;
    int m_grainSize;
    double m_grainCoef;
    int m_comixSize;
    double m_comixCoef;
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
