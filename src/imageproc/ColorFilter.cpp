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
#include "BinaryThreshold.h"
#include "Binarize.h"
#include "ColorFilter.h"

namespace imageproc
{

GrayImage wienerFilter(
    GrayImage const& image, QSize const& window_size, float const noise_sigma)
{
    GrayImage dst(image);
    wienerFilterInPlace(dst, window_size, noise_sigma);
    return dst;
}

void wienerFilterInPlace(
    GrayImage& image, QSize const& window_size, float const noise_sigma)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("wienerFilter: empty window_size");
    }
    if (image.isNull())
    {
        return;
    }

    if (noise_sigma > 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        float const noise_variance = noise_sigma * noise_sigma;

        uint8_t* image_line = image.data();
        int const image_stride = image.stride();

        GrayImage gmean = grayMapMean(image, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        GrayImage gdeviation = grayMapDeviation(image, window_size);
        if (gdeviation.isNull())
        {
            return;
        }

        uint8_t* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float const variance = deviation * deviation;

                float const src_pixel = (float) image_line[x];
                float const delta_pixel = src_pixel - mean;
                float const delta_variance = variance - noise_variance;
                float dst_pixel = mean;
                if (delta_variance > 0.0f)
                {
                    dst_pixel += delta_pixel * delta_variance / variance;
                }
                int val = (int) (dst_pixel + 0.5f);
                val = (val < 0) ? 0 : (val < 255) ? val : 255;
                image_line[x] = (uint8_t) val;
            }
            image_line += image_stride;
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }
}

QImage wienerColorFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    wienerColorFilterInPlace(dst, f_size, coef);
    return dst;
}

void wienerColorFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    if (image.isNull())
    {
        return;
    }
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("wienerFilter: empty window_size");
    }

    if (coef > 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage wiener(wienerFilter(gray, window_size, 255.0f * coef));
        if (wiener.isNull())
        {
            return;
        }

        uint8_t* wiener_line = wiener.data();
        int const wiener_stride = wiener.stride();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float color = wiener_line[x];
                // color = coef * color + (1.0 - coef) * origin;

                float const colscale = (color + 1.0f) / (origin + 1.0f);
                float const coldelta = color - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (int) (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            wiener_line += wiener_stride;
        }
    }
}

QImage autoLevelFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    autoLevelFilterInPlace(dst, f_size, coef);
    return dst;
}

void autoLevelFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("autoLevelFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = grayMapMean(gray, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        int gmin = 256, gmax = 0;
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const mean = gmean_line[x];
                gmin = (gmin < mean) ? gmin : mean;
                gmax = (gmax < mean) ? mean : gmax;
            }
            gmean_line += gmean_stride;
        }
        gmean = GrayImage(); /* free */

        float const a1 = 256.0f / (gmax - gmin + 1);
        float const a0 = -a1 * gmin;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float retval = origin * a1 + a0;
                retval = coef * retval + (1.0f - coef) * origin;
                float const colscale = (retval + 1.0f) / (origin + 1.0f);
                float const coldelta = retval - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
        }
    }
}

QImage knnDenoiserFilter(
    QImage const& image, int const radius, float const coef)
{
    QImage dst(image);
    knnDenoiserFilterInPlace(dst, radius, coef);
    return dst;
}

void knnDenoiserFilterInPlace(
    QImage& image, int const radius, float const coef)
{
    if (image.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef > 0.0))
    {
        float const threshold_weight = 0.02f;
        float const threshold_lerp = 0.66f;
        float const noise_eps = 0.0000001f;
        float const noise_lerpc = 0.16f;

        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gknn = grayKnnDenoiser(gray, radius, coef);
        if (gknn.isNull())
        {
            return;
        }

        uint8_t* gknn_line = gknn.data();
        int const gknn_stride = gknn.stride();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float const knn = gknn_line[x];

                float const colscale = (knn + 1.0f) / (origin + 1.0f);
                float const coldelta = knn - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (int) (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            gknn_line += gknn_stride;
        }
    }
}

QImage colorDespeckleFilter(
    QImage const& image, int const radius, float const coef)
{
    QImage dst(image);
    colorDespeckleFilterInPlace(dst, radius, coef);
    return dst;
}

void colorDespeckleFilterInPlace(
    QImage& image, int const radius, float const coef)
{
    if (image.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage temp = GrayImage(image);
        if (temp.isNull())
        {
            return;
        }

        uint8_t* temp_line = temp.data();
        int const temp_stride = temp.stride();

        for (int j = 0; j < radius; j++)
        {
            gray_line = gray.data();
            temp_line = temp.data();
            for (int y = 0; y < h; y++)
            {
                uint8_t* gray_line1 = (y > 0) ? (gray_line - gray_stride) : gray_line;
                uint8_t* gray_line2 = (y < (h - 1)) ? (gray_line + gray_stride) : gray_line;
                for (int x = 0; x < w; x++)
                {
                    int x1 = (x > 0) ? (x - 1) : x;
                    int x2 = (x < (w - 1)) ? (x + 1) : x;

                    int pA = gray_line1[x1];
                    int pB = gray_line1[x];
                    int pC = gray_line1[x2];
                    int pD = gray_line[x1];
                    int pE = gray_line[x];
                    int pF = gray_line[x2];
                    int pH = gray_line2[x1];
                    int pG = gray_line2[x];
                    int pI = gray_line2[x2];

                    int dIy = (pH - pB) * (pH - pB);
                    int dIx = (pF - pD) * (pF - pD);
                    int dIxy = (pH - pB) * (pF - pD);
                    int dI = dIy + dIx;
                    int pR = pE;
                    int pM = (pA + pB + pC + pD + pE + pF + pG + pH + pI + 4) / 9;

                    if (dI > 0)
                    {
                        float gy = pH + pB - pE - pE;
                        float gx = pF + pD - pE - pE;
                        float gxy = pG + pC - pI - pA;
                        float gd2 = (gy * dIx + gx * dIy - 0.5f * gxy * dIxy);
                        if((pM > 128 && gd2 < 0.0f) || (pM <= 128 && gd2 >= 0.0f))
                        {
                            pR = pE + (int) (coef * gd2 / dI + 0.5f);
                            pR = (pR < 0) ? 0 : (pR < 255) ? pR : 255;
                        }
                    }
                    else
                    {
                        pR = pM;
                    }
                    temp_line[x] = pR;

                }
                gray_line += gray_stride;
                temp_line += temp_stride;
            }
            gray_line = gray.data();
            temp_line = temp.data();
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    gray_line[x] = temp_line[x];
                }
                gray_line += gray_stride;
                temp_line += temp_stride;
            }
        }
        temp = GrayImage(image);
        if (temp.isNull())
        {
            return;
        }

        gray_line = gray.data();
        temp_line = temp.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = temp_line[x];
                float color = gray_line[x];
                float const colscale = (color + 1.0f) / (origin + 1.0f);
                float const coldelta = color - origin * colscale;
                for (unsigned int c = 0; c < cnum; c++)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (int) (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            temp_line += temp_stride;
        }
    }
}

