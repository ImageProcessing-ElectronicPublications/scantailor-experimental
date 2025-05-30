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

#include <cmath>
#include <cassert>
#include <stdexcept>
#include <string>
#include <boost/foreach.hpp>
#include <Eigen/Core>
#include <Eigen/QR>
#include <QLineF>
#include <QtGlobal>
#include <QDebug>
#include <boost/foreach.hpp>
#include <algorithm>
#include "CylindricalSurfaceDewarper.h"
#include "ToLineProjector.h"
#include "NumericTraits.h"
#include "ToVec.h"
#include "ToPoint.h"
#include "../foundation/MultipleTargetsSupport.h"

/*
Naming conventions:
img: Coordinates in the warped image.
pln: Coordinates on a plane where the 4 corner points of the curved
     quadrilateral are supposed to lie. In our model we assume that
     all 4 lie on the same plane. The corner points are mapped to
     the following points on the plane:
     * Start point of curve1 [top curve]: (0, 0)
     * End point of curve1 [top curve]: (1, 0)
     * Start point of curve2 [bottom curve]: (0, 1)
     * End point of curve2 [bottom curve]: (1, 1)
     pln and img coordinates are linked by a 2D homography,
     namely m_pln2img and m_img2pln.
crv: Dewarped normalized coordinates. crv X coordinates are linked
     to pln X ccoordinates through m_arcLengthMapper while the Y
     coordinates are linked by a one dimensional homography that's
     different for each generatrix.
*/

using namespace Eigen;

namespace dewarping
{

class CylindricalSurfaceDewarper::CoupledPolylinesIterator
{
public:
    CoupledPolylinesIterator(
        std::vector<QPointF> const& img_directrix1,
        std::vector<QPointF> const& img_directrix2,
        HomographicTransform<2, double> const& pln2img,
        HomographicTransform<2, double> const& img2pln);

    bool next(QPointF& img_pt1, QPointF& img_pt2, double& pln_x);
private:
    void next1(QPointF& img_pt1, QPointF& img_pt2, double& pln_x);

    void next2(QPointF& img_pt1, QPointF& img_pt2, double& pln_x);

    void advance1();

    void advance2();

