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

#define _USE_MATH_DEFINES

#include <string.h>
#include <math.h>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <QImage>
#include <QtGlobal>
#include <QRect>
#include <QDebug>
#include "Grayscale.h"
#include "GrayImage.h"
#include "BinaryImage.h"
#include "ImageMetrics.h"

namespace imageproc
{

static inline bool binaryGetBW(uint32_t const* bw_line, unsigned int x)
{
    static uint32_t const msb = uint32_t(1) << 31;
    uint32_t const mask = msb >> (x & 31);

    return (bw_line[x >> 5] & mask);
}

double grayMetricMSE(
    GrayImage const& orig,  GrayImage const& ref)
{
    if (orig.isNull() || ref.isNull())
    {
        return -1.0;
    }
    int const w = orig.width();
    int const h = orig.height();
    uint8_t const* orig_line = orig.data();
    int const orig_stride = orig.stride();

    if ((ref.width() != w) || (ref.height() != h))
    {
        return -1.0;
    }
    uint8_t const* ref_line = ref.data();
    int const ref_stride = ref.stride();

    double mse = 0.0;
    for (int y = 0; y < h; y++)
    {
        double msel = 0.0;
        for (int x = 0; x < w; x++)
        {
            float const origin = orig_line[x];
            float const target = ref_line[x];
            float const delta = origin - target;
            float const d2 = delta * delta;
            msel += d2;
        }
        mse += msel;
        orig_line += orig_stride;
        ref_line += ref_stride;
    }
    mse /= w;
    mse /= h;
    mse = (mse > 0.0) ? sqrt(mse) : 0.0;
    mse /= 255.0;

    return mse;
}

double binaryMetricBW(
    BinaryImage const& orig)
{
    if (orig.isNull())
    {
        return -1.0;
    }
    int const w = orig.width();
    int const h = orig.height();
    uint32_t const* orig_line = orig.data();
    unsigned int const orig_stride = orig.wordsPerLine();
    uint32_t black = 0, count = 0;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if(binaryGetBW(orig_line, x))
            {
                black++;
            }
            count++;
        }
        orig_line += orig_stride;
    }
    double bwm = (double) black / (double) count;

    return bwm;
}

double grayMetricBW(
    GrayImage const& orig)
{
    if (orig.isNull())
    {
        return -1.0;
    }
    int const w = orig.width();
    int const h = orig.height();
    uint8_t const* orig_line = orig.data();
    int const orig_stride = orig.stride();

    uint64_t mean = 0;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned int origin = orig_line[x];
            mean += origin;
        }
        orig_line += orig_stride;
    }
    mean /= w;
    mean /= h;

    uint32_t black = 0, count = 0;
    orig_line = orig.data();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned int origin = orig_line[x];
            if(origin < mean)
            {
                black++;
            }
            count++;
        }
        orig_line += orig_stride;
    }
    double bwm = (double) black / (double) count;

    return bwm;
}

} // namespace imageproc