QImage blurFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    blurFilterInPlace(dst, f_size, coef);
    return dst;
}

void blurFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("blurFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = grayMapMean(gray, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const mean = gmean_line[x];

                float const origin = gray_line[x];
                float retval = coef * mean + (1.0 - coef) * origin;
                float const colscale = (retval + 1.0f) / (origin + 1.0f);
                float const coldelta = retval - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            gmean_line += gmean_stride;
        }
    }
}

QImage screenFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    screenFilterInPlace(dst, f_size, coef);
    return dst;
}

void screenFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    if (image.isNull())
    {
        return;
    }
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("screenFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = grayMapMean(gray, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        unsigned long int histogram[256] = {0};
        unsigned long int szi = (h * w) >> 8;
        gray_line = gray.data();
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                unsigned int const mean = gmean_line[x];

                gray_line[x] = mean;
                unsigned int indx = mean;
                histogram[indx]++;
            }
            gray_line += gray_stride;
            gmean_line += gmean_stride;
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
                float const origin = gray_line[x];
                float const remap = histogram[gray_line[x]];
                float const colscale = (remap + 1.0f) / (origin + 1.0f);
                float const coldelta = remap - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    float valpos = origcol * colscale + coldelta * kcg;
                    float valneg = 255.0f - valpos;
                    float retval = origcol * valneg;
                    retval /= 255.0f;
                    retval += valpos;
                    retval = coef * retval + (1.0f - coef) * origcol;
                    int val = (int) (retval + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
        }
    }
}

QImage colorCurveFilter(
    QImage& image, float const coef)
{
    QImage dst(image);
    colorCurveFilterInPlace(dst, coef);
    return dst;
}

void colorCurveFilterInPlace(
    QImage& image, float const coef)
{
    if (image.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int icoef = (int) (coef * 256.0f + 0.5f);
        unsigned int const w = image.width();
        unsigned int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        uint8_t pix_replace[256];

        int thres = BinaryThreshold::otsuThreshold(image);
        thres <<= 8;
        for (unsigned int j = 0; j < 256; j++)
        {
            int val = (j << 8);
            int delta = (val - thres);
            int dsqr = delta * delta;
            int ddiv = (delta < 0) ? -thres : (65280 - thres);
            dsqr = (ddiv == 0) ? 0 : dsqr / ddiv;
            delta -= dsqr;
            delta *= icoef;
            delta += 128;
            delta >>= 8;
            val += delta;
            val += 128;
            val >>= 8;
            pix_replace[j] = (uint8_t) val;
        }

        for (unsigned long int i = 0; i < (h * image_stride); i++)
        {
            uint8_t val = image_line[i];
            image_line[i] = pix_replace[val];
        }
    }
}

QImage colorSqrFilter(
    QImage& image, float const coef)
{
    QImage dst(image);
    colorSqrFilterInPlace(dst, coef);
    return dst;
}

void colorSqrFilterInPlace(
    QImage& image, float const coef)
{
    if (image.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int icoef = (int) (coef * 256.0f + 0.5f);
        unsigned int const w = image.width();
        unsigned int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
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

        for (unsigned long int i = 0; i < (h * image_stride); i++)
        {
            uint8_t val = image_line[i];
            image_line[i] = pix_replace[val];
        }
    }
}

GrayImage coloredSignificanceFilter(
    QImage const& image, float const coef)
{
    GrayImage dst(image);
    coloredSignificanceFilterInPlace(image, dst, coef);
    return dst;
}

void coloredSignificanceFilterInPlace(
    QImage const& image, GrayImage& gray, float const coef)
{
    if (image.isNull())
    {
        return;
    }
    if (gray.isNull())
    {
        gray = GrayImage(image);
    }
    if (gray.isNull())
    {
        return;
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wg = gray.width();
    unsigned int const hg = gray.height();
    uint8_t const* image_line = (uint8_t const*) image.bits();
    int const image_stride = image.bytesPerLine();
    unsigned int const cnum = image_stride / w;
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    if ((coef != 0.0f) && (w == wg) && (h == hg))
    {
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                QRgb pixel = image.pixel(x, y);
                int r = qRed(pixel);
                int g = qGreen(pixel);
                int b = qBlue(pixel);
                int hsv_s, hsv_si;
                int max = 0, min = 255;
                max = (r < g) ? g : r;
                max = (max < b) ? b : max;
                min = (r > g) ? g : r;
                min = (min > b) ? b : min;
                hsv_s = max - min;
                hsv_si = 255 - hsv_s;
                float retval = coef * (float) hsv_si + (1.0f - coef) * 255.0f;
                int val = (int) (retval + 0.5f);
                val = (val < 0) ? 0 : (val < 255) ? val : 255;
                gray_line[x] = (uint8_t) val;
            }
            gray_line += gray_stride;
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
            gray_line += gray_stride;
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

    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wg = gray.width();
    unsigned int const hg = gray.height();

    if ((w == wg) && (h == hg))
    {
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;
        uint8_t const* gray_line = gray.data();
        int const gray_stride = gray.stride();

        for (unsigned int y = 0; y < h; ++y)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                float ycbcr = gray_line[x];
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    unsigned int const indx = x * cnum + c;
                    float const origin = image_line[indx];
                    float retval = origin;
                    retval *= (ycbcr + 1.0f);
                    retval /= 256.0f;
                    int val = (int) (retval + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
        }
    }
}

void coloredMaskInPlace(
    QImage& image, BinaryImage content, BinaryImage mask)
{
    if (image.isNull() || content.isNull() || mask.isNull())
    {
        return;
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wc = content.width();
    unsigned int const hc = content.height();
    unsigned int const wm = mask.width();
    unsigned int const hm = mask.height();

    if ((w == wc) && (h == hc) && (w == wm) && (h == hm))
    {
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;
        uint32_t const* content_line = content.data();
        int const content_stride = content.wordsPerLine();
        uint32_t const* mask_line = mask.data();
        int const mask_stride = mask.wordsPerLine();
        double fgmean[4] = {0};
        unsigned long int count = 0;

        uint32_t const msb = uint32_t(1) << 31;
        for (unsigned int y = 0; y < h; ++y)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                if (content_line[x >> 5] & (msb >> (x & 31)))
                {
                    if (!(mask_line[x >> 5] & (msb >> (x & 31))))
                    {
                        for (unsigned int c = 0; c < cnum; ++c)
                        {
                            unsigned int const indx = x * cnum + c;
                            fgmean[c] += image_line[indx];
                        }
                        count++;
                    }
                }
            }
            image_line += image_stride;
            content_line += content_stride;
            mask_line += mask_stride;
        }
        if (count > 0)
        {
            for (unsigned int c = 0; c < cnum; ++c)
            {
                fgmean[c] /= count;
                fgmean[c] += 0.5;
            }

            image_line = (uint8_t*) image.bits();
            content_line = content.data();
            mask_line = mask.data();
            for (unsigned int y = 0; y < h; ++y)
            {
                for (unsigned int x = 0; x < w; ++x)
                {
                    if (content_line[x >> 5] & (msb >> (x & 31)))
                    {
                        if (!(mask_line[x >> 5] & (msb >> (x & 31))))
                        {
                            for (unsigned int c = 0; c < cnum; ++c)
                            {
                                unsigned int const indx = x * cnum + c;
                                image_line[indx] = (uint8_t) fgmean[c];
                            }
                        }
                    }
                }
                image_line += image_stride;
                content_line += content_stride;
                mask_line += mask_stride;
            }
        }
    }
}

