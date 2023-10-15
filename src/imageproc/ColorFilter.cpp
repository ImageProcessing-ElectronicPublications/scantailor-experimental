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
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                gray_line[x] = (uint8_t) retval;
            }
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
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;
        uint32_t const* content_line = content.data();
        int const content_wpl = content.wordsPerLine();
        uint32_t const* mask_line = mask.data();
        int const mask_wpl = mask.wordsPerLine();

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
                            image_line[indx] = (uint8_t) 0;
                        }
                    }
                }
            }
            image_line += image_bpl;
            content_line += content_wpl;
            mask_line += mask_wpl;
        }
    }
}

void hsvKMeansInPlace(
    QImage& dst, QImage const& image, BinaryImage const& mask, int const ncount, float const coef_sat, float const coef_norm, float const coef_bg)
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

        uint8_t* dst_line = (uint8_t*) dst.bits();
        int const dst_bpl = dst.bytesPerLine();
        unsigned int const dnum = dst_bpl / w;
        uint8_t const* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const inum = image_bpl / w;
        uint32_t const* mask_line = mask.data();
        int const mask_wpl = mask.wordsPerLine();

        QImage hsv_img(w, h, QImage::Format_RGB32);
        uint8_t* hsv_line = (uint8_t*) hsv_img.bits();
        int const hsv_bpl = hsv_img.bytesPerLine();
        unsigned int const hnum = hsv_bpl / w;

        unsigned long mean_len[256] = {0};
        double mean_h0[256] = {0.0};
        double mean_s0[256] = {0.0};
        double mean_h[256] = {0.0};
        double mean_s[256] = {0.0};
        double mean_v[256] = {0.0};

        float ctorad = (float)(2.0 * M_PI / 256.0);
        uint32_t const msb = uint32_t(1) << 31;

        for (unsigned int y = 0; y < h; y++)
        {
            QRgb *rowh = (QRgb*)hsv_img.constScanLine(y);
            for (unsigned int x = 0; x < w; x++)
            {
                QRgb pixel = image.pixel(x, y);
                int r = qRed(pixel);
                int g = qGreen(pixel);
                int b = qBlue(pixel);
                float hsv_h, hsv_s, hsv_v;
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
                if (!(mask_line[x >> 5] & (msb >> (x & 31))))
                {
                    mean_h[0] += r;
                    mean_s[0] += g;
                    mean_v[0] += b;
                    mean_len[0]++;
                }
                rowh[x] = qRgb(r, g, b);
            }
            mask_line += mask_wpl;
        }

        if (mean_len[0] > 0)
        {
            double mean_bg_part = 1.0 / (double) mean_len[0];
            mean_h[0] *= mean_bg_part;
            mean_s[0] *= mean_bg_part;
            mean_v[0] *= mean_bg_part;
        }

        GrayImage clusters(image);
        uint8_t* clusters_line = clusters.data();
        int const clusters_bpl = clusters.stride();

        float const fk = (ncount > 0) ? (256.0f / (float) ncount) : 0.0f;
        for (int i = 1; i <= ncount; i++)
        {
            float const hsv_h = ((float) i - 0.5f) * fk;
            mean_h0[i] = 128.0 * (1.0 + cos(hsv_h * ctorad));
            mean_s0[i] = 128.0 * (1.0 + sin(hsv_h * ctorad));
            mean_h[i] = mean_h0[i];
            mean_s[i] = mean_s0[i];
            mean_v[i] = 255.0;
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
                    float dist_min = 196608.0f;
                    int indx_min = 0;
                    for (int k = 1; k <= ncount; k++)
                    {
                        float const delta_h = hsv_h - mean_h[k];
                        float const delta_s = hsv_s - mean_s[k];
                        float const delta_v = hsv_v - mean_v[k];
                        float const dist = delta_h * delta_h + delta_s * delta_s + delta_v * delta_v;
                        if (dist < dist_min)
                        {
                            indx_min = k;
                            dist_min = dist;
                        }
                    }
                    clusters_line[x] = indx_min;
                }
            }
            mask_line += mask_wpl;
            clusters_line += clusters_bpl;
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
                mask_line += mask_wpl;
                clusters_line += clusters_bpl;
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
                    float const hsv_hmc = mean_h0[i];
                    float const hsv_hms = mean_s0[i];
                    mean_h[i] = hsv_hmc;
                    mean_s[i] = hsv_hms;
                    mean_v[i] = 255.0;

                    mask_line = mask.data();
                    float dist_min = 196608.0f;
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
                                float const delta_h = hsv_h - hsv_hmc;
                                float const delta_s = hsv_s - hsv_hms;
                                float const delta_v = 255.0f - hsv_v;
                                float const dist = delta_h * delta_h + delta_s * delta_s + delta_v * delta_v;
                                if (dist < dist_min)
                                {
                                    mean_h[i] = hsv_h;
                                    mean_s[i] = hsv_s;
                                    mean_v[i] = hsv_v;
                                    dist_min = dist;
                                }
                            }
                        }
                        mask_line += mask_wpl;
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
                        float dist_min = 196608.0f;
                        int indx_min = 0;
                        for (int k = 1; k <= ncount; k++)
                        {
                            float const delta_h = hsv_h - mean_h[k];
                            float const delta_s = hsv_s - mean_s[k];
                            float const delta_v = hsv_v - mean_v[k];
                            float const dist = delta_h * delta_h + delta_s * delta_s + delta_v * delta_v;
                            if (dist < dist_min)
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
                mask_line += mask_wpl;
                clusters_line += clusters_bpl;
            }

            if (changes == 0)
            {
                break;
            }
        }

        for (int k = 0; k <= ncount; k++)
        {
            float const hsv_hsc = (mean_h[k] - 128.0f) * 2.0f;
            float const hsv_hss = (mean_s[k] - 128.0f) * 2.0f;
            float const hsv_v = mean_v[k];
            float hsv_h = atan2(hsv_hss, hsv_hsc) / ctorad;
            hsv_h = (hsv_h < 0.0f) ? (hsv_h + 256.0f) : hsv_h;
            float const hsv_s = sqrt(hsv_hsc * hsv_hsc + hsv_hss * hsv_hss);
            mean_h[k] = hsv_h;
            mean_s[k] = hsv_s;
        }
        float min_sat = 512.0f;
        float max_sat = 0.0f;
        float min_vol = 512.0f;
        float max_vol = 0.0f;
        for (int k = 0; k <= ncount; k++)
        {
            min_sat = (mean_s[k] < min_sat) ? mean_s[k] : min_sat;
            max_sat = (mean_s[k] > max_sat) ? mean_s[k] : max_sat;
            min_vol = (mean_v[k] < min_vol) ? mean_v[k] : min_vol;
            max_vol = (mean_v[k] > max_vol) ? mean_v[k] : max_vol;
        }
        float d_sat = max_sat - min_sat;
        float d_vol = max_vol - min_vol;
        for (int k = 0; k <= ncount; k++)
        {
            double sat_new = (d_sat > 0.0f) ? ((mean_s[k] - min_sat) * 255.0f / d_sat) : 255.0f;
            sat_new = sat_new * coef_sat + mean_s[k] * (1.0f - coef_sat);
            double vol_new = (d_vol > 0.0f) ? ((mean_v[k] - min_vol) * 255.0f / d_vol) : 0.0f;
            vol_new = vol_new * coef_norm + mean_v[k] * (1.0f - coef_norm);
            mean_s[k] = sat_new;
            mean_v[k] = vol_new;
        }
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
            mask_line += mask_wpl;
            clusters_line += clusters_bpl;
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
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();

        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;
        uint32_t const* mask_line = mask.data();
        int const mask_wpl = mask.wordsPerLine();
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
                            mask_line_f += (mask_wpl * yf);
                            uint8_t* gray_line_f = gray.data();
                            gray_line_f += (gray_bpl * yf);
                            uint8_t* image_line_f = (uint8_t*) image.bits();
                            image_line_f += (image_bpl * yf);
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
            image_line += image_bpl;
            gray_line += gray_bpl;
            mask_line += mask_wpl;
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
        uint8_t* gray_line = gray.data();
        int const gray_bpl = gray.stride();

        uint8_t* image_line = (uint8_t*) image.bits();
        int const image_bpl = image.bytesPerLine();
        unsigned int const cnum = image_bpl / w;
        uint32_t const* mask_line = mask.data();
        int const mask_wpl = mask.wordsPerLine();
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
                            mask_line_f += (mask_wpl * yf);
                            uint8_t* gray_line_f = gray.data();
                            gray_line_f += (gray_bpl * yf);
                            uint8_t* image_line_f = (uint8_t*) image.bits();
                            image_line_f += (image_bpl * yf);
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
            image_line += image_bpl;
            gray_line += gray_bpl;
            mask_line += mask_wpl;
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

} // namespace imageproc
