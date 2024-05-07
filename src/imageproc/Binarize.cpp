/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <QImage>
#include <QRect>
#include <QDebug>
#include "Binarize.h"
#include "BinaryImage.h"
#include "BinaryThreshold.h"
#include "Grayscale.h"
#include "GrayImage.h"
#include "ColorFilter.h"
#include "RasterOpGeneric.h"

namespace imageproc
{

static inline bool binaryGetBW(uint32_t* bw_line, unsigned int x)
{
    static uint32_t const msb = uint32_t(1) << 31;
    uint32_t const mask = msb >> (x & 31);

    return (bw_line[x >> 5] & mask);
}

static inline void binarySetBW(uint32_t* bw_line, unsigned int x, bool black)
{
    static uint32_t const msb = uint32_t(1) << 31;
    uint32_t const mask = msb >> (x & 31);
    if (black)
    {
        // black
        bw_line[x >> 5] |= mask;
    }
    else
    {
        // white
        bw_line[x >> 5] &= ~mask;
    }
}

BinaryImage binarizeOtsu(QImage const& src, int const delta)
{
    return BinaryImage(src, BinaryThreshold(BinaryThreshold::otsuThreshold(src) + delta));
}

BinaryImage binarizeMokji(
    QImage const& src, unsigned const max_edge_width,
    unsigned const min_edge_magnitude)
{
    BinaryThreshold const threshold(
        BinaryThreshold::mokjiThreshold(
            src, max_edge_width, min_edge_magnitude
        )
    );
    return BinaryImage(src, threshold);
}

BinaryImage binarizeUse(GrayImage const& src, unsigned int const threshold)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    uint8_t const* src_line = src.data();
    unsigned int const src_stride = src.stride();

    BinaryImage bw_img(w, h);
    if (bw_img.isNull())
    {
        return BinaryImage();
    }

    uint32_t* bw_line = bw_img.data();
    unsigned int const bw_stride = bw_img.wordsPerLine();

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            binarySetBW(bw_line, x, (src_line[x] < threshold));
        }
        src_line += src_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}  // binarizeUse

BinaryImage binarizeFromMap(GrayImage const& src, GrayImage const& threshold,
                            unsigned char const lower_bound, unsigned char const upper_bound, int const delta)
{
    if (src.isNull() || threshold.isNull())
    {
        return BinaryImage();
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    unsigned int const wt = threshold.width();
    unsigned int const ht = threshold.height();

    if ((w != wt) || (h != ht))
    {
        return BinaryImage();
    }

    uint8_t const* src_line = src.data();
    unsigned int const src_stride = src.stride();
    uint8_t const* threshold_line = threshold.data();
    unsigned int const threshold_stride = threshold.stride();

    BinaryImage bw_img(w, h);
    if (bw_img.isNull())
    {
        return BinaryImage();
    }

    uint32_t* bw_line = bw_img.data();
    unsigned int const bw_stride = bw_img.wordsPerLine();

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            binarySetBW(bw_line, x, (src_line[x] < lower_bound || (src_line[x] <= upper_bound && ((int) src_line[x] < ((int) threshold_line[x] + delta)))));
        }
        src_line += src_stride;
        threshold_line += threshold_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}  // binarizeFromMap

void binarizeNegate(BinaryImage& src)
{
    if (src.isNull())
    {
        return;
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    uint32_t* src_line = src.data();
    unsigned int const src_stride = src.wordsPerLine();

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            binarySetBW(src_line, x, !(binaryGetBW(src_line, x)));
        }
        src_line += src_stride;
    }

}  // binarizeNegate