QImage imageHSVcylinder(QImage const& image)
{
    if (image.isNull())
    {
        return image;
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();

    QImage hsv_img(w, h, QImage::Format_RGB32);
    if (hsv_img.isNull())
    {
        return QImage();
    }

    uint8_t* hsv_line = (uint8_t*) hsv_img.bits();

    float ctorad = (float)(2.0 * M_PI / 256.0);

    for (unsigned int y = 0; y < h; y++)
    {
        QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
        for (unsigned int x = 0; x < w; x++)
        {
            QRgb pixel = image.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);
            float hsv_h = 0.0f, hsv_s = 0.0f, hsv_v = 0.0f;
            int max = 0, min = 255;
            max = (r < g) ? g : r;
            max = (max < b) ? b : max;
            min = (r > g) ? g : r;
            min = (min > b) ? b : min;
            hsv_h = max - min;
            if (hsv_h > 0.0f)
            {
                if (max == r)
                {
                    hsv_h = (256.0f * (g - b) / hsv_h) / 6.0f;
                    if (hsv_h < 0.0f)
                    {
                        hsv_h += 256.0f;
                    }
                }
                else if (max == g)
                {
                    hsv_h = (256.0f * (2.0f + (float) (b - r) / hsv_h)) / 6.0f;
                }
                else
                {
                    hsv_h = (256.0f * (4.0f + (float) (r - g) / hsv_h)) / 6.0f;
                }
            }
            hsv_s = max - min;
            if (max > 0)
            {
                hsv_s *= 256.0f;
                hsv_s /= max;
            }
            hsv_v = max;
            r = (int) (128.0f + 0.5f * hsv_s * cos(hsv_h * ctorad)); // +0.5f for round
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (int) (128.0f + 0.5f * hsv_s * sin(hsv_h * ctorad)); // +0.5f for round
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (int) (0.5f + hsv_v); // +0.5f for round
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            rowh[x] = qRgb(r, g, b);
        }
    }

    return hsv_img;
}

QImage imageHSLcylinder(QImage const& image)
{
    if (image.isNull())
    {
        return image;
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();

    QImage hsl_img(w, h, QImage::Format_RGB32);
    if (hsl_img.isNull())
    {
        return QImage();
    }

    uint8_t* hsl_line = (uint8_t*) hsl_img.bits();

    float ctorad = (float)(2.0 * M_PI / 256.0);

    for (unsigned int y = 0; y < h; y++)
    {
        QRgb *rowh = (QRgb*)hsl_img.constScanLine(y);
        for (unsigned int x = 0; x < w; x++)
        {
            QRgb pixel = image.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);
            float hsl_h = 0.0f, hsl_s = 0.0f, hsl_l = 0.0f;
            float hsl_d, hsl_p;
            int max = 0, min = 255;
            max = (r < g) ? g : r;
            max = (max < b) ? b : max;
            min = (r > g) ? g : r;
            min = (min > b) ? b : min;
            hsl_d = max - min;
            hsl_p = max + min;
            if (hsl_d > 0.0f)
            {
                if (max == r)
                {
                    hsl_h = (g - b) * 256.0f / hsl_d;
                }
                else if (max == g)
                {
                    hsl_h = (b - r) * 256.0f / hsl_d + 512;
                }
                else
                {
                    hsl_h = (r - g) * 256.0f / hsl_d + 1024;
                }
                hsl_h /= 6.0f;
                hsl_h = (hsl_h < 0.0f) ? (hsl_h + 256.0f) : hsl_h;
                hsl_s = 256.0f * hsl_d / (256.0f - fabs(256.0f - hsl_p));
            }
            hsl_l = 0.5f * hsl_p;
            r = (int) (128.0f + 0.5f * hsl_s * cos(hsl_h * ctorad));
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (int) (128.0f + 0.5f * hsl_s * sin(hsl_h * ctorad));
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (int) (0.5f + hsl_l); // +0.5f for round
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            rowh[x] = qRgb(r, g, b);
        }
    }

    return hsl_img;
}

QImage imageYCbCr(QImage const& image)
{
    if (image.isNull())
    {
        return image;
    }

    unsigned int const w = image.width();
    unsigned int const h = image.height();

    QImage ycbcr_img(w, h, QImage::Format_RGB32);
    if (ycbcr_img.isNull())
    {
        return QImage();
    }

    uint8_t* ycbcr_line = (uint8_t*) ycbcr_img.bits();

    for (unsigned int y = 0; y < h; y++)
    {
        QRgb *rowh = (QRgb*)ycbcr_img.constScanLine(y);
        for (unsigned int x = 0; x < w; x++)
        {
            QRgb pixel = image.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            float cy = 0.299f  * r + 0.587f * g + 0.114f * b;
            float cb = 128.0f - 0.168736f * r - 0.331264f * g + 0.5f * b;
            float cr = 128.0f + 0.5f * r - 0.418688f * g - 0.081312f * b;

            r = (int) (0.5f + cb);
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (int) (0.5f + cr);
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (int) (0.5f + cy);
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            rowh[x] = qRgb(r, g, b);
        }
    }

    return ycbcr_img;
}

float pixelDistance(
    float const h0, float const s0, float const v0,
    float const h1, float const s1, float const v1)
{
    float const delta_h = h0 - h1;
    float const delta_s = s0 - s1;
    float const delta_v = v0 - v1;
    float const delta_h2 = delta_h * delta_h;
    float const delta_s2 = delta_s * delta_s;
    float const delta_v2 = delta_v * delta_v;
    float const dist = delta_h2 + delta_h2 + delta_s2 + delta_s2 + delta_v2;

    return dist;
}

