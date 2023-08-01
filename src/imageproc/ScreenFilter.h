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

#ifndef IMAGEPROC_SCREEN_FILTER_H_
#define IMAGEPROC_SCREEN_FILTER_H_

#include "imageproc_config.h"
#include <QSize>

class QImage;

namespace imageproc
{

class GrayImage;

/**
 * @brief Applies the Screen filter to a image.
 *
 * @param image The image to apply the filter to. A null image is allowed.
 * @param window_size The local neighbourhood around a pixel to use.
 * @param coef The part of filter in the result.
 * @return The filtered image.
 */
IMAGEPROC_EXPORT QImage screenFilter(
    QImage const& image, QSize const& window_size, double coef = 0.0);

/**
 * @brief An in-place version of screenFilter().
 * @see screenFilter()
 */
IMAGEPROC_EXPORT void screenFilterInPlace(
    QImage& image, QSize const& window_size, double coef = 0.0);

} // namespace imageproc

#endif