unsigned int binarizeBiModalValue(GrayImage const& src, int const delta)
{
    unsigned int threshold = 128;
    if (src.isNull())
    {
        return threshold;
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    uint8_t const* gray_line = src.data();
    unsigned int const gray_stride = src.stride();
    unsigned int const histsize = 256;
    uint64_t im, iw, ib, histogram[histsize] = {0};
    unsigned int k, Tn;
    double Tw, Tb;
    double part = 0.5 + (double) delta / 256.0;
    part = (part < 0.0) ? 0.0 : (part < 1.0) ? part : 1.0;
    double const partval = part * (double) histsize;

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            uint8_t const pixel = gray_line[x];
            histogram[pixel]++;
        }
        gray_line += gray_stride;
    }

    Tb = 0.0;
    for (k = 0; k < histsize; k++)
    {
        Tb += histogram[k];
    }
    Tb /= w;
    Tb /= h;

    // threshold = (unsigned int) (partval + 0.5);
    threshold = (unsigned int) (Tb + 0.5);
    Tn = 0;
    while (threshold != Tn)
    {
        Tn = threshold;
        Tb = Tw = 0.0;
        ib = iw = 0;
        for (k = 0; k < histsize; k++)
        {
            im = histogram[k];
            if (k < threshold)
            {
                Tb += (double) (im * k);
                ib += im;
            }
            else
            {
                Tw += (double) (im * k);
                iw += im;
            }
        }
        Tb = (ib > 0) ? (Tb / ib) : 0.0;
        Tw = (iw > 0) ? (Tw / iw) : (double) histsize;
        threshold = (unsigned int) (part * Tw + (1.0 - part) * Tb + 0.5);
    }

    return threshold;
}  // binarizeBiModalValue

BinaryImage binarizeBiModal(GrayImage const& src, int const delta)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    unsigned int threshold = binarizeBiModalValue(src, delta);
    BinaryImage bw_img = binarizeUse(src, threshold);

    return bw_img;
}  // binarizeBiModal

BinaryImage binarizeMean(GrayImage const& src, int const delta)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    uint8_t const* src_line = src.data();
    unsigned int const src_stride = src.stride();
    unsigned long int count = 0, countb = 0;
    double meanl = 0, mean = 0.0, meanw = 0.0, countw = 0.0;
    double dist, dist_mean = 0, threshold = 128;

    for (unsigned int y = 0; y < h; ++y)
    {
        meanl = 0.0;
        for (unsigned int x = 0; x < w; ++x)
        {
            double const pixel = src_line[x];
            meanl += pixel;
            count++;
        }
        mean += meanl;
        src_line += src_stride;
    }
    mean = (count > 0) ? (mean / count) : 128.0;

    src_line = src.data();
    for (unsigned int y = 0; y < h; ++y)
    {
        meanl = 0.0;
        for (unsigned int x = 0; x < w; ++x)
        {
            double const pixel = src_line[x];
            dist = (pixel > mean) ? (pixel - mean) : (mean - pixel);
            dist++;
            dist = 256.0 / dist;
            meanl += (pixel * dist);
            countw += dist;
        }
        meanw += meanl;
        src_line += src_stride;
    }
    meanw = (countw > 0.0) ? (meanw / countw) : 128.0;

    src_line = src.data();
    for (unsigned int y = 0; y < h; ++y)
    {
        meanl = 0.0;
        for (unsigned int x = 0; x < w; ++x)
        {
            double const pixel = src_line[x];
            dist = (pixel > meanw) ? (pixel - meanw) : (meanw - pixel);
            dist *= dist;
            meanl += dist;
        }
        dist_mean += meanl;
        src_line += src_stride;
    }
    dist_mean = (count > 0) ? (dist_mean / count) : 64.0 * 64.0;
    threshold = sqrt(dist_mean);

    src_line = src.data();
    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            double const pixel = src_line[x];
            dist = (pixel > meanw) ? (pixel - meanw) : (meanw - pixel);
            if (dist < threshold)
            {
                // white
                countb++;
            }
        }
        src_line += src_stride;
    }
    countb += countb;

    BinaryImage bw_img(w, h);
    if (bw_img.isNull())
    {
        return BinaryImage();
    }

    uint32_t* bw_line = bw_img.data();
    unsigned int const bw_stride = bw_img.wordsPerLine();

    src_line = src.data();
    threshold *= (count < countb) ? (1.0 - (double) delta * 0.02) : (1.0 + (double) delta * 0.02);
    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            double const pixel = src_line[x];
            dist = (pixel > meanw) ? (pixel - meanw) : (meanw - pixel);
            binarySetBW(bw_line, x, ((dist < threshold) ^ (count < countb)));
        }
        src_line += src_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}  // binarizeMean

