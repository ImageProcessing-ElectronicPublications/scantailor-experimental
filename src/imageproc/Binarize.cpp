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
#include "RasterOpGeneric.h"
#include "GaussBlur.h"

namespace imageproc
{

static inline bool binaryGetBW(uint32_t const* bw_line, unsigned int x)
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

BinaryImage binarizeUse(
    GrayImage const& src, unsigned int const threshold)
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

    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            binarySetBW(bw_line, x, (src_line[x] < threshold));
        }
        src_line += src_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}  // binarizeUse

BinaryImage binarizeFromMap(
    GrayImage const& src, GrayImage const& threshold, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
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

    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            binarySetBW(bw_line, x, (src_line[x] < bound_lower || (src_line[x] <= bound_upper && ((int) src_line[x] < ((int) threshold_line[x] + delta)))));
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

    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            binarySetBW(src_line, x, !(binaryGetBW(src_line, x)));
        }
        src_line += src_stride;
    }

}  // binarizeNegate

unsigned int binarizeBiModalValue(
    GrayImage const& src, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
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

    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            unsigned char pixel = gray_line[x];
            pixel = (pixel < bound_lower) ? bound_lower : (pixel < bound_upper) ? pixel : bound_upper;
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

BinaryImage binarizeBiModal(
    GrayImage const& src, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    unsigned int threshold = binarizeBiModalValue(src, delta, bound_lower, bound_upper);
    BinaryImage bw_img = binarizeUse(src, threshold);

    return bw_img;
}  // binarizeBiModal

BinaryImage binarizeMean(
    GrayImage const& src, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    uint8_t const* src_line = src.data();
    unsigned int const src_stride = src.stride();
    uint64_t count = 0, countb = 0;
    double meanl = 0, mean = 0.0, meanw = 0.0, countw = 0.0;
    double dist, dist_mean = 0, threshold = 128;

    /*
        for (unsigned int y = 0; y < h; y++)
        {
            meanl = 0.0;
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char pix = src_line[x];
                pix = (pix < bound_lower) ? bound_lower : (pix < bound_upper) ? pix : bound_upper;
                double const pixel = pix;
                meanl += pixel;
                count++;
            }
            mean += meanl;
            src_line += src_stride;
        }
        mean = (count > 0) ? (mean / count) : 128.0;

        src_line = src.data();
        for (unsigned int y = 0; y < h; y++)
        {
            meanl = 0.0;
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char pix = src_line[x];
                pix = (pix < bound_lower) ? bound_lower : (pix < bound_upper) ? pix : bound_upper;
                double const pixel = pix;
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
    */
    meanw = grayDominantaValue(src);

    src_line = src.data();
    count = 0;
    for (unsigned int y = 0; y < h; y++)
    {
        meanl = 0.0;
        for (unsigned int x = 0; x < w; x++)
        {
            unsigned char pix = src_line[x];
            pix = (pix < bound_lower) ? bound_lower : (pix < bound_upper) ? pix : bound_upper;
            double const pixel = pix;
            dist = (pixel > meanw) ? (pixel - meanw) : (meanw - pixel);
            dist *= dist;
            meanl += dist;
            count++;
        }
        dist_mean += meanl;
        src_line += src_stride;
    }
    dist_mean = (count > 0) ? (dist_mean / count) : 64.0 * 64.0;
    threshold = sqrt(dist_mean);

    src_line = src.data();
    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            unsigned char pix = src_line[x];
            pix = (pix < bound_lower) ? bound_lower : (pix < bound_upper) ? pix : bound_upper;
            double const pixel = pix;
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
    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
        {
            unsigned char pix = src_line[x];
            pix = (pix < bound_lower) ? bound_lower : (pix < bound_upper) ? pix : bound_upper;
            double const pixel = pix;
            dist = (pixel > meanw) ? (pixel - meanw) : (meanw - pixel);
            binarySetBW(bw_line, x, ((dist < threshold) ^ (count < countb)));
        }
        src_line += src_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}  // binarizeMean

BinaryImage binarizeDots(
    GrayImage const& src, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayDotsMap(src, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}  // binarizeDots

BinaryImage binarizeNiblack(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayNiblackMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

BinaryImage binarizeGatosCleaner(
    GrayImage const& src, BinaryImage const& niblack,
    int const radius, double const q,
    double const p1, double const p2)
{
    GrayImage gray = GrayImage(src);
    if (gray.isNull() || niblack.isNull())
    {
        return niblack;
    }
    if (radius < 1)
    {
        return niblack;
    }

    int const w = src.width();
    int const h = src.height();
    unsigned int const ws = radius + radius + 1;

    if ((w != niblack.width()) || (h != niblack.height()))
    {
        return niblack;
    }

    IntegralImage<uint32_t> niblack_bg_ii(w, h);
    IntegralImage<uint32_t> gray_bg_ii(w, h);

    uint32_t const* niblack_line = niblack.data();
    int const niblack_stride = niblack.wordsPerLine();
    uint8_t const* gray_line = gray.data();
    int const gray_stride = gray.stride();

    for (int y = 0; y < h; y++)
    {
        niblack_bg_ii.beginRow();
        gray_bg_ii.beginRow();
        for (int x = 0; x < w; x++)
        {
            // bg: 1, fg: 0
            uint32_t const niblack_inverted_pixel =
                (~niblack_line[x >> 5] >> (31 - (x & 31))) & uint32_t(1);
            uint32_t const gray_pixel = gray_line[x];
            niblack_bg_ii.push(niblack_inverted_pixel);

            // bg: gray_pixel, fg: 0
            gray_bg_ii.push(gray_pixel & ~(niblack_inverted_pixel - uint32_t(1)));
        }
        gray_line += gray_stride;
        niblack_line += niblack_stride;
    }

    QRect const image_rect(gray.rect());
    GrayImage background(gray);
    if (background.isNull())
    {
        return niblack;
    }

    uint8_t* background_line = background.data();
    int const background_stride = background.stride();

    uint64_t niblack_bg_sum = niblack_bg_ii.sum(image_rect);
    uint64_t src_size = w * h;
    if ((niblack_bg_sum == 0) || (niblack_bg_sum == src_size))
    {
        return niblack;
    }

    // sum(background - original) for foreground pixels according to Niblack.
    uint32_t sum_diff = 0;

    // sum(background) pixels for background pixels according to Niblack.
    uint32_t sum_bg = 0;

    niblack_line = niblack.data();
    unsigned int const w2 = w + w;
    unsigned int const h2 = h + h;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (binaryGetBW(niblack_line, x))
            {
                uint32_t niblack_sum_bg = 0;
                unsigned int wss = 0;
                QRect window;
                while ((niblack_sum_bg == 0) && ((wss < w2) || (wss < h2)))
                {
                    wss += ws;
                    window = QRect(0, 0, wss, wss);
                    window.moveCenter(QPoint(x, y));
                    window &= image_rect;
                    niblack_sum_bg = niblack_bg_ii.sum(window);
                }

                // Foreground pixel. Interpolate from background pixels in window.
                if (niblack_sum_bg > 0)
                {
                    uint32_t const gray_sum_bg = gray_bg_ii.sum(window);
                    uint32_t const bg = (gray_sum_bg + (niblack_sum_bg >> 1)) / niblack_sum_bg;
                    sum_diff += bg - background_line[x];
                    background_line[x] = bg;
                }
                else
                {
                    sum_diff += 255 - background_line[x];
                    background_line[x] = 255;
                }
            }
            else
            {
                sum_bg += background_line[x];
            }
        }
        background_line += background_stride;
        niblack_line += niblack_stride;
    }

    double const delta = double(sum_diff) / (src_size - niblack_bg_sum);
    double const b = double(sum_bg) / niblack_bg_sum;

    /*
        double const q = 0.6;
        double const p1 = 0.5;
        double const p2 = 0.8;
    */
    double const exp_scale = -4.0 / (b * (1.0 - p1));
    double const exp_bias = 2.0 * (1.0 + p1) / (1.0 - p1);
    double const threshold_scale = q * delta * (1.0 - p2);
    double const threshold_bias = q * delta * p2;

    rasterOpGeneric(
        [exp_scale, exp_bias, threshold_scale, threshold_bias]
        (uint8_t& gray, uint8_t const bg)
    {
        double const threshold = threshold_scale /
                                 (1.0 + exp(double(bg) * exp_scale + exp_bias)) + threshold_bias;
        gray = double(bg) - double(gray) > threshold ? 0x00 : 0xff;
    },
    gray, background
    );

    BinaryImage bw_img = BinaryImage(gray);

    return bw_img;
}

/*
 * niblack = mean - k * stderr, k = 0.2
 */
BinaryImage binarizeGatos(
    GrayImage const& src, int const radius,
    double const noise_sigma, double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper,
    double const q, double const p1, double const p2)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage wiener(grayWiener(src, 5, noise_sigma));
    BinaryImage niblack(binarizeNiblack(wiener, radius, k, delta, bound_lower, bound_upper));
    BinaryImage bw_img(binarizeGatosCleaner(wiener, niblack, radius, q, p1, p2));

    return bw_img;
}