    std::vector<QPointF>::const_iterator m_seq1It;
    std::vector<QPointF>::const_iterator m_seq2It;
    std::vector<QPointF>::const_iterator m_seq1End;
    std::vector<QPointF>::const_iterator m_seq2End;
    HomographicTransform<2, double> m_pln2img;
    HomographicTransform<2, double> m_img2pln;
    QPointF m_prevImgPt1;
    QPointF m_prevImgPt2;
    QPointF m_nextImgPt1;
    QPointF m_nextImgPt2;
    double m_nextPlnX1;
    double m_nextPlnX2;
};

CylindricalSurfaceDewarper::CylindricalSurfaceDewarper(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2,
    double depth_perception,
    double curve_correct,
    double curve_angle)
    : m_pln2img(calcPlnToImgHomography(img_directrix1, img_directrix2))
    , m_img2pln(m_pln2img.inv())
    , m_depthPerception(depth_perception)
    , m_curveCorrect(curve_correct)
    , m_curveAngle(curve_angle)
    , m_plnStraightLineY(
          calcPlnStraightLineY(img_directrix1, img_directrix2, m_pln2img, m_img2pln)
      )
    , m_directrixArcLength(1.0)
    , m_imgDirectrix1Intersector(img_directrix1)
    , m_imgDirectrix2Intersector(img_directrix2)
{
    initArcLengthMapper(img_directrix1, img_directrix2);
}

CylindricalSurfaceDewarper::Generatrix
CylindricalSurfaceDewarper::mapGeneratrix(double crv_x, State& state) const
{
    double const pln_x = m_arcLengthMapper.arcLenToX(crv_x, state.m_arcLengthHint);

    double const lin_y1 = -0.05 * (m_curveAngle - 2.0);
    double const lin_y2 = -lin_y1;
    double const lin_y = lin_y1 + (lin_y2 - lin_y1) * pln_x;

    Vector2d const pln_top_pt(pln_x, 0);
    Vector2d const pln_bottom_pt(pln_x, 1);
    QPointF const img_top_pt(toPoint(m_pln2img(pln_top_pt)));
    QPointF const img_bottom_pt(toPoint(m_pln2img(pln_bottom_pt)));
    QLineF const img_generatrix(img_top_pt, img_bottom_pt);
    ToLineProjector const projector(img_generatrix);
    QPointF const img_directrix1_pt(
        m_imgDirectrix1Intersector.intersect(img_generatrix, state.m_intersectionHint1)
    );
    QPointF const img_directrix2_pt(
        m_imgDirectrix2Intersector.intersect(img_generatrix, state.m_intersectionHint2)
    );
    double const img_directrix1_proj(projector.projectionScalar(img_directrix1_pt));
    double const img_directrix2_proj(projector.projectionScalar(img_directrix2_pt));

    double const pln_straight_line_delta = 2.0 * m_plnStraightLineY - 1.0;
    double const pln_straight_line_delta2 = pln_straight_line_delta * pln_straight_line_delta;
    double const pln_straight_line_frac = (pln_straight_line_delta2 > 1.0) ? 1.0 : pln_straight_line_delta2;
    double const pln_straight_line_y = pln_straight_line_frac * 0.5
                                       + (1.0 - pln_straight_line_frac) * m_plnStraightLineY;
    double const img_directrix12f_proj = (1.0 - pln_straight_line_y) * img_directrix1_proj
                                         + pln_straight_line_y * img_directrix2_proj;
    double const img_directrix12fd_proj = img_directrix12f_proj - pln_straight_line_y;
    //double const curve_coef = 1.0 + 0.5 * (m_curveCorrect - 2.0);
    double const curve_coef = (m_curveCorrect < 2.0) ? (1.0 / (3.0 - m_curveCorrect)) : (m_curveCorrect - 1.0);
    double const img_directrix12fds_proj = img_directrix12fd_proj * curve_coef + lin_y;
    double const img_directrix12fs_proj = img_directrix12fds_proj + pln_straight_line_y;

    QPointF const img_straight_line_pt(toPoint(m_pln2img(Vector2d(pln_x, img_directrix12fs_proj))));
    double const img_straight_line_proj(projector.projectionScalar(img_straight_line_pt));

    boost::array<std::pair<double, double>, 3> pairs;
    pairs[0] = std::make_pair(0.0, img_directrix1_proj);
    pairs[1] = std::make_pair(1.0, img_directrix2_proj);
    pairs[2] = std::make_pair(pln_straight_line_y, img_straight_line_proj);

    HomographicTransform<1, double> H(threePoint1DHomography(pairs));

    return Generatrix(img_generatrix, H);
}

QPointF
CylindricalSurfaceDewarper::mapToDewarpedSpace(QPointF const& img_pt) const
{
    State state;
    return mapToDewarpedSpace(img_pt, state);
}

QPointF
CylindricalSurfaceDewarper::mapToDewarpedSpace(QPointF const& img_pt, State& state) const
{
    double const pln_x = m_img2pln(toVec(img_pt))[0];
    double const crv_x = m_arcLengthMapper.xToArcLen(pln_x, state.m_arcLengthHint);

    double const lin_y1 = -0.05 * (m_curveAngle - 2.0);
    double const lin_y2 = -lin_y1;
    double const lin_y = lin_y1 + (lin_y2 - lin_y1) * pln_x;

    Vector2d const pln_top_pt(pln_x, 0);
    Vector2d const pln_bottom_pt(pln_x, 1);
    QPointF const img_top_pt(toPoint(m_pln2img(pln_top_pt)));
    QPointF const img_bottom_pt(toPoint(m_pln2img(pln_bottom_pt)));
    QLineF const img_generatrix(img_top_pt, img_bottom_pt);
    ToLineProjector const projector(img_generatrix);
    QPointF const img_directrix1_pt(
        m_imgDirectrix1Intersector.intersect(img_generatrix, state.m_intersectionHint1)
    );
    QPointF const img_directrix2_pt(
        m_imgDirectrix2Intersector.intersect(img_generatrix, state.m_intersectionHint2)
    );
    double const img_directrix1_proj(projector.projectionScalar(img_directrix1_pt));
    double const img_directrix2_proj(projector.projectionScalar(img_directrix2_pt));

    double const pln_straight_line_delta = 2.0 * m_plnStraightLineY - 1.0;
    double const pln_straight_line_delta2 = pln_straight_line_delta * pln_straight_line_delta;
    double const pln_straight_line_frac = (pln_straight_line_delta2 > 1.0) ? 1.0 : pln_straight_line_delta2;
    double const pln_straight_line_y = pln_straight_line_frac * 0.5
                                       + (1.0 - pln_straight_line_frac) * m_plnStraightLineY;
    double const img_directrix12f_proj = (1.0 - pln_straight_line_y) * img_directrix1_proj
                                         + pln_straight_line_y * img_directrix2_proj;
    double const img_directrix12fd_proj = img_directrix12f_proj - pln_straight_line_y;
    //double const curve_coef = 1.0 + 0.5 * (m_curveCorrect - 2.0);
    double const curve_coef = (m_curveCorrect < 2.0) ? (1.0 / (3.0 - m_curveCorrect)) : (m_curveCorrect - 1.0);
    double const img_directrix12fds_proj = img_directrix12fd_proj * curve_coef + lin_y;
    double const img_directrix12fs_proj = img_directrix12fds_proj + pln_straight_line_y;

    QPointF const img_straight_line_pt(toPoint(m_pln2img(Vector2d(pln_x, img_directrix12fs_proj))));
    double const img_straight_line_proj(projector.projectionScalar(img_straight_line_pt));

    boost::array<std::pair<double, double>, 3> pairs;
    pairs[0] = std::make_pair(img_directrix1_proj, 0.0);
    pairs[1] = std::make_pair(img_directrix2_proj, 1.0);
    pairs[2] = std::make_pair(img_straight_line_proj, pln_straight_line_y);

    HomographicTransform<1, double> const H(threePoint1DHomography(pairs));

    double const img_pt_proj(projector.projectionScalar(img_pt));
    double const crv_y = H(img_pt_proj);

    return QPointF(crv_x, crv_y);
}

QPointF
CylindricalSurfaceDewarper::mapToWarpedSpace(QPointF const& crv_pt) const
{
    State state;
    Generatrix const gtx(mapGeneratrix(crv_pt.x(), state));
    return gtx.imgLine.pointAt(gtx.pln2img(crv_pt.y()));
}

HomographicTransform<2, double>
CylindricalSurfaceDewarper::calcPlnToImgHomography(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2)
{
    QPointF const tl = img_directrix1.front();
    QPointF const tr = img_directrix1.back();
    QPointF const bl = img_directrix2.front();
    QPointF const br = img_directrix2.back();

    boost::array<std::pair<QPointF, QPointF>, 4> pairs;
    pairs[0] = std::make_pair(QPointF(0, 0), tl);
    pairs[1] = std::make_pair(QPointF(1, 0), tr);
    pairs[2] = std::make_pair(QPointF(0, 1), bl);
    pairs[3] = std::make_pair(QPointF(1, 1), br);

    return fourPoint2DHomography(pairs);
}

double
CylindricalSurfaceDewarper::calcPlnStraightLineY(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2,
    HomographicTransform<2, double> const pln2img,
    HomographicTransform<2, double> const img2pln)
{
    double pln_y_accum = 0;
    double weight_accum = 0;

    CoupledPolylinesIterator it(img_directrix1, img_directrix2, pln2img, img2pln);
    QPointF img_curve1_pt;
    QPointF img_curve2_pt;
    double pln_x;
    while (it.next(img_curve1_pt, img_curve2_pt, pln_x))
    {
        QLineF const img_generatrix(img_curve1_pt, img_curve2_pt);
        QPointF const img_line1_pt(toPoint(pln2img(Vector2d(pln_x, 0))));
        QPointF const img_line2_pt(toPoint(pln2img(Vector2d(pln_x, 1))));
        ToLineProjector const projector(img_generatrix);
        double const p1 = 0;
        double const p2 = projector.projectionScalar(img_line1_pt);
        double const p3 = projector.projectionScalar(img_line2_pt);
        double const p4 = 1;
        double const dp1 = p2 - p1;
        double const dp2 = p4 - p3;
        double const weight = fabs(dp1 + dp2);
        if (weight < 0.01)
        {
            continue;
        }

        double const p0 = (p3 * dp1 + p2 * dp2) / (dp1 + dp2);
        Vector2d const img_pt(toVec(img_generatrix.pointAt(p0)));
        pln_y_accum += img2pln(img_pt)[1] * weight;
        weight_accum += weight;
    }

    return ((weight_accum == 0) ? 0.5 : (pln_y_accum / weight_accum));
}

HomographicTransform<2, double>
CylindricalSurfaceDewarper::fourPoint2DHomography(
    boost::array<std::pair<QPointF, QPointF>, 4> const& pairs)
{
    Matrix<double, 8, 8> A;
    Matrix<double, 8, 1> b;
    int i = 0;

    typedef std::pair<QPointF, QPointF> Pair;
    BOOST_FOREACH(Pair const& pair, pairs)
    {
        QPointF const from(pair.first);
        QPointF const to(pair.second);

        A.row(i) << -from.x(), -from.y(), -1, 0, 0, 0, to.x()*from.x(), to.x()*from.y();
        b[i] = -to.x();
        ++i;

        A.row(i) << 0, 0, 0, -from.x(), -from.y(), -1, to.y()*from.x(), to.y()*from.y();
        b[i] = -to.y();
        ++i;
    }

    auto qr = A.colPivHouseholderQr();
    if (!qr.isInvertible())
    {
        throw std::runtime_error("Failed to build 2D homography");
    }

    Matrix<double, 8, 1> const h(qr.solve(b));
    Matrix3d H;
    H << h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], 1.0;