GrayImage binarizeDotsMap (GrayImage const& src, int const delta)
{
    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    int y, x, yb, xb, k, wwidth = 8;
    int threshold, part;
    // Dots dither matrix
    int ddith[64] = {13,  9,  5, 12, 18, 22, 26, 19,  6,  1,  0,  8, 25, 30, 31, 23, 10,  2,  3,  4, 21, 29, 28, 27, 14,  7, 11, 15, 17, 24, 20, 16, 18, 22, 26, 19, 13,  9,  5, 12, 25, 30, 31, 23,  6,  1,  0,  8, 21, 29, 28, 27, 10,  2,  3,  4, 17, 24, 20, 16, 14,  7, 11, 15};
    int thres[32];

    for (k = 0; k < 32; k++)
    {
        part = k * 8 + delta - 128;
        part = (part < -128) ? -128 : (part < 128) ? part : 128;
        thres[k] = binarizeBiModalValue(src, part);
        thres[k] = (thres[k] < 1) ? 1 : ((thres[k] < 255) ? thres[k] : 255);
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
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
    }

    return gray;
}

BinaryImage binarizeDots(GrayImage const& src, int const delta)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeDotsMap(src, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, 0));

    return bw_img;
}  // binarizeDots

/*
 * niblack = mean - k * stderr, k = 0.2
 */
GrayImage binarizeNiblackMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeNiblackMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    GrayImage gdeviation = grayMapDeviation(src, window_size);
    if (gdeviation.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();
    uint8_t* gdeviation_line = gdeviation.data();
    int const gdeviation_stride = gdeviation.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const deviation = gdeviation_line[x];
            float threshold = mean - k * deviation;

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
        gdeviation_line += gdeviation_stride;
    }

    return gray;
}

BinaryImage binarizeNiblack(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeNiblack: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeNiblackMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}

BinaryImage binarizeGatosCleaner(
    GrayImage& wiener, BinaryImage const& niblack,
    QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeGatosPostfilter: invalid window_size");
    }

    if (wiener.isNull() || niblack.isNull())
    {
        return niblack;
    }

    int const w = wiener.width();
    int const h = wiener.height();
    int const wb = niblack.width();
    int const hb = niblack.height();

    if ((w != wb) || (h != hb))
    {
        return niblack;
    }

    IntegralImage<uint32_t> niblack_bg_ii(w, h);
    IntegralImage<uint32_t> wiener_bg_ii(w, h);

    uint32_t const* niblack_line = niblack.data();
    int const niblack_stride = niblack.wordsPerLine();
    uint8_t const* wiener_line = wiener.data();
    int const wiener_stride = wiener.stride();

    for (int y = 0; y < h; ++y)
    {
        niblack_bg_ii.beginRow();
        wiener_bg_ii.beginRow();
        for (int x = 0; x < w; ++x)
        {
            // bg: 1, fg: 0
            uint32_t const niblack_inverted_pixel =
                (~niblack_line[x >> 5] >> (31 - (x & 31))) & uint32_t(1);
            uint32_t const wiener_pixel = wiener_line[x];
            niblack_bg_ii.push(niblack_inverted_pixel);

            // bg: wiener_pixel, fg: 0
            wiener_bg_ii.push(wiener_pixel & ~(niblack_inverted_pixel - uint32_t(1)));
        }
        wiener_line += wiener_stride;
        niblack_line += niblack_stride;
    }

    std::vector<QRect> windows;
    for (int scale = 1;; ++scale)
    {
        windows.emplace_back(0, 0, window_size.width() * scale, window_size.height() * scale);
        if (windows.back().width() > w*2 && windows.back().height() > h * 2)
        {
            // Such a window is enough to cover the whole image when centered
            // at any of its corners.
            break;
        }
    }

    // sum(background - original) for foreground pixels according to Niblack.
    uint32_t sum_diff = 0;

    // sum(background) pixels for background pixels according to Niblack.
    uint32_t sum_bg = 0;

    QRect const image_rect(wiener.rect());
    GrayImage background(wiener);
    if (background.isNull())
    {
        return niblack;
    }

    uint8_t* background_line = background.data();
    int const background_stride = background.stride();
    niblack_line = niblack.data();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            for (QRect window : windows)
            {
                window.moveCenter(QPoint(x, y));
                window &= image_rect;
                uint32_t const niblack_sum_bg = niblack_bg_ii.sum(window);
                if (niblack_sum_bg == 0)
                {
                    // No background pixels in this window. Try a larger one.
                    continue;
                }

                static uint32_t const msb = uint32_t(1) << 31;
                if (niblack_line[x >> 5] & (msb >> (x & 31)))
                {
                    // Foreground pixel. Interpolate from background pixels in window.
                    uint32_t const wiener_sum_bg = wiener_bg_ii.sum(window);
                    uint32_t const bg = (wiener_sum_bg + (niblack_sum_bg >> 1)) / niblack_sum_bg;
                    sum_diff += bg - background_line[x];
                    background_line[x] = bg;
                }
                else
                {
                    sum_bg += background_line[x];
                }

                break;
            }
        }
        background_line += background_stride;
        niblack_line += niblack_stride;
    }

    double const delta = double(sum_diff) / (w*h - niblack_bg_ii.sum(image_rect));
    double const b = double(sum_bg) / niblack_bg_ii.sum(image_rect);

    double const q = 0.6;
    double const p1 = 0.5;
    double const p2 = 0.8;

    double const exp_scale = -4.0 / (b * (1.0 - p1));
    double const exp_bias = 2.0 * (1.0 + p1) / (1.0 - p1);
    double const threshold_scale = q * delta * (1.0 - p2);
    double const threshold_bias = q * delta * p2;

    rasterOpGeneric(
        [exp_scale, exp_bias, threshold_scale, threshold_bias]
        (uint8_t& wiener, uint8_t const bg)
    {
        double const threshold = threshold_scale /
                                 (1.0 + exp(double(bg) * exp_scale + exp_bias)) + threshold_bias;
        wiener = double(bg) - double(wiener) > threshold ? 0x00 : 0xff;
    },
    wiener, background
    );

    return BinaryImage(wiener);
}

