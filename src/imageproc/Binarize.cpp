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
    QImage const& src,
    unsigned const max_edge_width,
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
    GrayImage const& src,
    unsigned int const threshold)
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
    GrayImage const& src,
    GrayImage const& threshold,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    GrayImage const& src,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    unsigned int threshold = 128;
    if (src.isNull())
    {
        return threshold;
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    threshold = grayBiModalTiledValue(src, 0, 0, w, h, delta, bound_lower, bound_upper);

    return threshold;
}  // binarizeBiModalValue

BinaryImage binarizeBiModal(
    GrayImage const& src,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    GrayImage const& src,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    double meanl = 0, meanw = 0.0;
    double dist, dist_mean = 0, threshold = 128;

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
    GrayImage const& src,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayDotsMap(src, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}  // binarizeDots

BinaryImage binarizeImageToDots(
    GrayImage const& src,
    BinaryImage const& mask,
    BinaryImage const& mask_bw)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    BinaryImage bw_img(binarizeDots(src));
    if (bw_img.isNull())
    {
        return BinaryImage();
    }

    if (mask.isNull() || mask_bw.isNull())
    {
        return bw_img;
    }

    int const w = bw_img.width();
    int const h = bw_img.height();

    if ((w != mask.width()) || (h != mask.height()) || (w != mask_bw.width()) || (h != mask_bw.height()))
    {
        return bw_img;
    }

    uint32_t* bw_img_line = bw_img.data();
    int const bw_img_stride = bw_img.wordsPerLine();
    uint32_t const* mask_line = mask.data();
    int const mask_stride = mask.wordsPerLine();
    uint32_t const* mask_bw_line = mask_bw.data();
    int const mask_bw_stride = mask_bw.wordsPerLine();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (binaryGetBW(mask_bw_line, x))
            {
                binarySetBW(bw_img_line, x, binaryGetBW(mask_line, x));
            }
        }
        bw_img_line += bw_img_stride;
        mask_line += mask_stride;
        mask_bw_line += mask_bw_stride;
    }

    return bw_img;
}  // binarizeDots

BinaryImage binarizeBMTiled(
    GrayImage const& src,
    int const radius,
    float const coef,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayBiModalTiledMap(src, radius, coef, delta, bound_lower, bound_upper));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * niblack = mean - k * stderr, k = 0.2
 * modification by zvezdochiot:
 * niblack = mean - k * (stderr - delta), k = 0.2, delta = 0
 */
BinaryImage binarizeNiblack(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayNiblackMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * gatos = bg - f(i, bg, q, p), q = 0.6, p = 0.2
 * modification by zvezdochiot:
 * q = q - (1 - q) * delta * 2 / (max_delta - min_delta), delta = 0
 */
GrayImage binarizeGatosBG(
    GrayImage const& src,
    BinaryImage const& niblack,
    int const radius)
{
    GrayImage gray = GrayImage(src);
    if (gray.isNull() || niblack.isNull())
    {
        return GrayImage();
    }
    if (radius < 1)
    {
        return gray;
    }

    int const w = src.width();
    int const h = src.height();
    unsigned int const ws = radius + radius + 1;

    if ((w != niblack.width()) || (h != niblack.height()))
    {
        return gray;
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

    GrayImage background(gray);
    if (background.isNull())
    {
        return gray;
    }
    uint8_t* background_line = background.data();
    int const background_stride = background.stride();

    QRect const image_rect(gray.rect());

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
                uint32_t bg = background_line[x];
                if (niblack_sum_bg > 0)
                {
                    uint32_t const gray_sum_bg = gray_bg_ii.sum(window);
                    bg = (gray_sum_bg + (niblack_sum_bg >> 1)) / niblack_sum_bg;
                }
                background_line[x] = bg;
            }
        }
        background_line += background_stride;
        niblack_line += niblack_stride;
    }

    return background;
}