void paletteHSVcylinderGenerate(
    double* mean_h0, double* mean_s0, double* mean_v0,
    int const ncount, float const start_value)
{
    if (ncount > 0)
    {
        float ctorad = (float)(2.0 * M_PI / 256.0);
        float const fk = 256.0f / (float) ncount;

        for (int i = 1; i <= ncount; i++)
        {
            float const hsv_h = ((float) i - 0.5f) * fk;
            mean_h0[i] = 128.0 * (1.0 + cos(hsv_h * ctorad));
            mean_s0[i] = 128.0 * (1.0 + sin(hsv_h * ctorad));
            mean_v0[i] = start_value;
        }
    }
}

void paletteHSVcylinderToHSV(double* mean_h, double* mean_s, int const ncount)
{
    if (ncount > 0)
    {
        float ctorad = (float)(2.0 * M_PI / 256.0);

        for (int k = 0; k <= ncount; k++)
        {
            float const hsv_hsc = (mean_h[k] - 128.0f) * 2.0f;
            float const hsv_hss = (mean_s[k] - 128.0f) * 2.0f;
            float hsv_h = atan2(hsv_hss, hsv_hsc) / ctorad;
            hsv_h = (hsv_h < 0.0f) ? (hsv_h + 256.0f) : hsv_h;
            float const hsv_s = sqrt(hsv_hsc * hsv_hsc + hsv_hss * hsv_hss);
            mean_h[k] = hsv_h;
            mean_s[k] = hsv_s;
        }
    }
}

void paletteHSVsaturation(
    double* mean_s, float const coef_sat, int const ncount)
{
    if (ncount > 0)
    {
        float min_sat = 512.0f;
        float max_sat = 0.0f;
        for (int k = 0; k <= ncount; k++)
        {
            min_sat = (mean_s[k] < min_sat) ? mean_s[k] : min_sat;
            max_sat = (mean_s[k] > max_sat) ? mean_s[k] : max_sat;
        }
        float d_sat = max_sat - min_sat;
        for (int k = 0; k <= ncount; k++)
        {
            float sat_new = (d_sat > 0.0f) ? ((mean_s[k] - min_sat) * 255.0f / d_sat) : 255.0f;
            sat_new = sat_new * coef_sat + mean_s[k] * (1.0f - coef_sat);
            mean_s[k] = sat_new;
        }
    }
}

void paletteYCbCrsaturation(
    double* mean_cb, double* mean_cr,
    float const coef_sat, int const ncount)
{
    if (ncount > 0)
    {
        float max_sat = 0.0f;
        for (int k = 0; k <= ncount; k++)
        {
            float tsat = (mean_cb[k] < 128.0f) ? (128.0f - mean_cb[k]) : (mean_cb[k] - 128.0f);
            max_sat = (tsat > max_sat) ? tsat : max_sat;
            tsat = (mean_cr[k] < 128.0f) ? (128.0f - mean_cr[k]) : (mean_cr[k] - 128.0f);
            max_sat = (tsat > max_sat) ? tsat : max_sat;
        }
        for (int k = 0; k <= ncount; k++)
        {
            float sat_cb = (max_sat > 0.0f) ? ((mean_cb[k] - 128.0f) * 128.0f / max_sat + 128.0f) : mean_cb[k];
            float sat_cr = (max_sat > 0.0f) ? ((mean_cr[k] - 128.0f) * 128.0f / max_sat + 128.0f) : mean_cr[k];
            sat_cb = sat_cb * coef_sat + mean_cb[k] * (1.0f - coef_sat);
            sat_cr = sat_cr * coef_sat + mean_cr[k] * (1.0f - coef_sat);
            mean_cb[k] = sat_cb;
            mean_cr[k] = sat_cr;
        }
    }
}

void paletteHSVnorm(
    double* mean_v, float const coef_norm, int const ncount)
{
    if (ncount > 0)
    {
        float min_vol = 512.0f;
        float max_vol = 0.0f;
        for (int k = 0; k <= ncount; k++)
        {
            min_vol = (mean_v[k] < min_vol) ? mean_v[k] : min_vol;
            max_vol = (mean_v[k] > max_vol) ? mean_v[k] : max_vol;
        }
        float d_vol = max_vol - min_vol;
        for (int k = 0; k <= ncount; k++)
        {
            float vol_new = (d_vol > 0.0f) ? ((mean_v[k] - min_vol) * 255.0f / d_vol) : 0.0f;
            vol_new = vol_new * coef_norm + mean_v[k] * (1.0f - coef_norm);
            mean_v[k] = vol_new;
        }
    }
}


void paletteHSVtoRGB(
    double* mean_h, double* mean_s, double* mean_v, int const ncount)
{
    if (ncount > 0)
    {
        for (int k = 0; k <= ncount; k++)
        {
            int r, g, b;
            float const hsv_h = mean_h[k];
            float const hsv_s = mean_s[k];
            float const hsv_v = mean_v[k];
            r = g = b = (int) (hsv_v + 0.5f);
            int const i = (int) (hsv_h * 6.0f / 256.0f) % 6;
            int const vm = (int) ((256.0f - hsv_s) * hsv_v + 127)/ 256;
            int const va = (int) ((hsv_v - vm) * (6.0f * hsv_h - i * 256.0f) + 127)/ 256;
            int const vi = vm + va;
            int const vd = hsv_v - va;
            if (hsv_s > 0.0f)
            {
                switch (i)
                {
                default:
                case 0:
                    g = vi;
                    b = vm;
                    break;
                case 1:
                    r = vd;
                    b = vm;
                    break;
                case 2:
                    r = vm;
                    b = vi;
                    break;
                case 3:
                    r = vm;
                    g = vd;
                    break;
                case 4:
                    r = vi;
                    g = vm;
                    break;
                case 5:
                    g = vm;
                    b = vd;
                    break;
                }
            }
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            mean_h[k] = r;
            mean_s[k] = g;
            mean_v[k] = b;
        }
    }
}