    return HomographicTransform<2, double>(H);
}

HomographicTransform<1, double>
CylindricalSurfaceDewarper::threePoint1DHomography(
    boost::array<std::pair<double, double>, 3> const& pairs)
{
    Matrix<double, 3, 3> A;
    Matrix<double, 3, 1> b;
    int i = 0;

    typedef std::pair<double, double> Pair;
    BOOST_FOREACH(Pair const& pair, pairs)
    {
        double const from = pair.first;
        double const to = pair.second;

        A.row(i) << -from, -1, from * to;
        b[i] = -to;
        ++i;
    }

    auto qr = A.colPivHouseholderQr();
    if (!qr.isInvertible())
    {
        throw std::runtime_error("Failed to build curves homography");
    }

    Matrix<double, 3, 1> const h(qr.solve(b));
    Matrix2d H;
    H << h[0], h[1], h[2], 1.0;

    return HomographicTransform<1, double>(H);
}

void
CylindricalSurfaceDewarper::initArcLengthMapper(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2)
{
    CoupledPolylinesIterator it(img_directrix1, img_directrix2, m_pln2img, m_img2pln);
    QPointF img_curve1_pt;
    QPointF img_curve2_pt;
    double prev_pln_x = NumericTraits<double>::min();
    double pln_x;

    Matrix<double, 3, 3> const& coeff = m_pln2img.mat();
    double const cm0 = coeff(2, 0) * coeff(2, 0);
    double const cm1 = coeff(2, 1) * coeff(2, 1);
    double const cnorm = cm0 + cm1;
    double const coeff_h_w = (cnorm > 0.0) ? (cm1 / cnorm) : 1.0;

    while (it.next(img_curve1_pt, img_curve2_pt, pln_x))
    {
        if (pln_x <= prev_pln_x)
        {
            // This means our surface has an S-like shape.
            // We don't support that, and to make ReverseArcLength happy,
            // we have to skip such points.
            continue;
        }

        QLineF const img_generatrix(img_curve1_pt, img_curve2_pt);
        QPointF const img_line1_pt(toPoint(m_pln2img(Vector2d(pln_x, 0))));
        QPointF const img_line2_pt(toPoint(m_pln2img(Vector2d(pln_x, 1))));

        ToLineProjector const projector(img_generatrix);
        double const y1 = projector.projectionScalar(img_line1_pt);
        double const y2 = projector.projectionScalar(img_line2_pt);

        double const bx = 0.5 * ((y2 + y1) - 1.0) * coeff_h_w;
        double const by = 1.0 - (y2 - y1);
        double const bxy = bx + by;
        double elevation = m_depthPerception / (1.0 + coeff_h_w) * bxy;
        elevation = qBound(-0.5, elevation, 0.5);

        m_arcLengthMapper.addSample(pln_x, elevation);
        prev_pln_x = pln_x;
    }

    // Needs to go before normalizeRange().
    m_directrixArcLength = m_arcLengthMapper.totalArcLength();

    // Scale arc lengths to the range of [0, 1].
    m_arcLengthMapper.normalizeRange(1);
}