BinaryImage binarizeGatos(
    GrayImage const& src,
    int const radius,
    float const noise_sigma,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper,
    float const q,
    float const p)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage const wiener(grayWiener(src, 5, noise_sigma));
    BinaryImage niblack(binarizeNiblack(wiener, radius, k, 0, bound_lower, bound_upper));
    GrayImage threshold_map(binarizeGatosBG(wiener, niblack, radius));
    float const qd = q - (1.0f - q) * delta * 0.02f;
    grayBGtoMap(wiener, threshold_map, qd, p);
    BinaryImage bw_img(binarizeFromMap(wiener, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * sauvola = mean * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 * modification by zvezdochiot:
 * sauvola = mean * (1.0 + k * ((stderr + delta) / 128.0 - 1.0)), k = 0.34, delta = 0
 */
BinaryImage binarizeSauvola(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(graySauvolaMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * wolf = mean - k * (mean - min_v) * (1.0 - stderr / stdmax), k = 0.3
 * modification by zvezdochiot:
 * wolf = mean - k * (mean - min_v) * (1.0 - (stderr / stdmax + delta / 128.0)), k = 0.3, delta = 0
 */
BinaryImage binarizeWolf(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayWolfMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * bradley = mean * (1.0 - k), k = 0.2
 */
BinaryImage binarizeBradley(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
 * nick = mean - k * sqrt(stdev * stdev + cnick * mean * mean), k = 0.10
 * modification by zvezdochiot:
 * cnick = (max_delta - delta) / (max_delta - min_delta);
 */
BinaryImage binarizeNick(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayNickMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * grad = mean * k + meanG * (1.0 - k), meanG = mean(I * G) / mean(G), G = |I - mean|, k = 0.75
 * modification by zvezdochiot:
 * mean = mean + delta, delta = 0
 */
float binarizeGradValue(
    GrayImage const& gray,
    GrayImage const& gmean)
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

    double g, gi, gm;
    double sum_g = 0.0, sum_gi = 0.0, sum_gl = 0.0, sum_gil = 0.0;

    for (int y = 0; y < h; y++)
    {
        sum_gl = 0.0;
        sum_gil = 0.0;
        for (int x = 0; x < w; x++)
        {
            gi = gray_line[x];
            gm = gmean_line[x];
            g = gi;
            g -= gm;
            g = (g < 0.0) ? -g : g;
            gm *= g;
            sum_gl += g;
            sum_gil += gm;
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
    GrayImage const& src,
    int const radius,
    float const coef,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayGradMap(src, radius, coef, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

/*
 * singh = mean * (1.0 - k * (1.0 - dI / (256 - dI))), k = 0.2
 * dI = origin - mean
 * modification by zvezdochiot:
 * singh = mean * (1.0 - k * (1.0 - (dI + delta) / (256 - dI))), k = 0.2, delta = 0
 */
BinaryImage binarizeSingh(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(graySinghMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}  // binarizeSingh

/*
 * WAN = (mean + max) / 2 * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 * modification by zvezdochiot:
 * WAN = (mean + max) / 2 * (1.0 + k * ((stderr + delta) / 128.0 - 1.0)), k = 0.34, delta = 0
 */
BinaryImage binarizeWAN(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(grayWANMap(src, radius, k, delta));
    BinaryImage bw_img(binarizeFromMap(src, threshold_map, 0, bound_lower, bound_upper));

    return bw_img;
}

BinaryImage binarizeEdgeDiv(
    GrayImage const& src,
    int const radius,
    float const kep,
    float const kbd,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(grayRobust(src, radius, k));
    BinaryImage bw_img(binarizeBiModal(gray, delta, bound_lower, bound_upper));

    return bw_img;
} // binarizeRobust

/*
 * Grain:
 * I
 * B = BLUR(I,r)
 * D = GRAIN(I,B) {d = i-b+127}
 * S = BLUR(D,r)
 * N = GRAIN(S,D) {n = s-d+127}
 * T = GRAIN(D,N) {t = d-n+127} {t = d-s+d-127+127 = 2d-s}
 * O = k * T + (1 - k) * I
 */
BinaryImage binarizeGrain(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(grayGrain(src, radius, k));
    BinaryImage bw_img(binarizeBiModal(gray, delta, bound_lower, bound_upper));

    return bw_img;
} // binarizeGrain

BinaryImage binarizeMScale(
    GrayImage const& src,
    int const radius,
    float const coef,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    GrayImage const& src,
    int const radius,
    float const coef,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
