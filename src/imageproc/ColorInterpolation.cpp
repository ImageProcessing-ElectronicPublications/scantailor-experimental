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

#include "ColorInterpolation.h"
#include <QtGlobal>

namespace imageproc
{

QColor colorInterpolation(QColor const& from, QColor const& to, double dist)
{
    dist = qBound(0.0, dist, 1.0);

    qreal r1(from.redF()), g1(from.greenF()), b1(from.blueF()), a1(from.alphaF());
    qreal r2(to.redF()), g2(to.greenF()), b2(to.blueF()), a2(to.alphaF());

    qreal const r = r1 + (r2 - r1) * dist;
    qreal const g = g1 + (g2 - g1) * dist;
    qreal const b = b1 + (b2 - b1) * dist;
    qreal const a = a1 + (a2 - a1) * dist;

    return QColor::fromRgbF(r, g, b, a);
}

} // namespace imageproc
