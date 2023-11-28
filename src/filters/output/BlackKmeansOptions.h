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

#ifndef OUTPUT_BLACK_KMEANS_OPTIONS_H_
#define OUTPUT_BLACK_KMEANS_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

enum KmeansColorSpace { HSV, HSL, YCBCR };

class BlackKmeansOptions
{
public:
    BlackKmeansOptions();

    BlackKmeansOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    int kmeansCount() const
    {
        return m_kmeansCount;
    }
    void setKmeansCount(int val)
    {
        m_kmeansCount = val;
    }

    int kmeansValueStart() const
    {
        return m_kmeansValueStart;
    }
    void setKmeansValueStart(int val)
    {
        m_kmeansValueStart = val;
    }

    double kmeansSat() const
    {
        return m_kmeansSat;
    }
    void setKmeansSat(double val)
    {
        m_kmeansSat = val;
    }

    double kmeansNorm() const
    {
        return m_kmeansNorm;
    }
    void setKmeansNorm(double val)
    {
        m_kmeansNorm = val;
    }

    double kmeansBG() const
    {
        return m_kmeansBG;
    }
    void setKmeansBG(double val)
    {
        m_kmeansBG = val;
    }

    double coloredMaskCoef() const
    {
        return m_coloredMaskCoef;
    }
    void setColoredMaskCoef(double val)
    {
        m_coloredMaskCoef = val;
    }

    KmeansColorSpace kmeansColorSpace() const
    {
        return m_kmeansColorSpace;
    }
    void setKmeansColorSpace(KmeansColorSpace val)
    {
        m_kmeansColorSpace = val;
    }

    int kmeansMorphology() const
    {
        return m_kmeansMorphology;
    }
    void setKmeansMorphology(int val)
    {
        m_kmeansMorphology = val;
    }

    bool operator==(BlackKmeansOptions const& other) const;

    bool operator!=(BlackKmeansOptions const& other) const;
private:
    int m_kmeansCount;
    int m_kmeansValueStart;
    double m_kmeansSat;
    double m_kmeansNorm;
    double m_kmeansBG;
    double m_coloredMaskCoef;
    KmeansColorSpace m_kmeansColorSpace;
    int m_kmeansMorphology;

    static KmeansColorSpace parseKmeansColorSpace(QString const& str);

    static QString formatKmeansColorSpace(KmeansColorSpace type);
};

} // namespace output

#endif