/*
 * sauvola = mean * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
BinaryImage binarizeSauvola(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(graySauvolaMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

/*
 * wolf = mean - k * (mean - min_v) * (1.0 - stderr / stdmax), k = 0.3
 */
BinaryImage binarizeWolf(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayWolfMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

/*
 * bradley = mean * (1.0 - k), k = 0.2
 */
BinaryImage binarizeBradley(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayBradleyMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}  // binarizeBradley

/*
 * grad = mean * k + meanG * (1.0 - k), meanG = mean(I * G) / mean(G), G = |I - mean|, k = 0.75
 */
float binarizeGradValue(
    GrayImage const& gray, GrayImage const& gmean)
{
    float tvalue = 127.5f;

    if (gray.isNull() || gmean.isNull())
    {
        return tvalue;
    }

    int const w = gray.width();
    int const h = gray.height();

    if ((gmean.width() != w) || (gmean.height() != h))
    {
        return tvalue;
    }

    uint8_t const* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t const* gmean_line = gmean.data();
    int const gmean_stride = gmean.stride();

    double g, gi;
    double sum_g = 0.0, sum_gi = 0.0, sum_gl = 0.0, sum_gil = 0.0;

    for (int y = 0; y < h; y++)
    {
        sum_gl = 0.0;
        sum_gil = 0.0;
        for (int x = 0; x < w; x++)
        {
            gi = gray_line[x];
            g = gmean_line[x];
            g -= gi;
            g = (g < 0.0) ? -g : g;
            gi *= g;
            sum_gl += g;
            sum_gil += gi;
        }
        sum_g += sum_gl;
        sum_gi += sum_gil;
        gray_line += gray_stride;
        gmean_line += gmean_stride;
    }
    tvalue = (sum_g == 0.0) ? tvalue : (sum_gi / sum_g);

    return tvalue;
}

BinaryImage binarizeGrad(
    GrayImage const& src, int const radius,
    double const coef, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayGradMap(src, radius, coef));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

/*
 * singh = (1.0 - k) * (mean + (max - min) * (1.0 - img / 255.0)), k = 0.2
 */
BinaryImage binarizeSingh(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(graySinghMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}  // binarizeSingh

/*
 * WAN = (mean + max) / 2 * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
BinaryImage binarizeWAN(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayWANMap(src, radius, k));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

BinaryImage binarizeEdgeDiv(
    GrayImage const& src, int const radius,
    double const kep, double const kbd, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(grayEdgeDiv(src, radius, kep, kbd));
    BinaryImage bw_img(binarizeBiModal(gray, delta, bound_lower, bound_upper));

    return bw_img;
}  // binarizeEdgeDiv

/*
 * Robust = 255.0 - (surround + 255.0) * sc / (surround + sc), k = 0.2
 * sc = surround - img
 * surround = blur(img, r), r = 10
 */
BinaryImage binarizeRobust(
    GrayImage const& src, int const radius,
    double const k, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(grayRobust(src, radius, k));
    BinaryImage bw_img(binarizeBiModal(gray, delta, bound_lower, bound_upper));

    return bw_img;
} // binarizeRobust

BinaryImage binarizeMScale(
    GrayImage const& src, int const radius,
    double const coef, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayMScaleMap(src, radius, coef));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}  // binarizeMScale

BinaryImage binarizeEngraving(
    GrayImage const& src, int const radius,
    double const coef, int const delta,
    unsigned char const bound_lower, unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayEngravingMap(src, radius, coef));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, delta, bound_lower, bound_upper));

    return bw_img;
}

} // namespace imageproc
