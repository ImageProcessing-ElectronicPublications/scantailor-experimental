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

#ifndef IMAGEPROC_COLOR_FILTER_H_
#define IMAGEPROC_COLOR_FILTER_H_

#include "imageproc_config.h"
#include <QSize>

class QImage;

namespace imageproc
{

class GrayImage;

/**
 * @brief Applies the Wiener filter to a grayscale image.
 *
 * @param image The image to apply the filter to. A null image is allowed.
 * @param window_size The local neighbourhood around a pixel to use.
 * @param noise_sigma The standard deviation of noise in the image.
 * @return The filtered image.
 */
IMAGEPROC_EXPORT GrayImage wienerFilter(
    GrayImage const& image, QSize const& window_size, float noise_sigma);

/**
 * @brief An in-place version of wienerFilter().
 * @see wienerFilter()
 */
IMAGEPROC_EXPORT void wienerFilterInPlace(
    GrayImage& image, QSize const& window_size, float noise_sigma);

IMAGEPROC_EXPORT QImage wienerColorFilter(
    QImage const& image, QSize const& window_size, float coef = 0.0f);

IMAGEPROC_EXPORT void wienerColorFilterInPlace(
    QImage& image, QSize const& window_size, float coef = 0.0f);

IMAGEPROC_EXPORT QImage knnDenoiserFilter(
    QImage const& image, int radius = 1, float coef = 0.0f);

IMAGEPROC_EXPORT void knnDenoiserFilterInPlace(
    QImage& image, int radius = 1, float coef = 0.0f);

IMAGEPROC_EXPORT QImage colorDespeckleFilter(
    QImage const& image, int radius = 1, float coef = 0.0f);

IMAGEPROC_EXPORT void colorDespeckleFilterInPlace(
    QImage& image, int radius = 1, float coef = 0.0f);

IMAGEPROC_EXPORT QImage blurFilter(
    QImage const& image, QSize const& window_size, float coef = 0.0f);

IMAGEPROC_EXPORT void blurFilterInPlace(
    QImage& image, QSize const& window_size, float coef = 0.0f);

/**
 * @brief Applies the Screen filter to a image.
 *
 * @param image The image to apply the filter to. A null image is allowed.
 * @param window_size The local neighbourhood around a pixel to use.
 * @param coef The part of filter in the result.
 * @return The filtered image.
 */
IMAGEPROC_EXPORT QImage screenFilter(
    QImage const& image, QSize const& window_size, float coef = 0.0f);

/**
 * @brief An in-place version of screenFilter().
 * @see screenFilter()
 */
IMAGEPROC_EXPORT void screenFilterInPlace(
    QImage& image, QSize const& window_size, float coef = 0.0f);

IMAGEPROC_EXPORT QImage colorCurveFilter(
    QImage& image, float coef = 0.5f);

IMAGEPROC_EXPORT void colorCurveFilterInPlace(
    QImage& image, float coef = 0.5f);

IMAGEPROC_EXPORT QImage colorSqrFilter(
    QImage& image, float coef = 0.5f);

IMAGEPROC_EXPORT void colorSqrFilterInPlace(
    QImage& image, float coef = 0.5f);

IMAGEPROC_EXPORT GrayImage coloredSignificanceFilter(
    QImage const& image, float coef = 0.0f);

IMAGEPROC_EXPORT void coloredSignificanceFilterInPlace(
    QImage const& image, GrayImage& gray, float coef = 0.0f);

IMAGEPROC_EXPORT QImage coloredDimmingFilter(
    QImage& image, GrayImage& gray);

IMAGEPROC_EXPORT void coloredDimmingFilterInPlace(
    QImage& image, GrayImage& gray);

IMAGEPROC_EXPORT void coloredMaskInPlace(
    QImage& image, BinaryImage content, BinaryImage mask);

IMAGEPROC_EXPORT void hsvKMeansInPlace(
    QImage& dst, QImage const& image, BinaryImage const& mask, int const ncount,
    float coef_sat = 0.0f, float coef_norm = 0.0f, float coef_bg = 0.0f);

IMAGEPROC_EXPORT void maskMorphologicalErode(
    QImage& image, BinaryImage const& mask, int radius = 0);

IMAGEPROC_EXPORT void maskMorphologicalDilate(
    QImage& image, BinaryImage const& mask, int radius = 0);

IMAGEPROC_EXPORT void maskMorphologicalOpen(
    QImage& image, BinaryImage const& mask, int radius = 0);

IMAGEPROC_EXPORT void maskMorphologicalClose(
    QImage& image, BinaryImage const& mask, int radius = 0);

IMAGEPROC_EXPORT void maskMorphological(
    QImage& image, BinaryImage const& mask, int radius = 0);

} // namespace imageproc

#endif