/*======================= CoupledPolylinesIterator =========================*/

CylindricalSurfaceDewarper::CoupledPolylinesIterator::CoupledPolylinesIterator(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2,
    HomographicTransform<2, double> const& pln2img,
    HomographicTransform<2, double> const& img2pln)
    : m_seq1It(img_directrix1.begin()),
      m_seq2It(img_directrix2.begin()),
      m_seq1End(img_directrix1.end()),
      m_seq2End(img_directrix2.end()),
      m_pln2img(pln2img),
      m_img2pln(img2pln),
      m_prevImgPt1(*m_seq1It),
      m_prevImgPt2(*m_seq2It),
      m_nextImgPt1(m_prevImgPt1),
      m_nextImgPt2(m_prevImgPt2),
      m_nextPlnX1(0),
      m_nextPlnX2(0)
{
}

bool
CylindricalSurfaceDewarper::CoupledPolylinesIterator::next(QPointF& img_pt1, QPointF& img_pt2, double& pln_x)
{
    if (m_nextPlnX1 < m_nextPlnX2 && m_seq1It != m_seq1End)
    {
        next1(img_pt1, img_pt2, pln_x);
        return true;
    }
    else if (m_seq2It != m_seq2End)
    {
        next2(img_pt1, img_pt2, pln_x);
        return true;
    }
    else
    {
        return false;
    }
}

