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

#ifndef OUTPUT_BLACK_WHITE_OPTIONS_H_
#define OUTPUT_BLACK_WHITE_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

enum ThresholdFilter { T_OTSU, T_MEANDELTA, T_DOTS8, T_BMTILED, T_NIBLACK, T_GATOS, T_SAUVOLA, T_WOLF, T_WINDOW, T_BRADLEY, T_NICK, T_GRAD, T_SINGH, T_FOX, T_WAN, T_EDGEPLUS, T_BLURDIV, T_EDGEDIV, T_EDGEADAPT, T_ROBUST, T_GRAIN, T_MSCALE, T_ENGRAVING };

class BlackWhiteOptions
{
public:
    BlackWhiteOptions();

    BlackWhiteOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    ThresholdFilter thresholdMethod() const
    {
        return m_thresholdMethod;
    }
    void setThresholdMethod(ThresholdFilter val)
    {
        m_thresholdMethod = val;
    }

    bool morphology() const
    {
        return m_morphology;
    }
    void setMorphology(bool val)
    {
        m_morphology = val;
    }

    bool negate() const
    {
        return m_negate;
    }
    void setNegate(bool val)
    {
        m_negate = val;
    }

    double dimmingColoredCoef() const
    {
        return m_dimmingColoredCoef;
    }
    void setDimmingColoredCoef(double val)
    {
        m_dimmingColoredCoef = val;
    }

    int thresholdAdjustment() const
    {
        return m_thresholdAdjustment;
    }
    void setThresholdAdjustment(int val)
    {
        m_thresholdAdjustment = val;
    }

    int getThresholdBoundLower() const
    {
        return m_boundLower;
    }
    void setThresholdBoundLower(int val)
    {
        if (val < m_boundUpper)
        {
            m_boundLower = val;
        }
    }

    int getThresholdBoundUpper() const
    {
        return m_boundUpper;
    }
    void setThresholdBoundUpper(int val)
    {
        if (val > m_boundLower)
        {
            m_boundUpper = val;
        }
    }

    int thresholdRadius() const
    {
        return m_thresholdRadius;
    }
    void setThresholdRadius(int val)
    {
        m_thresholdRadius = val;
    }

    double thresholdCoef() const
    {
        return m_thresholdCoef;
    }
    void setThresholdCoef(double val)
    {
        m_thresholdCoef = val;
    }

    double autoPictureCoef() const
    {
        return m_autoPictureCoef;
    }
    void setAutoPictureCoef(double val)
    {
        m_autoPictureCoef = val;
    }

    bool autoPictureOff() const
    {
        return m_autoPictureOff;
    }
    void setAutoPictureOff(bool val)
    {
        m_autoPictureOff = val;
    }

    bool pictureToDots8() const
    {
        return m_pictureToDots8;
    }
    void setPictureToDots8(bool val)
    {
        m_pictureToDots8 = val;
    }

    bool operator==(BlackWhiteOptions const& other) const;

    bool operator!=(BlackWhiteOptions const& other) const;
private:
    ThresholdFilter m_thresholdMethod;
    bool m_morphology;
    bool m_negate;
    double m_dimmingColoredCoef;
    int m_thresholdAdjustment;
    int m_boundLower;
    int m_boundUpper;
    int m_thresholdRadius;
    double m_thresholdCoef;
    double m_autoPictureCoef;
    bool m_autoPictureOff;
    bool m_pictureToDots8;

    static ThresholdFilter parseThresholdMethod(QString const& str);

    static QString formatThresholdMethod(ThresholdFilter type);
};

} // namespace output

#endif
