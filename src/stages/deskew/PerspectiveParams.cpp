/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include <vector>
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include "PerspectiveParams.h"
#include "XmlMarshaller.h"
#include "XmlUnmarshaller.h"
#include "dewarping/DistortionModel.h"
#include "dewarping/Curve.h"

namespace deskew
{

PerspectiveParams::PerspectiveParams()
    :   m_mode(MODE_AUTO)
{
}

PerspectiveParams::PerspectiveParams(QDomElement const& el)
    :   m_mode(el.attribute("mode") == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
{
    m_corners[TOP_LEFT] = XmlUnmarshaller::pointF(el.namedItem("tl").toElement());
    m_corners[TOP_RIGHT] = XmlUnmarshaller::pointF(el.namedItem("tr").toElement());
    m_corners[BOTTOM_RIGHT] = XmlUnmarshaller::pointF(el.namedItem("br").toElement());
    m_corners[BOTTOM_LEFT] = XmlUnmarshaller::pointF(el.namedItem("bl").toElement());
}

bool
PerspectiveParams::isValid() const
{
    dewarping::DistortionModel distortion_model;
    distortion_model.setTopCurve(
        std::vector<QPointF> {m_corners[TOP_LEFT], m_corners[TOP_RIGHT]}
    );
    distortion_model.setBottomCurve(
        std::vector<QPointF> {m_corners[BOTTOM_LEFT], m_corners[BOTTOM_RIGHT]}
    );
    return distortion_model.isValid();
}

void
PerspectiveParams::invalidate()
{
    *this = PerspectiveParams();
}

QDomElement
PerspectiveParams::toXml(QDomDocument& doc, QString const& name) const
{
    if (!isValid())
    {
        return QDomElement();
    }

    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute("mode", m_mode == MODE_MANUAL ? "manual" : "auto");
    el.appendChild(marshaller.pointF(m_corners[TOP_LEFT], "tl"));
    el.appendChild(marshaller.pointF(m_corners[TOP_RIGHT], "tr"));
    el.appendChild(marshaller.pointF(m_corners[BOTTOM_RIGHT], "br"));
    el.appendChild(marshaller.pointF(m_corners[BOTTOM_LEFT], "bl"));
    return el;
}

double
PerspectiveParams::getAngle() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = corner(TOP_LEFT);
        QPointF point_tr = corner(TOP_RIGHT);
        QPointF point_bl = corner(BOTTOM_LEFT);
        QPointF point_br = corner(BOTTOM_RIGHT);
        QPointF point_l = point_tl + point_bl;
        QPointF point_r = point_tr + point_br;
        QPointF point_t = point_tl + point_tr;
        QPointF point_b = point_bl + point_br;
        point_l /= 2.0f;
        point_r /= 2.0f;
        point_t /= 2.0f;
        point_b /= 2.0f;
        QLineF line_lr(point_l, point_r);
        QLineF line_tb(point_t, point_b);
        double angle_lr = line_lr.angle();
        angle_lr -= (angle_lr > 180.0) ? 360.0 : 0.0;
        double angle_tb = line_tb.angle() + 90.0;
        angle_tb -= (angle_tb > 180.0) ? 360.0 : 0.0;
        angle = angle_lr + angle_tb;
        angle /= 2.0;
        angle += (angle < 0.0) ? 360.0 : 0.0;
        angle -= (angle > 360.0) ? 360.0 : 0.0;
        angle -= (angle > 180.0) ? 360.0 : 0.0;
    }

    return angle;
}

double
PerspectiveParams::getAngleOblique() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = corner(TOP_LEFT);
        QPointF point_tr = corner(TOP_RIGHT);
        QPointF point_bl = corner(BOTTOM_LEFT);
        QPointF point_br = corner(BOTTOM_RIGHT);
        QPointF point_l = point_tl + point_bl;
        QPointF point_r = point_tr + point_br;
        QPointF point_t = point_tl + point_tr;
        QPointF point_b = point_bl + point_br;
        point_l /= 2.0f;
        point_r /= 2.0f;
        point_t /= 2.0f;
        point_b /= 2.0f;
        QLineF line_lr(point_l, point_r);
        QLineF line_tb(point_t, point_b);
        double angle_lr = line_lr.angle();
        angle_lr -= (angle_lr > 180.0) ? 360.0 : 0.0;
        double angle_tb = line_tb.angle() + 90.0;
        angle_tb -= (angle_tb > 180.0) ? 360.0 : 0.0;
        angle = angle_tb - angle_lr;
        angle += (angle < 0.0) ? 360.0 : 0.0;
        angle -= (angle > 360.0) ? 360.0 : 0.0;
        angle -= (angle > 180.0) ? 360.0 : 0.0;
    }

    return angle;
}

double
PerspectiveParams::getAngleHor() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = corner(TOP_LEFT);
        QPointF point_tr = corner(TOP_RIGHT);
        QPointF point_bl = corner(BOTTOM_LEFT);
        QPointF point_br = corner(BOTTOM_RIGHT);
        QLineF line_t(point_tl, point_tr);
        QLineF line_b(point_bl, point_br);
        double angle_t = line_t.angle();
        double angle_b = line_b.angle();
        angle = angle_b - angle_t;
        angle += (angle < 0.0) ? 360.0 : 0.0;
        angle -= (angle > 360.0) ? 360.0 : 0.0;
        angle -= (angle > 180.0) ? 360.0 : 0.0;
    }

    return angle;
}

double
PerspectiveParams::getAngleVert() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = corner(TOP_LEFT);
        QPointF point_tr = corner(TOP_RIGHT);
        QPointF point_bl = corner(BOTTOM_LEFT);
        QPointF point_br = corner(BOTTOM_RIGHT);
        QLineF line_l(point_tl, point_bl);
        QLineF line_r(point_tr, point_br);
        double angle_l = line_l.angle();
        double angle_r = line_r.angle();
        angle = angle_r - angle_l;
        angle += (angle < 0.0) ? 360.0 : 0.0;
        angle -= (angle > 360.0) ? 360.0 : 0.0;
        angle -= (angle > 180.0) ? 360.0 : 0.0;
    }

    return angle;
}

} // namespace deskew
