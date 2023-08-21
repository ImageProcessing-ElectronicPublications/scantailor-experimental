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

#include <string.h>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <algorithm>
#include <QImage>
#include <QtGlobal>
#include <QRect>
#include <QDebug>
#include "Grayscale.h"
#include "GrayImage.h"
#include "BinaryImage.h"
#include "IntegralImage.h"
#include "BinaryThreshold.h"
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

        IntegralImage<uint32_t> integral_image(w, h);
        IntegralImage<uint64_t> integral_sqimage(w, h);

        uint8_t* image_line = image.data();
        int const image_stride = image.stride();

        for (int y = 0; y < h; ++y)
        {
            integral_image.beginRow();
            integral_sqimage.beginRow();
            for (int x = 0; x < w; ++x)
            {
                uint32_t const pixel = image_line[x];
                integral_image.push(pixel);
                integral_sqimage.push(pixel * pixel);
            }
            image_line += image_stride;
        }

        int const window_lower_half = window_size.height() >> 1;
        int const window_upper_half = window_size.height() - window_lower_half;
        int const window_left_half = window_size.width() >> 1;
        int const window_right_half = window_size.width() - window_left_half;

        image_line = image.data();
        for (int y = 0; y < h; ++y)
        {
            int const top = ((y - window_lower_half) < 0) ? 0 : (y - window_lower_half);
            int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h; // exclusive

            for (int x = 0; x < w; ++x)
            {
                int const left = ((x - window_left_half) < 0) ? 0 : (x - window_left_half);
                int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w; // exclusive
                int const area = (bottom - top) * (right - left);
                assert(area > 0); // because window_size > 0 and w > 0 and h > 0

                QRect const rect(left, top, right - left, bottom - top);
                float const window_sum = integral_image.sum(rect);
                float const window_sqsum = integral_sqimage.sum(rect);

                float const r_area = 1.0f / area;
                float const mean = window_sum * r_area;
                float const sqmean = window_sqsum * r_area;
                float const variance = sqmean - mean * mean;

                float const src_pixel = (float) image_line[x];
                float const delta_pixel = src_pixel - mean;
                float const delta_variance = variance - noise_variance;
                float dst_pixel = mean;
                if (delta_variance > 0.0f)
                {
                    dst_pixel += delta_pixel * delta_variance / variance;
                }
                image_line[x] = (uint8_t) (dst_pixel + 0.5f);
            }
            image_line += image_stride;
        }
    }
}

QImage wienerColorFilter(
    QImage const& image, QSize const& window_size, float const coef)
{
    QImage dst(image);
    wienerColorFilterInPlace(dst, window_size, coef);
    return dst;
}

void wienerColorFilterInPlace(
    QImage& image, QSize const& window_size, float const coef)
{
    if (image.isNull())
    {
        return;
    }
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("wienerFilter: empty window_size");
    }

    if (coef > 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;

        GrayImage gray = GrayImage(image);
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();
        GrayImage wiener(wienerFilter(gray, window_size, 255.0f * coef));
        uint8_t* wiener_line = wiener.data();
        int const wiener_bpl = wiener.stride();

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
                    float val = origcol * colscale + coldelta;
                    val = (val < 0.0f) ? 0.0f : (val < 255.0f) ? val : 255.0f;
                    image_line[indx] = (uint8_t) (val + 0.5f);
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
            wiener_line += wiener_bpl;
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
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;

        GrayImage gray = GrayImage(image);
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();

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

        int const noise_area = ((2 * radius + 1) * (2 * radius + 1));
        float const noise_area_inv = (1.0f / (float) noise_area);
        float const noise_weight = (1.0f / (coef * coef));
        float const pixel_weight = (1.0f / 255.0f);

        gray_line = gray.data();
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float const origin = gray_line[x];
                float f_count = noise_area_inv;
                float sum_weights = 1.0f;
                float color = origin;

                for (int r = 1; r <= radius; r++)
                {
                    int const top = ((y - r) < 0) ? 0 : (y - r);
                    int const bottom = ((y + r) < h) ? (y + r) : h;
                    int const left = ((x - r) < 0) ? 0 : (x - r);
                    int const right = ((x + r) < w) ? (x + r) : w;
                    int const area = (bottom - top) * (right - left);
                    QRect const rect(left, top, right - left, bottom - top);
                    float const window_sum = integral_image.sum(rect);
                    float const r_area = 1.0f / area;
                    float const mean = window_sum * r_area;
                    float const delta = (origin - mean) * pixel_weight * r;
                    float const deltasq = delta * delta;

                    // Denoising
                    float r2 = r * r;
                    float weight_f = expf(-(r2 * noise_area_inv + deltasq * noise_weight));
                    float weight_r = (r << 3);
                    float weight_fr = weight_f * weight_r;
                    color += mean * weight_fr;
                    sum_weights += weight_fr;
                    f_count += (weight_f > threshold_weight) ? (noise_area_inv * weight_r) : 0.0f;
                }

                // Normalize result color
                sum_weights = (sum_weights > 0.0f) ? (1.0f / sum_weights) : 1.0f;
                color *= sum_weights;

                float lerp_q = (f_count > threshold_lerp) ? noise_lerpc : (1.0f - noise_lerpc);
                color = color + (origin - color) * lerp_q;

                // Result to memory
                color = (color < 0.0f) ? 0.0f : ((color < 255.0f) ? color : 255.0f);

                float const colscale = (color + 1.0f) / (origin + 1.0f);
                float const coldelta = color - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float val = origcol * colscale + coldelta;
                    val = (val < 0.0f) ? 0.0f : (val < 255.0f) ? val : 255.0f;
                    image_line[indx] = (uint8_t) (val + 0.5f);
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }
}

