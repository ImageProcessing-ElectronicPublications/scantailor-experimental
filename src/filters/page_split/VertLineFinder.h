/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef PAGE_SPLIT_VERTLINEFINDER_H_
#define PAGE_SPLIT_VERTLINEFINDER_H_

#include "acceleration/AcceleratableOperations.h"
#include <QPointF>
#include <QImage>
#include <QTransform>
#include <vector>
#include <memory>

class QLineF;
class DebugImages;

namespace imageproc
{
class GrayImage;
class AffineTransformedImage;
}

namespace page_split
{

class VertLineFinder
{
public:
    static std::vector<QLineF> findLines(
        imageproc::AffineTransformedImage const& image,
        std::shared_ptr<AcceleratableOperations> const& accel_ops,
        int max_lines, DebugImages* dbg = nullptr);
private:
    class QualityLine
    {
    public:
        QualityLine(
            QPointF const& top, QPointF const& bottom,
            unsigned quality);

        QPointF const& left() const
        {
            return m_left;
        }

        QPointF const& right() const
        {
            return m_right;
        }

        unsigned quality() const
        {
            return m_quality;
        }

        QLineF toQLine() const;
    private:
        QPointF m_left;
        QPointF m_right;
        unsigned m_quality;
    };

    class LineGroup
    {
    public:
        LineGroup(QualityLine const& line);

        bool belongsHere(QualityLine const& line) const;

        void add(QualityLine const& line);

        void merge(LineGroup const& other);

        QualityLine const& leader() const
        {
            return m_leader;
        }
    private:
        QualityLine m_leader;
        double m_left;
        double m_right;
    };

    static imageproc::GrayImage removeDarkVertBorders(imageproc::GrayImage const& src);

    static void selectVertBorders(imageproc::GrayImage& image);

    static void buildWeightTable(unsigned weight_table[]);
};

} // namespace page_split

#endif