void paletteHSLtoRGB(
    double* mean_h, double* mean_s, double* mean_l, int const ncount)
{
    if (ncount > 0)
    {
        float const p1256 = 1.0f / 256.0f;
        float const p6256 = 6.0f / 256.0f;
        float const p2562 = 256.0f / 2.0f;
        float const p2563 = 256.0f / 3.0f;
        float const p2566 = 256.0f / 6.0f;
        float const p25623 = 256.0f * 2.0f / 3.0f;
        for (int k = 0; k <= ncount; k++)
        {
            int r, g, b;
            float const hsl_h = mean_h[k];
            float const hsl_s = mean_s[k];
            float const hsl_l = mean_l[k];
            float const hsl_q = (hsl_l < p2562) ? (hsl_l * (256.0f + hsl_s) * p1256) : (((hsl_l + hsl_s) * 256.0f - hsl_l * hsl_s) * p1256);
            float const hsl_p = 2.0 * hsl_l - hsl_q;
            float const hsl_u = hsl_q - hsl_p;
            float tcr = hsl_h + p2563;
            tcr = (tcr < 256.0f) ? tcr : (tcr - 256.0f);
            float tcg = hsl_h;
            float tcb = hsl_h - p2563;
            tcb = (tcb < 0.0f) ? (tcb + 256.0f): tcb;
            tcr = ((tcr < p2566) ? (hsl_p + hsl_u * tcr * p6256) : (tcr < p2562) ? hsl_q : (tcr < p25623) ? (hsl_p + hsl_u * (p25623 - tcr) * p6256) : hsl_p);
            tcg = ((tcg < p2566) ? (hsl_p + hsl_u * tcg * p6256) : (tcg < p2562) ? hsl_q : (tcg < p25623) ? (hsl_p + hsl_u * (p25623 - tcg) * p6256) : hsl_p);
            tcb = ((tcb < p2566) ? (hsl_p + hsl_u * tcb * p6256) : (tcb < p2562) ? hsl_q : (tcb < p25623) ? (hsl_p + hsl_u * (p25623 - tcb) * p6256) : hsl_p);
            r = (int) (tcr + 0.5f);
            g = (int) (tcg + 0.5f);
            b = (int) (tcb + 0.5f);
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            mean_h[k] = r;
            mean_s[k] = g;
            mean_l[k] = b;
        }
    }
}

void paletteYCbCrtoRGB(
    double* mean_cb, double* mean_cr, double* mean_cy, int const ncount)
{
    if (ncount > 0)
    {
        for (int k = 0; k <= ncount; k++)
        {
            int r, g, b;
            float const cb = mean_cb[k];
            float const cr = mean_cr[k];
            float const cy = mean_cy[k];
            float fcr = cy + 1.402f * (cr - 128.0f);
            float fcg = cy - 0.344136f * (cb - 128.0f) - 0.714136f * (cr - 128.0f);
            float fcb = cy + 1.772f * (cb - 128.0f);
            r = (int) (fcr + 0.5f);
            g = (int) (fcg + 0.5f);
            b = (int) (fcb + 0.5f);
            r = (r < 0) ? 0 : (r < 255) ? r : 255;
            g = (g < 0) ? 0 : (g < 255) ? g : 255;
            b = (b < 0) ? 0 : (b < 255) ? b : 255;
            mean_cb[k] = r;
            mean_cr[k] = g;
            mean_cy[k] = b;
        }
    }
}

