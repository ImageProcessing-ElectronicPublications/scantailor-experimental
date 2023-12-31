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

#ifndef XMLUNMARSHALLER_H_
#define XMLUNMARSHALLER_H_

#include "foundation_config.h"

class QString;
class QDomElement;
class QSize;
class QSizeF;
class RelativeMargins;
class QPointF;
class QLineF;
class QRect;
class QRectF;
class QPolygonF;

class FOUNDATION_EXPORT XmlUnmarshaller
{
public:
    static QString string(QDomElement const& el);

    static QSize size(QDomElement const& el);

    static QSizeF sizeF(QDomElement const& el);

    static RelativeMargins relativeMargins(QDomElement const& el);

    static QPointF pointF(QDomElement const& el);

    static QLineF lineF(QDomElement const& el);

    static QRect rect(QDomElement const& el);

    static QRectF rectF(QDomElement const& el);

    static QPolygonF polygonF(QDomElement const& el);
};

#endif
