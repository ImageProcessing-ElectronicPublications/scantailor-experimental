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

#ifndef IMAGEPROC_POLYNOMIAL_SURFACE_H_
#define IMAGEPROC_POLYNOMIAL_SURFACE_H_

#include "imageproc_config.h"
#include <Eigen/Core>
#include <QSize>
#include <stdint.h>

namespace imageproc
{

class BinaryImage;
class GrayImage;

/**
 * \brief A polynomial function describing a 2D surface.
 */
class IMAGEPROC_EXPORT PolynomialSurface
{
    // Member-wise copying is OK.
public:
    /**
     * \brief Calculate a polynomial that approximates the given image.
     *
     * \param hor_degree The degree of the polynomial in horizontal direction.
     *        Must not be negative.  A value of 3 or 4 should be enough
     *        to approximate page background.
     * \param vert_degree The degree of the polynomial in vertical direction.
     *        Must not be negative.  A value of 3 or 4 should be enough
     *        to approximate page background.
     * \param src The image to approximate.  Must be grayscale and not null.
     *
     * \note Building a polynomial surface for full size 300 DPI scans
     *       takes forever, so pass a downscaled version here. 300x300
     *       pixels will be fine.  Once built, the polynomial surface
     *       may then be rendered in the original size, if necessary.
     *
     * \see Help -> About -> References -> [16]
     */
    PolynomialSurface(
        int hor_degree, int vert_degree, GrayImage const& src);

    /**
     * \brief Calculate a polynomial that approximates portions of the given image.
     *
    * \param hor_degree The degree of the polynomial in horizontal direction.
     *        Must not be negative.  A value of 5 should be enough
     *        to approximate page background.
     * \param vert_degree The degree of the polynomial in vertical direction.
     *        Must not be negative.  A value of 5 should be enough
     *        to approximate page background.
     * \param src The image to approximate.  Must be grayscale and not null.
     * \param mask Specifies which areas of \p src to consider.
     *        A pixel in \p src is considered if the corresponding pixel
     *        in \p mask is black.
     *
     * \note Building a polynomial surface for full size 300 DPI scans
     *       takes forever, so pass a downscaled version here. 300x300
     *       pixels will be fine.  Once built, the polynomial surface
     *       may then rendered in the original size, if necessary.
     */
    PolynomialSurface(
        int hor_degree, int vert_degree,
        GrayImage const& src, BinaryImage const& mask);

    /**
     * \brief Return a matrix of polynomial coefficients.
     *
     * It's only necessary to use this function when rendering the surface
     * by means other than render();
     *
     * To be used like this:
     * \code
     * MatrixXd coeffs = surface.coeffs();
     * for (int y = 0; y < height; ++y) {
     *     double const y_norm = double(y) / (height - 1);
     *     for (int x = 0; x < width; ++x) {
     *         double const x_norm = double(x) / (width - 1);
     *         double intensity = 0;
     *         for (int i = 0; i < coeffs.rows(); ++i) {
     *             double const y_pow = pow(y_norm, (double)i);
     *             for (int j = 0; j < coeffs.cols(); ++j) {
     *                 double const x_pow = pow(x_norm, (double)j);
     *                 intensity += y_pow * x_pow * coeffs(i, j);
     *             }
     *         }
     *         output_image(x, y) = lround(intensity * 255.0);
     *     }
     * }
     * \endcode
     */
    Eigen::MatrixXd const& coeffs() const
    {
        return m_coeffs;
    }

    /**
     * \brief Visualizes the polynomial surface as a grayscale image.
     *
     * The surface will be stretched / shrinked to fit the new size.
     */
    GrayImage render(QSize const& size) const;
private:
    void maybeReduceDegrees(int num_data_points);

    int calcNumTerms() const;

    static double calcScale(int dimension);

    static void prepareDataForLeastSquares(
        GrayImage const& image, Eigen::MatrixXd& AtA,
        Eigen::VectorXd& Atb, int h_degree, int v_degree);

    static void prepareDataForLeastSquares(
        GrayImage const& image, BinaryImage const& mask,
        Eigen::MatrixXd& AtA, Eigen::VectorXd& Atb,
        int h_degree, int v_degree);

    static void fixSquareMatrixRankDeficiency(Eigen::MatrixXd& mat);

    Eigen::MatrixXd m_coeffs;
    int m_horDegree;
    int m_vertDegree;
};

} // namespace imageproc

#endif
