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

#include "Grayscale.h"
#include "GrayImage.h"
#include "IntegralImage.h"
#include "ScreenFilter.h"
#include <QImage>
#include <QRect>
#include <QDebug>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <cstdint>
#include <cstddef>
#include <string.h>
#include <assert.h>

namespace imageproc
{

QImage screenFilter(
    QImage const& image, QSize const& window_size, double const coef)
{
    QImage dst(image);
    screenFilterInPlace(dst, window_size, coef);
    return dst;
}

void screenFilterInPlace(
    QImage& image, QSize const& window_size, double const coef)
{
    if (image.isNull())
    {
        return;
    }
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("wienerFilter: empty window_size");
    }
    if (coef < 0.0)
    {
        throw std::invalid_argument("wienerFilter: negative noise_sigma");
    }

    if (coef > 0.0)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();

        GrayImage gray = GrayImage(image);
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();
        unsigned int const cnum = image_bpl / w;

        IntegralImage<uint32_t> integral_image(w, h);

        for (int y = 0; y < h; ++y)
        {
            integral_image.beginRow();
            for (int x = 0; x < w; ++x)
            {
                uint32_t const pixel = gray_line[x];
                integral_image.push(pixel);
            }
            gray_line += gray_bpl;
        }

        int const window_lower_half = window_size.height() >> 1;
        int const window_upper_half = window_size.height() - window_lower_half;
        int const window_left_half = window_size.width() >> 1;
        int const window_right_half = window_size.width() - window_left_half;

        gray_line = gray.data();
        for (int y = 0; y < h; ++y)
        {
            int const top = ((y - window_lower_half) < 0) ? 0 : (y - window_lower_half);
            int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;
            for (int x = 0; x < w; ++x)
            {
                int const left = ((x - window_left_half) < 0) ? 0 : (x - window_left_half);
                int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
                int const area = (bottom - top) * (right - left);
                assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
                QRect const rect(left, top, right - left, bottom - top);
                double const window_sum = integral_image.sum(rect);

                double const r_area = 1.0 / area;
                double const mean = window_sum * r_area;
                double const meano = 255.0 - mean;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    double const origin = image_line[indx];
                    double retval = origin * mean / 255.0 + meano;
                    retval = coef * retval + (1.0 - coef) * origin;
                    retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
                    image_line[indx] = (uint8_t) retval;
                }
            }
            gray_line += gray_bpl;
            image_line += image_bpl;
        }
    }
}

} // namespace imageproc