void hsvKMeansInPlace(
    QImage& dst, QImage const& image, BinaryImage const& mask,
    int const ncount, int const start_value, int const color_space,
    float const coef_sat, float const coef_norm, float const coef_bg)
{
    if (dst.isNull() || image.isNull() || mask.isNull())
    {
        return;
    }

    if ((ncount > 0) && (ncount < 256))
    {
        unsigned int const w = dst.width();
        unsigned int const h = dst.height();
        unsigned int const wi = image.width();
        unsigned int const hi = image.height();
        unsigned int const wm = mask.width();
        unsigned int const hm = mask.height();

        if ((w != wi) || (h != hi) || (w != wm) || (h != hm))
        {
            return;
        }

        uint32_t const* mask_line = mask.data();
        int const mask_stride = mask.wordsPerLine();

        QImage hsv_img;
        switch (color_space)
        {
        case 0:
        { // HSV
            hsv_img = imageHSVcylinder(image);
            break;
        }
        case 1:
        { // HSL
            hsv_img = imageHSLcylinder(image);
            break;
        }
        case 2:
        { // YCbCr
            hsv_img = imageYCbCr(image);
            break;
        }
        default:
        { // HSV
            hsv_img = imageHSVcylinder(image);
            break;
        }
        }

        if (hsv_img.isNull())
        {
            return;
        }

        unsigned long mean_len[256] = {0};
        double mean_h0[256] = {0.0};
        double mean_s0[256] = {0.0};
        double mean_v0[256] = {0.0};
        double mean_h[256] = {0.0};
        double mean_s[256] = {0.0};
        double mean_v[256] = {0.0};

        uint32_t const msb = uint32_t(1) << 31;

        for (unsigned int y = 0; y < h; y++)
        {
            QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
            for (unsigned int x = 0; x < w; x++)
            {
                if (!(mask_line[x >> 5] & (msb >> (x & 31))))
                {
                    float const r = qRed(rowh[x]);
                    float const g = qGreen(rowh[x]);
                    float const b = qBlue(rowh[x]);
                    mean_h[0] += r;
                    mean_s[0] += g;
                    mean_v[0] += b;
                    mean_len[0]++;
                }
            }
            mask_line += mask_stride;
        }

        if (mean_len[0] > 0)
        {
            double mean_bg_part = 1.0 / (double) mean_len[0];
            mean_h[0] *= mean_bg_part;
            mean_s[0] *= mean_bg_part;
            mean_v[0] *= mean_bg_part;
        }

        GrayImage clusters(image);
        if (clusters.isNull())
        {
            return;
        }

        uint8_t* clusters_line = clusters.data();
        int const clusters_stride = clusters.stride();

        paletteHSVcylinderGenerate(mean_h0, mean_s0, mean_v0, ncount, start_value);

        for (int i = 1; i <= ncount; i++)
        {
            mask_line = mask.data();
            float dist_min = -1.0f;
            for (unsigned int y = 0; y < h; y++)
            {
                QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
                for (unsigned int x = 0; x < w; x++)
                {
                    if (mask_line[x >> 5] & (msb >> (x & 31)))
                    {
                        float const hsv_h = qRed(rowh[x]);
                        float const hsv_s = qGreen(rowh[x]);
                        float const hsv_v = qBlue(rowh[x]);
                        float const dist = pixelDistance(hsv_h, hsv_s, hsv_v, mean_h0[i], mean_s0[i], mean_v0[i]);
                        if ((dist_min < 0.0f) || (dist < dist_min))
                        {
                            mean_h[i] = hsv_h;
                            mean_s[i] = hsv_s;
                            mean_v[i] = hsv_v;
                            dist_min = dist;
                        }
                    }
                }
                mask_line += mask_stride;
            }
            mean_h0[i] = mean_h[i];
            mean_s0[i] = mean_s[i];
            mean_v0[i] = mean_v[i];
        }

        mask_line = mask.data();
        for (unsigned int y = 0; y < h; y++)
        {
            QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
            for (unsigned int x = 0; x < w; x++)
            {
                if (mask_line[x >> 5] & (msb >> (x & 31)))
                {
                    float const hsv_h = qRed(rowh[x]);
                    float const hsv_s = qGreen(rowh[x]);
                    float const hsv_v = qBlue(rowh[x]);
                    float dist_min = -1.0f;
                    int indx_min = 0;
                    for (int k = 1; k <= ncount; k++)
                    {
                        float const dist = pixelDistance(hsv_h, hsv_s, hsv_v, mean_h[k], mean_s[k], mean_v[k]);
                        if ((dist_min < 0.0f) || (dist < dist_min))
                        {
                            indx_min = k;
                            dist_min = dist;
                        }
                    }
                    clusters_line[x] = indx_min;
                }
            }
            mask_line += mask_stride;
            clusters_line += clusters_stride;
        }

        for (unsigned int itr = 0; itr < 50; itr++)
        {
            for (int i = 1; i <= ncount; i++)
            {
                mean_h[i] = 0.0;
                mean_s[i] = 0.0;
                mean_v[i] = 0.0;
                mean_len[i] = 0;
            }

            mask_line = mask.data();
            clusters_line = clusters.data();
            for (unsigned int y = 0; y < h; y++)
            {
                QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
                for (unsigned int x = 0; x < w; x++)
                {
                    if (mask_line[x >> 5] & (msb >> (x & 31)))
                    {
                        int const cluster = clusters_line[x];
                        float const hsv_h = qRed(rowh[x]);
                        float const hsv_s = qGreen(rowh[x]);
                        float const hsv_v = qBlue(rowh[x]);
                        mean_h[cluster] += hsv_h;
                        mean_s[cluster] += hsv_s;
                        mean_v[cluster] += hsv_v;
                        mean_len[cluster]++;
                    }
                }
                mask_line += mask_stride;
                clusters_line += clusters_stride;
            }
            unsigned long changes = 0;
            for (int i = 1; i <= ncount; i++)
            {
                if (mean_len[i] > 0)
                {
                    float const mean_lr = 1.0f / (float) mean_len[i];
                    mean_h[i] *= mean_lr;
                    mean_s[i] *= mean_lr;
                    mean_v[i] *= mean_lr;
                }
                else
                {
                    mean_h[i] = mean_h0[i];
                    mean_s[i] = mean_s0[i];
                    mean_v[i] = mean_v0[i];

                    mask_line = mask.data();
                    float dist_min = -1.0f;
                    for (unsigned int y = 0; y < h; y++)
                    {
                        QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
                        for (unsigned int x = 0; x < w; x++)
                        {
                            if (mask_line[x >> 5] & (msb >> (x & 31)))
                            {
                                float const hsv_h = qRed(rowh[x]);
                                float const hsv_s = qGreen(rowh[x]);
                                float const hsv_v = qBlue(rowh[x]);
                                float const dist = pixelDistance(hsv_h, hsv_s, hsv_v, mean_h0[i], mean_s0[i], mean_v0[i]);
                                if ((dist_min < 0.0f) || (dist < dist_min))
                                {
                                    mean_h[i] = hsv_h;
                                    mean_s[i] = hsv_s;
                                    mean_v[i] = hsv_v;
                                    dist_min = dist;
                                }
                            }
                        }
                        mask_line += mask_stride;
                    }
                    mean_len[i] = 1;
                    changes++;
                }
            }

            mask_line = mask.data();
            clusters_line = clusters.data();
            for (unsigned int y = 0; y < h; y++)
            {
                QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
                for (unsigned int x = 0; x < w; x++)
                {
                    if (mask_line[x >> 5] & (msb >> (x & 31)))
                    {
                        float const hsv_h = qRed(rowh[x]);
                        float const hsv_s = qGreen(rowh[x]);
                        float const hsv_v = qBlue(rowh[x]);
                        float dist_min = -1.0f;
                        int indx_min = 0;
                        for (int k = 1; k <= ncount; k++)
                        {
                            float const dist = pixelDistance(hsv_h, hsv_s, hsv_v, mean_h[k], mean_s[k], mean_v[k]);
                            if ((dist_min < 0.0f) || (dist < dist_min))
                            {
                                indx_min = k;
                                dist_min = dist;
                            }
                        }
                        if (indx_min != clusters_line[x])
                        {
                            clusters_line[x] = indx_min;
                            changes++;
                        }
                    }
                }
                mask_line += mask_stride;
                clusters_line += clusters_stride;
            }

            if (changes == 0)
            {
                break;
            }
        }

        if (color_space < 2)
        {
            paletteHSVcylinderToHSV(mean_h, mean_s, ncount);
            paletteHSVsaturation(mean_s, coef_sat, ncount);
        }
        else
        {
            paletteYCbCrsaturation(mean_h, mean_s, coef_sat, ncount);
        }
        paletteHSVnorm(mean_v, coef_norm, ncount);

        switch (color_space)
        {
        case 0:
        { // HSV
            paletteHSVtoRGB(mean_h, mean_s, mean_v, ncount);
            break;
        }
        case 1:
        { // HSL
            paletteHSLtoRGB(mean_h, mean_s, mean_v, ncount);
            break;
        }
        case 2:
        { // YCbCr
            paletteYCbCrtoRGB(mean_h, mean_s, mean_v, ncount);
            break;
        }
        default:
        { // HSV
            paletteHSVtoRGB(mean_h, mean_s, mean_v, ncount);
            break;
        }
        }

        mean_h[0] *= coef_bg;
        mean_s[0] *= coef_bg;
        mean_v[0] *= coef_bg;
        mean_h[0] += ((1.0f - coef_bg) * 255.0f);
        mean_s[0] += ((1.0f - coef_bg) * 255.0f);
        mean_v[0] += ((1.0f - coef_bg) * 255.0f);

        mask_line = mask.data();
        clusters_line = clusters.data();
        for (unsigned int y = 0; y < h; y++)
        {
            QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
            for (unsigned int x = 0; x < w; x++)
            {
                int r, g, b;
                if (mask_line[x >> 5] & (msb >> (x & 31)))
                {
                    int const cluster = clusters_line[x];
                    r = mean_h[cluster];
                    g = mean_s[cluster];
                    b = mean_v[cluster];
                }
                else
                {
                    QRgb const pixel = dst.pixel(x, y);
                    r = qRed(pixel);
                    g = qGreen(pixel);
                    b = qBlue(pixel);
                    if ((r == 255) && (g == 255) && (b == 255))
                    {
                        r = mean_h[0];
                        g = mean_s[0];
                        b = mean_v[0];
                    }
                }
                rowh[x] = qRgb(r, g, b);
            }
            mask_line += mask_stride;
            clusters_line += clusters_stride;
        }

        dst = hsv_img;
    }
}