void
CylindricalSurfaceDewarper::CoupledPolylinesIterator::next1(QPointF& img_pt1, QPointF& img_pt2, double& pln_x)
{
    Vector2d const pln_pt1(m_img2pln(toVec(m_nextImgPt1)));
    pln_x = pln_pt1[0];
    img_pt1 = m_nextImgPt1;

    Vector2d const pln_ptx(pln_pt1[0], pln_pt1[1] + 1);
    QPointF const img_ptx(toPoint(m_pln2img(pln_ptx)));

    if (QLineIntersect(QLineF(img_pt1, img_ptx), QLineF(m_nextImgPt2, m_prevImgPt2), &img_pt2) == QLineF::NoIntersection)
    {
        img_pt2 = m_nextImgPt2;
    }

    advance1();
    if (m_seq2It != m_seq2End && toVec(m_nextImgPt2 - img_pt2).squaredNorm() < 1)
    {
        advance2();
    }
}

void
CylindricalSurfaceDewarper::CoupledPolylinesIterator::next2(QPointF& img_pt1, QPointF& img_pt2, double& pln_x)
{
    Vector2d const pln_pt2(m_img2pln(toVec(m_nextImgPt2)));
    pln_x = pln_pt2[0];
    img_pt2 = m_nextImgPt2;

    Vector2d const pln_ptx(pln_pt2[0], pln_pt2[1] + 1);
    QPointF const img_ptx(toPoint(m_pln2img(pln_ptx)));

    if (QLineIntersect(QLineF(img_pt2, img_ptx), QLineF(m_nextImgPt1, m_prevImgPt1), &img_pt1) == QLineF::NoIntersection)
    {
        img_pt1 = m_nextImgPt1;
    }

    advance2();
    if (m_seq1It != m_seq1End && toVec(m_nextImgPt1 - img_pt1).squaredNorm() < 1)
    {
        advance1();
    }
}

void
CylindricalSurfaceDewarper::CoupledPolylinesIterator::advance1()
{
    if (++m_seq1It == m_seq1End)
    {
        return;
    }

    m_prevImgPt1 = m_nextImgPt1;
    m_nextImgPt1 = *m_seq1It;
    m_nextPlnX1 = m_img2pln(toVec(m_nextImgPt1))[0];
}

void
CylindricalSurfaceDewarper::CoupledPolylinesIterator::advance2()
{
    if (++m_seq2It == m_seq2End)
    {
        return;
    }

    m_prevImgPt2 = m_nextImgPt2;
    m_nextImgPt2 = *m_seq2It;
    m_nextPlnX2 = m_img2pln(toVec(m_nextImgPt2))[0];
}

} // namespace dewarping