BinaryImage binarizeGatos(
    GrayImage const& src, QSize const window_size,
    double const noise_sigma, double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeGatos: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    int const w = src.width();
    int const h = src.height();

    GrayImage wiener(wienerFilter(src, QSize(5, 5), noise_sigma));
    BinaryImage niblack(binarizeNiblack(wiener, window_size, k, delta));
    BinaryImage bw_img(binarizeGatosCleaner(wiener, niblack, window_size));

    return bw_img;
}

/*
 * sauvola = mean * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
GrayImage binarizeSauvolaMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSauvolaMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    GrayImage gdeviation = grayMapDeviation(src, window_size);
    if (gdeviation.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();
    uint8_t* gdeviation_line = gdeviation.data();
    int const gdeviation_stride = gdeviation.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const deviation = gdeviation_line[x];

            float threshold = mean * (1.0 + k * (deviation / 128.0 - 1.0));

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
        gdeviation_line += gdeviation_stride;
    }

    return gray;
}

BinaryImage binarizeSauvola(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSauvola: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeSauvolaMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}

/*
 * wolf = mean - k * (mean - min_v) * (1.0 - stderr / stdmax), k = 0.3
 */
GrayImage binarizeWolfMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWolfMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    GrayImage gdeviation = grayMapDeviation(src, window_size);
    if (gdeviation.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();
    uint8_t* gdeviation_line = gdeviation.data();
    int const gdeviation_stride = gdeviation.stride();

    uint32_t min_gray_level = 255;
    float max_deviation = 0.0;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            uint32_t origin = gray_line[x];
            float const deviation = gdeviation_line[x];
            max_deviation = (max_deviation < deviation) ? deviation : max_deviation;
            min_gray_level = (min_gray_level < origin) ? min_gray_level : origin;
        }
        gray_line += gray_stride;
        gdeviation_line += gdeviation_stride;
    }

    gray_line = gray.data();
    gdeviation_line = gdeviation.data();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const deviation = gdeviation_line[x];
            float const a = 1.0 - deviation / max_deviation;
            float threshold = mean - k * a * (mean - min_gray_level);

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
        gdeviation_line += gdeviation_stride;
    }

    return gray;
}

BinaryImage binarizeWolf(
    GrayImage const& src, QSize const window_size,
    unsigned char const lower_bound, unsigned char const upper_bound,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWolf: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeWolfMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, lower_bound, upper_bound, delta));

    return bw_img;
}

