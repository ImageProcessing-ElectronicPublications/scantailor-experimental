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

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <cstdint>
#include <cstddef>
#include <string.h>
#include <assert.h>
#include <QImage>
#include <QRect>
#include <QDebug>
#include "Grayscale.h"
#include "GrayImage.h"
#include "IntegralImage.h"
#include "BinaryThreshold.h"
#include "ColorFilter.h"

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
        throw std::invalid_argument("screenFilter: empty window_size");
    }

    if (coef != 0.0)
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

        size_t histogram[256] = {0};
        size_t szi = (h * w) >> 8;
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
                gray_line[x] = mean;
                unsigned int indx = (unsigned int) (mean + 0.5);
                histogram[indx]++;
            }
            gray_line += gray_bpl;
        }

        for (unsigned int i = 1; i < 256; i++)
        {
            histogram[i] += histogram[i - 1];
        }
        for (unsigned int i = 0; i < 256; i++)
        {
            histogram[i] += (szi >> 1);
            histogram[i] /= szi;
            histogram[i] = (histogram[i] < 255) ? histogram[i] : 255;
        }

        gray_line = gray.data();
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                double const origin = gray_line[x];
                double const remap = histogram[gray_line[x]];
                double const colscale = (remap + 1.0) / (origin + 1.0);
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    double origcol = image_line[indx];
                    double valpos = origcol * colscale;
                    double valneg = 255.0 - valpos;
                    double retval = origcol * valneg;
                    retval /= 255.0;
                    retval += valpos;
                    retval = coef * retval + (1.0 - coef) * origcol;
                    retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
                    image_line[indx] = (uint8_t) retval;
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }

}

QImage colorCurveFilter(
    QImage& image, double const coef)
{
    QImage dst(image);
    colorCurveFilterInPlace(dst, coef);
    return dst;
}

void colorCurveFilterInPlace(
    QImage& image, double const coef)
{
    if (image.isNull())
    {
        return;
    }

    if (coef != 0.0)
    {
        int icoef = (int) (coef * 256.0 + 0.5);
        unsigned int const w = image.width();
        unsigned int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        uint8_t pix_replace[256];

        int thres = BinaryThreshold::otsuThreshold(image);
        thres <<= 8;
        for (unsigned int j = 0; j < 256; j++)
        {
            int val = (j << 8);
            int delta = (val - thres);
            int dsqr = delta * delta;
            dsqr = (delta < 0) ? -(dsqr  / thres) : (dsqr / (65280 - thres));
            delta -= dsqr;
            delta *= icoef;
            delta += 128;
            delta >>= 8;
            val += delta;
            val += 128;
            val >>= 8;
            pix_replace[j] = (uint8_t) val;
        }

        for (size_t i = 0; i < (h * image_bpl); i++)
        {
            uint8_t val = image_line[i];
            image_line[i] = pix_replace[val];
        }
    }
}

QImage colorSqrFilter(
    QImage& image, double const coef)
{
    QImage dst(image);
    colorSqrFilterInPlace(dst, coef);
    return dst;
}

void colorSqrFilterInPlace(
    QImage& image, double const coef)
{
    if (image.isNull())
    {
        return;
    }

    if (coef != 0.0)
    {
        int icoef = (int) (coef * 256.0 + 0.5);
        unsigned int const w = image.width();
        unsigned int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        uint8_t pix_replace[256];

        for (unsigned int j = 0; j < 256; j++)
        {
            unsigned int val = j;
            val++;
            val *= val;
            val += 255;
            val >>= 8;
            val--;
            val = icoef * val + (256 - icoef) * j;
            val += 128;
            val >>= 8;
            pix_replace[j] = (uint8_t) val;
        }

        for (size_t i = 0; i < (h * image_bpl); i++)
        {
            uint8_t val = image_line[i];
            image_line[i] = pix_replace[val];
        }
    }
}

GrayImage coloredSignificanceFilter(
    QImage const& image, double const coef)
{
    GrayImage dst(image);
    coloredSignificanceFilterInPlace(image, dst, coef);
    return dst;
}

void coloredSignificanceFilterInPlace(
    QImage const& image, GrayImage& gray, double const coef)
{
    if (image.isNull())
    {
        return;
    }
    if (gray.isNull())
    {
        gray = GrayImage(image);
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wg = gray.width();
    unsigned int const hg = gray.height();
    uint8_t const* image_line = (uint8_t const*) image.bits();
    int const image_bpl = image.bytesPerLine();
    unsigned int const cnum = image_bpl / w;
    uint8_t* gray_line = gray.data();
    int const gray_bpl = gray.stride();
    if ((coef != 0.0) && (cnum > 2) && (w == wg) && (h == hg))
    {
        double const ycb[6] = {-0.168736, -0.331264, 0.5, 0.0, 0.0, 0.0};
        double const ycr[6] = {0.5, -0.418688, -0.081312, 0.0, 0.0, 0.0};

        for (unsigned int y = 0; y < h; ++y)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                double ycbsum = 0.0;
                double ycrsum = 0.0;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    unsigned int const indx = x * cnum + c;
                    double const origin = image_line[indx];
                    ycbsum += (origin * ycb[c]);
                    ycrsum += (origin * ycr[c]);
                }
                ycbsum = (ycbsum < 0.0) ? -ycbsum : ycbsum;
                ycrsum = (ycrsum < 0.0) ? -ycrsum : ycrsum;
                double ycbcr = (ycbsum + ycrsum);
                ycbcr = (ycbcr < 0.0) ? -ycbcr : ycbcr;
                ycbcr = 255.0 - ycbcr;
                double retval = coef * ycbcr + (1.0 - coef) * 255;
                retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
                gray_line[x] = (uint8_t) retval;
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }
    else
    {
        for (unsigned int y = 0; y < hg; ++y)
        {
            for (unsigned int x = 0; x < wg; ++x)
            {
                gray_line[x] = (uint8_t) 255;
            }
            gray_line += gray_bpl;
        }
    }
}

QImage coloredDimmingFilter(
    QImage& image, GrayImage& gray)
{
    QImage dst(image);
    coloredSignificanceFilterInPlace(dst, gray);
    return dst;
}

void coloredDimmingFilterInPlace(
    QImage& image, GrayImage& gray)
{
    if (image.isNull() || gray.isNull())
    {
        return;
    }

    double const ycb[6] = {-0.168736, -0.331264, 0.5, 0.0, 0.0, 0.0};
    double const ycr[6] = {0.5, -0.418688, -0.081312, 0.0, 0.0, 0.0};
    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wg = gray.width();
    unsigned int const hg = gray.height();
    if ((w == wg) && (h == hg))
    {
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;
        uint8_t const* gray_line = gray.data();
        int const gray_bpl = gray.stride();

        for (unsigned int y = 0; y < h; ++y)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                double ycbcr = gray_line[x];
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    unsigned int const indx = x * cnum + c;
                    double const origin = image_line[indx];
                    double retval = origin;
                    retval *= (ycbcr + 1.0);
                    retval /= 256.0;
                    retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
                    image_line[indx] = (uint8_t) retval;
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }
}

} // namespace imageproc