void maskMorphologicalErode(
    QImage& image, BinaryImage const& mask, int const radius)
{
    if (image.isNull() || mask.isNull())
    {
        return;
    }

    int const w = image.width();
    int const h = image.height();

    int const wm = mask.width();
    int const hm = mask.height();

    if ((w != wm) || (h != hm))
    {
        return;
    }

    if (radius > 0)
    {
        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;
        uint32_t const* mask_line = mask.data();
        int const mask_stride = mask.wordsPerLine();
        uint32_t const msb = uint32_t(1) << 31;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                if (mask_line[x >> 5] & (msb >> (x & 31)))
                {
                    int origin = gray_line[x];
                    for (int yf = (y - radius); yf <= (y + radius); yf++)
                    {
                        if ((yf >= 0) && (yf < h))
                        {
                            uint32_t const* mask_line_f = mask.data();
                            mask_line_f += (mask_stride * yf);
                            uint8_t* gray_line_f = gray.data();
                            gray_line_f += (gray_stride * yf);
                            uint8_t* image_line_f = (uint8_t*) image.bits();
                            image_line_f += (image_stride * yf);
                            for (int xf = (x - radius); xf <= (x + radius); xf++)
                            {
                                if ((xf >= 0) && (xf < w))
                                {
                                    int const refer = gray_line_f[xf];
                                    if (mask_line_f[xf >> 5] & (msb >> (xf & 31)))
                                    {
                                        if (origin > refer)
                                        {
                                            for (unsigned int c = 0; c < cnum; c++)
                                            {
                                                image_line[x * cnum + c] = image_line_f[xf * cnum + c];
                                            }
                                            origin = refer;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            mask_line += mask_stride;
        }
    }
}

void maskMorphologicalDilate(
    QImage& image, BinaryImage const& mask, int const radius)
{
    if (image.isNull() || mask.isNull())
    {
        return;
    }

    int const w = image.width();
    int const h = image.height();

    int const wm = mask.width();
    int const hm = mask.height();

    if ((w != wm) || (h != hm))
    {
        return;
    }

    if (radius > 0)
    {
        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;
        uint32_t const* mask_line = mask.data();
        int const mask_stride = mask.wordsPerLine();
        uint32_t const msb = uint32_t(1) << 31;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                if (mask_line[x >> 5] & (msb >> (x & 31)))
                {
                    int origin = gray_line[x];
                    for (int yf = (y - radius); yf <= (y + radius); yf++)
                    {
                        if ((yf >= 0) && (yf < h))
                        {
                            uint32_t const* mask_line_f = mask.data();
                            mask_line_f += (mask_stride * yf);
                            uint8_t* gray_line_f = gray.data();
                            gray_line_f += (gray_stride * yf);
                            uint8_t* image_line_f = (uint8_t*) image.bits();
                            image_line_f += (image_stride * yf);
                            for (int xf = (x - radius); xf <= (x + radius); xf++)
                            {
                                if ((xf >= 0) && (xf < w))
                                {
                                    int const refer = gray_line_f[xf];
                                    if (mask_line_f[xf >> 5] & (msb >> (xf & 31)))
                                    {
                                        if (origin < refer)
                                        {
                                            for (unsigned int c = 0; c < cnum; c++)
                                            {
                                                image_line[x * cnum + c] = image_line_f[xf * cnum + c];
                                            }
                                            origin = refer;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            mask_line += mask_stride;
        }
    }
}

void maskMorphologicalOpen(
    QImage& image, BinaryImage const& mask, int const radius)
{
    if (image.isNull() || mask.isNull())
    {
        return;
    }
    maskMorphologicalErode(image, mask, radius);
    maskMorphologicalDilate(image, mask, radius);
}

void maskMorphologicalClose(
    QImage& image, BinaryImage const& mask, int const radius)
{
    if (image.isNull() || mask.isNull())
    {
        return;
    }
    maskMorphologicalDilate(image, mask, radius);
    maskMorphologicalErode(image, mask, radius);
}

void maskMorphological(
    QImage& image, BinaryImage const& mask, int const radius)
{
    if (image.isNull() || mask.isNull())
    {
        return;
    }
    if (radius < 0)
    {
        maskMorphologicalClose(image, mask, -radius);
        maskMorphologicalOpen(image, mask, -radius);
    }
    else
    {
        maskMorphologicalOpen(image, mask, radius);
        maskMorphologicalClose(image, mask, radius);
    }
}

QImage unPaperFilter(
    QImage const& image, unsigned int const iters, float const coef)
{
    QImage dst(image);
    unPaperFilterInPlace(dst, iters, coef);
    return dst;
}

void unPaperFilterInPlace(
    QImage& image, unsigned int const iters, float const coef)
{
    if (image.isNull())
    {
        return;
    }

    if ((iters > 0) && (coef > 0.0f))
    {
        unsigned int const w = image.width();
        unsigned int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        unsigned int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        unsigned int const gray_stride = gray.stride();

        unsigned int const histsize = 256;
        unsigned long int histogram[histsize] = {0};
        unsigned long int hmax = 0, bgcount = 0;
        double bgthres, bg[4] = {0.0}, bgsum[4] = {0.0};

        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                uint8_t const pixel = gray_line[x];
                histogram[pixel]++;
            }
            gray_line += gray_stride;
        }

        for (unsigned int k = 0; k < histsize; k++)
        {
            hmax = (hmax > histogram[k]) ? hmax : histogram[k];
        }
        bgthres = (1.0 - coef) * hmax;

        image_line = (uint8_t*) image.bits();
        gray_line = gray.data();
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned int const origin = gray_line[x];
                if (histogram[origin] > bgthres)
                {
                    unsigned int const indx = x * cnum;
                    for (unsigned int c = 0; c < cnum; c++)
                    {
                        double const origcol = image_line[indx + c];
                        bgsum[c] += origcol;
                    }
                    bgcount++;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
        }
        if (bgcount > 0)
        {
            for (unsigned int c = 0; c < cnum; c++)
            {
                bg[c] = bgsum[c] / bgcount;
            }
            for (unsigned int k = 0; k < iters; k++)
            {
                bgcount = 0;
                for (unsigned int c = 0; c < cnum; c++)
                {
                    bgsum[c] = 0.0;
                }
                double distmax = 0;
                image_line = (uint8_t*) image.bits();
                for (unsigned int y = 0; y < h; y++)
                {
                    for (unsigned int x = 0; x < w; x++)
                    {
                        double dist = 0;
                        unsigned int const indx = x * cnum;
                        for (unsigned int c = 0; c < cnum; c++)
                        {
                            double const origcol = image_line[indx + c];
                            double const delta = origcol - bg[c];
                            dist += delta * delta;
                        }
                        distmax = (dist < distmax) ? distmax : dist;
                    }
                    image_line += image_stride;
                }
                double const distthres = distmax * coef * coef;
                image_line = (uint8_t*) image.bits();
                for (unsigned int y = 0; y < h; y++)
                {
                    for (unsigned int x = 0; x < w; x++)
                    {
                        double dist = 0;
                        unsigned int const indx = x * cnum;
                        for (unsigned int c = 0; c < cnum; c++)
                        {
                            double const origcol = image_line[indx + c];
                            double const delta = origcol - bg[c];
                            dist += delta * delta;
                        }
                        if (dist < distthres)
                        {
                            for (unsigned int c = 0; c < cnum; c++)
                            {
                                double const origcol = image_line[indx + c];
                                bgsum[c] += origcol;
                            }
                            bgcount++;
                        }
                    }
                    image_line += image_stride;
                }
                if (bgcount > 0)
                {
                    for (unsigned int c = 0; c < cnum; c++)
                    {
                        bg[c] = bgsum[c] / bgcount;
                    }
                    image_line = (uint8_t*) image.bits();
                    for (unsigned int y = 0; y < h; y++)
                    {
                        for (unsigned int x = 0; x < w; x++)
                        {
                            double dist = 0;
                            unsigned int const indx = x * cnum;
                            for (unsigned int c = 0; c < cnum; c++)
                            {
                                double const origcol = image_line[indx + c];
                                double const delta = origcol - bg[c];
                                dist += delta * delta;
                            }
                            if (dist < distthres)
                            {
                                for (unsigned int c = 0; c < cnum; ++c)
                                {
                                    image_line[indx + c] = (uint8_t) (bg[c] + 0.5);
                                }
                            }
                        }
                        image_line += image_stride;
                    }
                }
            }
        }
    }
}


QImage gravureFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    gravureFilterInPlace(dst, f_size, coef);
    return dst;
}

void gravureFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("gravureFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = grayMapMean(gray, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        double mean_delta = 0.0;
        for (int y = 0; y < h; ++y)
        {
            double mean_delta_line = 0.0;
            for (int x = 0; x < w; ++x)
            {
                float const mean = gmean_line[x];
                float const origin = gray_line[x];

                float const delta = (origin < mean) ? (mean - origin) : (origin - mean);
                mean_delta_line += delta;
            }
            mean_delta += mean_delta_line;
            gray_line += gray_stride;
            gmean_line += gmean_stride;
        }
        mean_delta /= w;
        mean_delta /= h;
        if (mean_delta > 0.0)
        {
            gray_line = gray.data();
            gmean_line = gmean.data();
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    float const mean = gmean_line[x];
                    float const origin = gray_line[x];

                    float const tline = mean / mean_delta;
                    int threshold = (int) tline;
                    float delta = tline - threshold;
                    delta = (delta < 0.5f) ? (0.5f - delta) : (delta - 0.5f);
                    delta += delta;
                    /* overlay */
                    float retval = origin;
                    if (origin > 127.5f)
                    {
                        retval = 255.0f - retval;
                        delta = 1.0f - delta;
                    }
                    retval *= delta;
                    retval += retval;
                    if (origin > 127.5f)
                    {
                        retval = 255.0f - retval;
                    }
                    retval = coef * retval + (1.0f - coef) * origin;
                    float const colscale = (retval + 1.0f) / (origin + 1.0f);
                    float const coldelta = retval - origin * colscale;
                    for (unsigned int c = 0; c < cnum; ++c)
                    {
                        int const indx = x * cnum + c;
                        float origcol = image_line[indx];
                        float kcg = (origcol + 1.0f) / (origin + 1.0f);
                        int val = (origcol * colscale + coldelta * kcg + 0.5f);
                        val = (val < 0) ? 0 : (val < 255) ? val : 255;
                        image_line[indx] = (uint8_t) val;
                    }
                }
                image_line += image_stride;
                gray_line += gray_stride;
                gmean_line += gmean_stride;
            }
        }
    }
}

QImage dots8Filter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    dots8FilterInPlace(dst, f_size, coef);
    return dst;
}

void dots8FilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("dots8Filter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = grayMapMean(gray, window_size);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        int y, x, yb, xb, k, wwidth = 8;
        int threshold, part;
        // Dots dither matrix
        int ddith[64] = {13,  9,  5, 12, 18, 22, 26, 19,  6,  1,  0,  8, 25, 30, 31, 23, 10,  2,  3,  4, 21, 29, 28, 27, 14,  7, 11, 15, 17, 24, 20, 16, 18, 22, 26, 19, 13,  9,  5, 12, 25, 30, 31, 23,  6,  1,  0,  8, 21, 29, 28, 27, 10,  2,  3,  4, 17, 24, 20, 16, 14,  7, 11, 15};
        int thres[32];

        for (k = 0; k < 32; k++)
        {
            part = k * 8 - 128;
            part = (part < -128) ? -128 : (part < 128) ? part : 128;
            thres[k] = binarizeBiModalValue(gmean, part);
        }

        k = 0;
        for (y = 0; y < wwidth; y++)
        {
            for (x = 0; x < wwidth; x++)
            {
                ddith[k] = thres[ddith[k]];
                k++;
            }
        }

        for (y = 0; y < h; ++y)
        {
            yb = y % wwidth;
            for (x = 0; x < w; ++x)
            {
                xb = x % wwidth;
                threshold = ddith[yb * wwidth + xb];
                gmean_line[x] = (uint8_t) threshold;
            }
            gmean_line += gmean_stride;
        }

        gray_line = gray.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float gmul = gmean_line[x];

                /* overlay */
                float retval = origin;
                if (origin > 127.5f)
                {
                    retval = 255.0f - retval;
                    gmul = 255.0f - gmul;
                }
                retval *= gmul;
                retval += retval;
                retval /= 255.0f;
                if (origin > 127.5f)
                {
                    retval = 255.0f - retval;
                }
                retval = coef * retval + (1.0f - coef) * origin;
                float const colscale = (retval + 1.0f) / (origin + 1.0f);
                float const coldelta = retval - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            gmean_line += gmean_stride;
        }
    }
}

QImage edgedivFilter(
    QImage const& image, int const f_size, float const coef)
{
    QImage dst(image);
    edgedivFilterInPlace(dst, f_size, coef);
    return dst;
}

void edgedivFilterInPlace(
    QImage& image, int const f_size, float const coef)
{
    int const ff_size = f_size + f_size + 1;
    QSize const& window_size = QSize(ff_size, ff_size);
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("edgedivFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_stride = image.bytesPerLine();
        unsigned int const cnum = image_stride / w;

        GrayImage gray = GrayImage(image);
        if (gray.isNull())
        {
            return;
        }

        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean(binarizeEdgeDivPrefilter(gray, window_size, coef, coef));
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        gray_line = gray.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float retval = gmean_line[x];
                float const colscale = (retval + 1.0f) / (origin + 1.0f);
                float const coldelta = retval - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float kcg = (origcol + 1.0f) / (origin + 1.0f);
                    int val = (origcol * colscale + coldelta * kcg + 0.5f);
                    val = (val < 0) ? 0 : (val < 255) ? val : 255;
                    image_line[indx] = (uint8_t) val;
                }
            }
            image_line += image_stride;
            gray_line += gray_stride;
            gmean_line += gmean_stride;
        }
    }
}

} // namespace imageproc