/*
 * bradley = mean * (1.0 - k), k = 0.2
 */
GrayImage binarizeBradleyMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeBradleyMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float threshold = (k < 1.0) ? (mean * (1.0 - k)) : 0;

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
    }

    return gray;
}  // binarizeBradleyMap

BinaryImage binarizeBradley(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeBradley: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeBradleyMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}  // binarizeBradley

/*
 * singh = (1.0 - k) * (mean + (max - min) * (1.0 - img / 255.0)), k = 0.2
 */
GrayImage binarizeSinghMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSinghMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    GrayImage graymm = grayMapContrast(src, window_size);
    if (graymm.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();
    uint8_t* graymm_line = graymm.data();
    int const graymm_stride = graymm.stride();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float const mean = gmean_line[x];
            float const origin = gray_line[x];
            float const maxmin = graymm_line[x];
            float threshold = (1.0 - k) * (mean + maxmin * (1.0 - origin / 255.0));

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
        graymm_line += graymm_stride;
    }

    return gray;
}  // binarizeSinghMap

BinaryImage binarizeSingh(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSingh: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeSinghMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}  // binarizeSingh

/*
 * WAN = (mean + max) / 2 * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
GrayImage binarizeWANMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWANMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    GrayImage gdeviation = grayMapDeviation(src, window_size);
    if (gdeviation.isNull())
    {
        return GrayImage();
    }

    GrayImage gmax = grayMapMax(src, window_size);
    if (gmax.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();
    uint8_t* gdeviation_line = gdeviation.data();
    int const gdeviation_stride = gdeviation.stride();
    uint8_t* gmax_line = gmax.data();
    int const gmax_stride = gmax.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const deviation = gdeviation_line[x];
            float const imax = gmax_line[x];

            float threshold = (mean + imax) * 0.5 * (1.0 + k * (deviation / 128.0 - 1.0));
            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
        gdeviation_line += gdeviation_stride;
        gmax_line += gmax_stride;
    }

    return gray;
}

BinaryImage binarizeWAN(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWAN: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeWANMap(src, window_size, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}

GrayImage binarizeEdgeDivPrefilter(
    GrayImage const& src, QSize const window_size,
    double const kep, double const kbd)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeBlurDivPrefilter: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    int const w = gray.width();
    int const h = gray.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const origin = gray_line[x];
            float retval = origin;
            if (kep > 0.0)
            {
                // EdgePlus
                // edge = I / blur (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                float const edge = (retval + 1) / (mean + 1) - 0.5;
                // edgeplus = I * edge, mean value = 0.5 * mean(I)
                float const edgeplus = origin * edge;
                // return k * edgeplus + (1 - k) * I
                retval = kep * edgeplus + (1.0 - kep) * origin;
            }
            if (kbd > 0.0)
            {
                // BlurDiv
                // edge = blur / I (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                float const edgeinv = (mean + 1) / (retval + 1) - 0.5;
                // edgenorm = edge * k + max * (1 - k), mean value = {0.5 .. 1.0} * mean(I)
                float const edgenorm = kbd * edgeinv + (1.0 - kbd);
                // return I / edgenorm
                retval = (edgenorm > 0.0) ? (origin / edgenorm) : origin;
            }
            // trim value {0..255}
            retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
            gray_line[x] = (int) retval;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
    }

    return gray;
}  // binarizeEdgeDivPrefilter

BinaryImage binarizeEdgeDiv(
    GrayImage const& src, QSize const window_size,
    double const kep, double const kbd, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeBlurDiv: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(binarizeEdgeDivPrefilter(src, window_size, kep, kbd));
    BinaryImage bw_img(binarizeBiModal(gray, delta));

    return bw_img;
}  // binarizeEdgeDiv

/*
 * Robust = 255.0 - (surround + 255.0) * sc / (surround + sc), k = 0.2
 * sc = surround - img
 * surround = blur(img, w), w = 15
 */
