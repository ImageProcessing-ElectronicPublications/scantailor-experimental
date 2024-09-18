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

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include "DewarpingParams.h"
#include "dewarping/DistortionModel.h"
#include "dewarping/DepthPerception.h"

namespace deskew
{

DewarpingParams::DewarpingParams()
    :   m_mode(MODE_AUTO)
{
}

DewarpingParams::DewarpingParams(QDomElement const& el)
    :   m_distortionModel(el.namedItem("distortion-model").toElement())
    ,   m_depthPerception(el.attribute("depthPerception"))
    ,   m_mode(el.attribute("mode") == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
{
}

DewarpingParams::~DewarpingParams()
{
}

bool
DewarpingParams::isValid() const
{
    return m_distortionModel.isValid();
}

void
DewarpingParams::invalidate()
{
    *this = DewarpingParams();
}

QDomElement
DewarpingParams::toXml(QDomDocument& doc, QString const& name) const
{
    if (!isValid())
    {
        return QDomElement();
    }

    QDomElement el(doc.createElement(name));
    el.appendChild(m_distortionModel.toXml(doc, "distortion-model"));
    el.setAttribute("depthPerception", m_depthPerception.toString());
    el.setAttribute("mode", m_mode == MODE_MANUAL ? "manual" : "auto");
    return el;
}

double
DewarpingParams::getAngle() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = m_distortionModel.topCurve().polyline().front();
        QPointF point_tr = m_distortionModel.topCurve().polyline().back();
        QPointF point_bl = m_distortionModel.bottomCurve().polyline().front();
        QPointF point_br = m_distortionModel.bottomCurve().polyline().back();
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
DewarpingParams::getAngleOblique() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = m_distortionModel.topCurve().polyline().front();
        QPointF point_tr = m_distortionModel.topCurve().polyline().back();
        QPointF point_bl = m_distortionModel.bottomCurve().polyline().front();
        QPointF point_br = m_distortionModel.bottomCurve().polyline().back();
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
DewarpingParams::getAngleHor() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = m_distortionModel.topCurve().polyline().front();
        QPointF point_tr = m_distortionModel.topCurve().polyline().back();
        QPointF point_bl = m_distortionModel.bottomCurve().polyline().front();
        QPointF point_br = m_distortionModel.bottomCurve().polyline().back();
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
DewarpingParams::getAngleVert() const
{
    double angle = 0.0;
    if (isValid())
    {
        QPointF point_tl = m_distortionModel.topCurve().polyline().front();
        QPointF point_tr = m_distortionModel.topCurve().polyline().back();
        QPointF point_bl = m_distortionModel.bottomCurve().polyline().front();
        QPointF point_br = m_distortionModel.bottomCurve().polyline().back();
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