QImage blurFilter(
    QImage const& image, QSize const& window_size, float const coef)
{
    QImage dst(image);
    blurFilterInPlace(dst, window_size, coef);
    return dst;
}

void blurFilterInPlace(
    QImage& image, QSize const& window_size, float const coef)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("blurFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;

        GrayImage gray = GrayImage(image);
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();

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
                float const window_sum = integral_image.sum(rect);

                float const r_area = 1.0 / area;
                float const mean = window_sum * r_area;

                float const origin = gray_line[x];
                float retval = coef * mean + (1.0 - coef) * origin;
                float const colscale = (retval + 1.0f) / (origin + 1.0f);
                float const coldelta = retval - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float val = origcol * colscale + coldelta;
                    val = (val < 0.0f) ? 0.0f : (val < 255.0f) ? val : 255.0f;
                    image_line[indx] = (uint8_t) (val + 0.5f);
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }
}

QImage screenFilter(
    QImage const& image, QSize const& window_size, float const coef)
{
    QImage dst(image);
    screenFilterInPlace(dst, window_size, coef);
    return dst;
}

void screenFilterInPlace(
    QImage& image, QSize const& window_size, float const coef)
{
    if (image.isNull())
    {
        return;
    }
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("screenFilter: empty window_size");
    }

    if (coef != 0.0f)
    {
        int const w = image.width();
        int const h = image.height();
        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;

        GrayImage gray = GrayImage(image);
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();

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
                float const window_sum = integral_image.sum(rect);

                float const r_area = 1.0f / area;
                float const mean = window_sum * r_area;
                gray_line[x] = mean;
                unsigned int indx = (unsigned int) (mean + 0.5f);
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
                float const origin = gray_line[x];
                float const remap = histogram[gray_line[x]];
                float const colscale = (remap + 1.0f) / (origin + 1.0f);
                float const coldelta = remap - origin * colscale;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    int const indx = x * cnum + c;
                    float origcol = image_line[indx];
                    float valpos = origcol * colscale + coldelta;
                    float valneg = 255.0f - valpos;
                    float retval = origcol * valneg;
                    retval /= 255.0f;
                    retval += valpos;
                    retval = coef * retval + (1.0f - coef) * origcol;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                    image_line[indx] = (uint8_t) retval;
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
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

    unsigned int const w = image.width();
    unsigned int const h = image.height();
    unsigned int const wg = gray.width();
    unsigned int const hg = gray.height();
    uint8_t const* image_line = (uint8_t const*) image.bits();
    int const image_bpl = image.bytesPerLine();
    unsigned int const cnum = image_bpl / w;
    uint8_t* gray_line = gray.data();
    int const gray_bpl = gray.stride();

    if ((coef != 0.0f) && (cnum > 2) && (w == wg) && (h == hg))
    {
        float const ycb[6] = {-0.168736f, -0.331264f, 0.5f, 0.0f, 0.0f, 0.0f};
        float const ycr[6] = {0.5f, -0.418688f, -0.081312f, 0.0f, 0.0f, 0.0f};

        for (unsigned int y = 0; y < h; ++y)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                float ycbsum = 0.0f;
                float ycrsum = 0.0f;
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    unsigned int const indx = x * cnum + c;
                    float const origin = image_line[indx];
                    ycbsum += (origin * ycb[c]);
                    ycrsum += (origin * ycr[c]);
                }
                ycbsum = (ycbsum < 0.0f) ? -ycbsum : ycbsum;
                ycrsum = (ycrsum < 0.0f) ? -ycrsum : ycrsum;
                float ycbcr = (ycbsum + ycrsum);
                ycbcr = (ycbcr < 0.0f) ? -ycbcr : ycbcr;
                ycbcr = 255.0f - ycbcr;
                float retval = coef * ycbcr + (1.0f - coef) * 255.0f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
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
                float ycbcr = gray_line[x];
                for (unsigned int c = 0; c < cnum; ++c)
                {
                    unsigned int const indx = x * cnum + c;
                    float const origin = image_line[indx];
                    float retval = origin;
                    retval *= (ycbcr + 1.0f);
                    retval /= 256.0f;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                    image_line[indx] = (uint8_t) retval;
                }
            }
            image_line += image_bpl;
            gray_line += gray_bpl;
        }
    }
}

} // namespace imageproc