GrayImage binarizeRobustPrefilter(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeRobustPrefilter: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    GrayImage gmean = grayMapMean(src, window_size);
    if (gmean.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = gmean_line[x];
            float const origin = gray_line[x];
            float retval = origin;
            if (k > 0.0)
            {
                float const sc = mean - origin;
                float const robust = 255.0 - (mean + 255.0) * sc / (mean + sc);
                retval = k * robust + (1.0 - k) * origin;
            }
            retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
            gray_line[x] = (int) retval;
        }
        gray_line += gray_stride;
        gmean_line += gmean_stride;
    }

    return gray;
}

BinaryImage binarizeRobust(
    GrayImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeRobust: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(binarizeRobustPrefilter(src, window_size, k));
    BinaryImage bw_img(binarizeBiModal(gray, delta));

    return bw_img;
} // binarizeRobust

GrayImage binarizeMScaleMap(
    GrayImage const& src, QSize const window_size, double const coef)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMScaleMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    unsigned int whcp, l, i, j, blsz, rsz, radius;
    double immean, kover, sensitivity, sensdiv, senspos, sensinv;
    unsigned int pim, immin, immax, imt, cnth, cntw, level = 0;
    unsigned int maskbl, maskover, tim, threshold = 0;
    unsigned long int idx;

    radius = (window_size.height() + window_size.width()) >> 1;
    whcp = (h + w) >> 1;
    blsz = 1;
    while (blsz < whcp)
    {
        level++;
        blsz <<= 1;
    }
    blsz >>= 1;
    rsz = 1;
    while ((rsz < radius) && (level > 1))
    {
        level--;
        rsz <<= 1;
    }

    gray_line = gray.data();
    immin = gray_line[0];
    immax = immin;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            pim = gray_line[x];
            if (pim < immin)
            {
                immin = pim;
            }
            if (pim > immax)
            {
                immax = pim;
            }
        }
        gray_line += gray_stride;
    }
    immean = (double) (immax + immin);
    immean *= 0.5;
    immean += 0.5;
    tim = (unsigned int) immean;

    gray_line = gray.data();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            gray_line[x] = tim;
        }
        gray_line += gray_stride;
    }

    kover = 1.5;

    if (coef < 0.0)
    {
        sensitivity = -coef;
        sensdiv = sensitivity;
        sensdiv += 1.0;
        sensinv = 1.0 / sensdiv;
        senspos = sensitivity / sensdiv;
    }
    else
    {
        sensitivity = coef;
        sensdiv = sensitivity;
        sensdiv += 1.0;
        senspos = 1.0 / sensdiv;
        sensinv = sensitivity / sensdiv;
    }

    src_line = src.data();
    gray_line = gray.data();
    for (l = 0; l < level; l++)
    {
        cnth = (h + blsz - 1) / blsz;
        cntw = (w + blsz - 1) / blsz;
        maskbl = blsz;
        maskover = (unsigned int) (kover * maskbl);
        for (i = 0; i < cnth; i++)
        {
            int y0 = i * maskbl;
            int y1 = y0 + maskover;
            y1 = (y1 < h) ? y1 : h;
            for (j = 0; j < cntw; j++)
            {
                int x0 = j * maskbl;
                int x1 = x0 + maskover;
                x1 = (x1 < w) ? x1 : w;

                idx = y0 * src_stride + x0;
                immin = src_line[idx];
                immax = immin;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        idx = y * src_stride + x;
                        pim = src_line[idx];
                        if (pim < immin)
                        {
                            immin = pim;
                        }
                        if (pim > immax)
                        {
                            immax = pim;
                        }
                    }
                }
                immean = (double) (immax + immin);
                immean *= 0.5;
                immean *= sensinv;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        idx = y * gray_stride + x;
                        imt = gray_line[idx];
                        imt *= senspos;
                        imt += immean;
                        imt += 0.5;
                        imt = (imt < 0.0) ? 0.0 : ((imt < 255.0) ? imt : 255.0);
                        gray_line[idx] = (uint8_t) imt;
                    }
                }
            }
        }
        blsz >>= 1;
    }

    return gray;
}  // binarizeMScaleMap

BinaryImage binarizeMScale(
    GrayImage const& src, QSize const window_size,
    double const coef, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMScale: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeMScaleMap(src, window_size, coef));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, 255, delta));

    return bw_img;
}  // binarizeMScale

} // namespace imageproc
