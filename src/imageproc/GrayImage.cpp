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

#include <new>
#include <stdexcept>
#include "GrayImage.h"
#include "Grayscale.h"

namespace imageproc
{

GrayImage::GrayImage(QSize size)
{
    if (size.isEmpty())
    {
        return;
    }

    m_image = QImage(size, QImage::Format_Indexed8);
    m_image.setColorTable(createGrayscalePalette());
    if (m_image.isNull())
    {
        throw std::bad_alloc();
    }
}

GrayImage::GrayImage(QImage const& image)
    :   m_image(toGrayscale(image))
{
}

GridAccessor<unsigned char const>
GrayImage::accessor() const
{
    return GridAccessor<unsigned char const> {data(), stride(), width(), height()};
}

GridAccessor<unsigned char>
GrayImage::accessor()
{
    return GridAccessor<unsigned char> {data(), stride(), width(), height()};
}

/*
 * mean, w = 200
 */
GrayImage grayMapMean(
    GrayImage const& src,
    int const radius)
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

    if (radius > 0)
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char const* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char* gray_line = gray.data();
        int const gray_stride = gray.stride();

        IntegralImage<uint32_t> integral_image(w, h);

        for (int y = 0; y < h; y++)
        {
            integral_image.beginRow();
            for (int x = 0; x < w; x++)
            {
                uint32_t const pixel = src_line[x];
                integral_image.push(pixel);
            }
            src_line += src_stride;
        }

        for (int y = 0; y < h; y++)
        {
            int const top = ((y - radius) < 0) ? 0 : (y - radius);
            int const bottom = ((y + radius + 1) < h) ? (y + radius + 1) : h;

            for (int x = 0; x < w; x++)
            {
                int const left = ((x - radius) < 0) ? 0 : (x - radius);
                int const right = ((x + radius) < w) ? (x + radius) : w;
                int const area = (bottom - top) * (right - left);
                assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0

                QRect const rect(left, top, right - left, bottom - top);
                double const window_sum = integral_image.sum(rect);

                double const r_area = 1.0 / area;
                double mean = window_sum * r_area;

                mean += 0.5;
                mean = (mean < 0.0) ? 0.0 : ((mean < 255.0) ? mean : 255.0);
                gray_line[x] = (unsigned char) mean;
            }
            gray_line += gray_stride;
        }
    }

    return gray;
}  // grayMapMean

/*
 * stdev, w = 200
 */
GrayImage grayMapDeviation(
    GrayImage const& src,
    int const radius)
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

    if (radius > 0)
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char const* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char* gray_line = gray.data();
        int const gray_stride = gray.stride();

        IntegralImage<uint32_t> integral_image(w, h);
        IntegralImage<uint64_t> integral_sqimage(w, h);

        uint32_t min_gray_level = 255;

        for (int y = 0; y < h; y++)
        {
            integral_image.beginRow();
            integral_sqimage.beginRow();
            for (int x = 0; x < w; x++)
            {
                uint32_t const pixel = src_line[x];
                integral_image.push(pixel);
                integral_sqimage.push(pixel * pixel);
                min_gray_level = std::min(min_gray_level, pixel);
            }
            src_line += src_stride;
        }

        for (int y = 0; y < h; y++)
        {
            int const top = ((y - radius) < 0) ? 0 : (y - radius);
            int const bottom = ((y + radius + 1) < h) ? (y + radius + 1) : h;

            for (int x = 0; x < w; x++)
            {
                int const left = ((x - radius) < 0) ? 0 : (x - radius);
                int const right = ((x + radius + 1) < w) ? (x + radius + 1) : w;
                int const area = (bottom - top) * (right - left);
                assert(area > 0); // because window_size > 0 and w > 0 and h > 0

                QRect const rect(left, top, right - left, bottom - top);
                double const window_sum = integral_image.sum(rect);
                double const window_sqsum = integral_sqimage.sum(rect);

                double const r_area = 1.0 / area;
                double const mean = window_sum * r_area;
                double const sqmean = window_sqsum * r_area;

                double const variance = sqmean - mean * mean;
                double deviation = sqrt(fabs(variance));

                deviation += 0.5;
                deviation = (deviation < 0.0) ? 0.0 : ((deviation < 255.0) ? deviation : 255.0);
                gray_line[x] = (unsigned char) deviation;
            }
            gray_line += gray_stride;
        }
    }

    return gray;
} // grayMapDeviation

GrayImage grayMapMax(
    GrayImage const& src,
    int const radius)
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

    if (radius > 0)
    {
        GrayImage gmax = GrayImage(src);
        if (gmax.isNull())
        {
            return gray;
        }
        int const w = src.width();
        int const h = src.height();
        unsigned char* gray_line = gray.data();
        int const gray_stride = gray.stride();
        unsigned char* gmax_line = gmax.data();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                int const left = ((x - radius) < 0) ? 0 : (x - radius);
                int const right = ((x + radius + 1) < w) ? (x + radius + 1) : w;

                unsigned char const origin = gray_line[x];
                unsigned char immax = origin;
                for (int xf = left; xf < right; xf++)
                {
                    unsigned char imf = gray_line[xf];
                    if (imf > immax)
                    {
                        immax = imf;
                    }
                }
                gmax_line[x] = immax;
            }
            gray_line += gray_stride;
            gmax_line += gray_stride;
        }

        gmax_line = gmax.data();
        gray_line = gray.data();
        for (int y = 0; y < h; y++)
        {
            int const top = ((y - radius) < 0) ? 0 : (y - radius);
            int const bottom = ((y + radius + 1) < h) ? (y + radius + 1) : h;

            for (int x = 0; x < w; x++)
            {
                unsigned char const origin = gray_line[x];
                unsigned char immax = origin;
                unsigned long int idx = top * gray_stride + x;
                for (int yf = top; yf < bottom; yf++)
                {
                    unsigned char immaxf = gmax_line[idx];
                    if (immaxf > immax)
                    {
                        immax = immaxf;
                    }
                    idx += gray_stride;
                }
                gray_line[x] = immax;
            }
            gray_line += gray_stride;
        }
    }

    return gray;
}  // grayMapMax

GrayImage grayMapContrast(
    GrayImage const& src,
    int const radius)
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

    if (radius > 0)
    {
        GrayImage gmax = GrayImage(src);
        if (gmax.isNull())
        {
            return gray;
        }
        GrayImage gmin = GrayImage(src);
        if (gmin.isNull())
        {
            return gray;
        }
        int const w = src.width();
        int const h = src.height();
        unsigned char* gray_line = gray.data();
        int const gray_stride = gray.stride();
        unsigned char* gmax_line = gmax.data();
        unsigned char* gmin_line = gmin.data();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                int const left = ((x - radius) < 0) ? 0 : (x - radius);
                int const right = ((x + radius + 1) < w) ? (x + radius + 1) : w;

                unsigned char const origin = gray_line[x];
                unsigned char immin = origin;
                unsigned char immax = origin;
                for (int xf = left; xf < right; xf++)
                {
                    unsigned char imf = gray_line[xf];
                    if (imf > immax)
                    {
                        immax = imf;
                    }
                    if (imf < immin)
                    {
                        immin = imf;
                    }
                }
                gmax_line[x] = immax;
                gmin_line[x] = immin;
            }
            gray_line += gray_stride;
            gmax_line += gray_stride;
            gmin_line += gray_stride;
        }

        gmax_line = gmax.data();
        gmin_line = gmin.data();
        gray_line = gray.data();
        for (int y = 0; y < h; y++)
        {
            int const top = ((y - radius) < 0) ? 0 : (y - radius);
            int const bottom = ((y + radius + 1) < h) ? (y + radius + 1) : h;

            for (int x = 0; x < w; x++)
            {
                unsigned char const origin = gray_line[x];
                unsigned char immin = origin;
                unsigned char immax = origin;
                unsigned long int idx = top * gray_stride + x;
                for (int yf = top; yf < bottom; yf++)
                {
                    unsigned char immaxf = gmax_line[idx];
                    unsigned char imminf = gmin_line[idx];
                    if (immaxf > immax)
                    {
                        immax = immaxf;
                    }
                    if (imminf < immin)
                    {
                        immin = imminf;
                    }
                    idx += gray_stride;
                }
                unsigned char threshold = (immax - immin);

                gray_line[x] = threshold;
            }
            gray_line += gray_stride;
        }
    }

    return gray;
}  // grayMapContrast

/*
 * bimodaltiled = k * BML + (1 - k) * BMG, k = 0.75
 *     BML = bilinescale(bimodalvalue(I(w x w)), w = 2 * r + 1, r = 50
 *     BMG = bimodalvalue(I)
 */
unsigned int grayBiModalTiledValue(
    GrayImage const& src,
    unsigned int const x1,
    unsigned int const y1,
    unsigned int const x2,
    unsigned int const y2,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
{
    unsigned int const histsize = 256;
    unsigned int threshold = histsize >> 1;
    if (src.isNull())
    {
        return threshold;
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    unsigned char const* src_line = src.data();
    unsigned int const src_stride = src.stride();
    uint64_t im, iw, ib, histogram[histsize] = {0};
    unsigned int k, Tn;
    uint64_t Tw, Tb;
    int Tmin = 255, Tmax = 0, Ti;
    int part = 256 + delta + delta;
    part = (part < 0) ? 0 : (part < 512) ? part : 512;

    unsigned int const x1w = (x1 < w) ? x1 : w;
    unsigned int const y1w = (y1 < h) ? y1 : h;
    unsigned int const x2w = (x2 < w) ? x2 : w;
    unsigned int const y2w = (y2 < h) ? y2 : h;
    uint64_t const wsize = (y2w - y1w) * (x2w - x1w);

    if (wsize > 0)
    {
        src_line += y1w * src_stride;
        for (unsigned int y = y1w; y < y2w; y++)
        {
            for (unsigned int x = x1w; x < x2w; x++)
            {
                unsigned char pixel = src_line[x];
                pixel = (pixel < bound_lower) ? bound_lower : (pixel <= bound_upper) ? pixel : bound_upper;
                histogram[pixel]++;
            }
            src_line += src_stride;
        }

        Ti = 0;
        while (Ti < Tmin)
        {
            Tmin = (histogram[Ti] > 0) ? Ti : Tmin;
            Ti++;
        }
        Ti = 255;
        while (Ti > Tmax)
        {
            Tmax = (histogram[Ti] > 0) ? Ti : Tmax;
            Ti--;
        }
        Tmax++;
        Tmin <<= 7;
        Tmax <<= 7;

        threshold = part * Tmax + (512 - part) * Tmin;
        threshold >>= 8;
        Tn = threshold + 1;
        while (threshold != Tn)
        {
            Tn = threshold;
            Tb = Tw = 0;
            ib = iw = 0;
            for (k = 0; k < histsize; k++)
            {
                im = histogram[k];
                if ((k << 8) < threshold)
                {
                    Tb += im * k;
                    ib += im;
                }
                else
                {
                    Tw += im * k;
                    iw += im;
                }
            }
            Tb = (ib > 0) ? ((Tb << 7) / ib) : Tmin;
            Tw = (iw > 0) ? ((Tw << 7) / iw) : Tmax;
            threshold = ((part * Tw + (512 - part) * Tb + 127) >> 8);
        }
        threshold += 127;
        threshold >>= 8;
    }

    return threshold;
}  // grayBiModalTiledValue

GrayImage grayBiModalTiledMap(
    GrayImage const& src,
    int const radius,
    float const coef,
    int const delta,
    unsigned char const bound_lower,
    unsigned char const bound_upper)
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
    unsigned char* gray_line = gray.data();
    int const gray_stride = gray.stride();

    int const w = src.width();
    int const h = src.height();
    unsigned int const bmg = grayBiModalTiledValue(src, 0, 0, w, h, delta, bound_lower, bound_upper);

    if (radius > 0)
    {
        int const wsize = 2 * radius + 1;
        int const ww = (w + wsize - 1) / wsize;
        int const wh = (h + wsize - 1) / wsize;

        GrayImage tlocal = GrayImage(QSize(ww, wh));
        if (tlocal.isNull())
        {
            return GrayImage();
        }

        unsigned char* tlocal_line = tlocal.data();
        int const tlocal_stride = tlocal.stride();

        for (int wy = 0; wy < wh; wy++)
        {
            int const y1 = wy * wsize;
            int const y2 = ((y1 + wsize) < h) ? (y1 + wsize) : h;
            for (int wx = 0; wx < ww; wx++)
            {
                int const x1 = wx * wsize;
                int const x2 = ((x1 + wsize) < w) ? (x1 + wsize) : w;

                int const t = grayBiModalTiledValue(gray, x1, y1, x2, y2, delta, bound_lower, bound_upper);

                tlocal_line[wx] = (unsigned char) t;
            }
            tlocal_line += tlocal_stride;
        }

        /* biline scale */
        tlocal_line = tlocal.data();
        for (int y = 0; y < h; y++)
        {
            float const yd = (0.5f + y) / wsize - 0.5f;
            int y1 = yd;
            int y2 = y1 + 1;
            float const dy1 = yd - y1;
            float const dy2 = 1.0f - dy1;
            y1 = (y1 < 0) ? 0 : ((y1 < wh) ? y1 : (wh - 1));
            y2 = (y2 < 0) ? 0 : ((y2 < wh) ? y2 : (wh - 1));
            for (int x = 0; x < w; x++)
            {
                float const xd = (0.5f + x) / wsize - 0.5f;
                int x1 = xd;
                int x2 = x1 + 1;
                float const dx1 = xd - x1;
                float const dx2 = 1.0f - dx1;
                x1 = (x1 < 0) ? 0 : ((x1 < ww) ? x1 : (ww - 1));
                x2 = (x2 < 0) ? 0 : ((x2 < ww) ? x2 : (ww - 1));

                float const t11 = tlocal_line[y1 * tlocal_stride + x1];
                float const t12 = tlocal_line[y1 * tlocal_stride + x2];
                float const t21 = tlocal_line[y2 * tlocal_stride + x1];
                float const t22 = tlocal_line[y2 * tlocal_stride + x2];

                float const bml = dy2 * (dx2 * t11 + dx1 * t12)
                                + dy1 * (dx2 * t21 + dx1 * t22);
                float t = coef * bml + (1.0f - coef) * bmg + 0.5f;
                t = (t < 0.0f) ? 0.0f : ((t < 255.0f) ? t : 255.0f);
                gray_line[x] = (unsigned char) t;
            }
            gray_line += gray_stride;
        }
    }
    else
    {
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                gray_line[x] = (unsigned char) bmg;
            }
            gray_line += gray_stride;
        }
    }

    return gray;
}  // grayBiModalTiledMap

/*
 * niblack = mean - k * stderr, k = 0.2
 */
GrayImage grayNiblackMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float threshold = mean - k * deviation;

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }

    return gmean;
}

/*
 * gatos = bg - f(i, bg, q, p), q = 0.6, p = 0.2
 */
void grayBGtoMap(
    GrayImage const& src,
    GrayImage& background,
    float const q,
    float const p)
{
    if (src.isNull() || background.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();

    if ((w != background.width()) || (h != background.height()))
    {
        return;
    }
    unsigned char const* src_line = src.data();
    int const src_stride = src.stride();
    unsigned char* background_line = background.data();
    int const background_stride = background.stride();
    int const hist_size = 256;
    unsigned char histogram[hist_size] = {0};

    // sum(background - original) for foreground pixels (background > original).
    uint64_t sum_diff = 0;
    // sum(background) for background pixels (background <= original).
    uint64_t sum_bg = 0;
    uint64_t sum_contour = 0;
    uint64_t src_size = w * h;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int const im = src_line[x];
            int const bg = background_line[x];
            if (im < bg)
            {
                sum_diff += (bg - im);
                sum_contour++;
            }
            else
            {
                sum_bg += bg;
            }
        }
        src_line += src_stride;
        background_line += background_stride;
    }

    if ((sum_contour == 0) || (sum_contour == src_size))
    {
        return;
    }

    float const d = ((float) sum_diff) / sum_contour;
    float const b = ((float) sum_bg) / (src_size - sum_contour);

    /*
        double const q = 0.6;
        double const p = 0.2;
    */
    float const qd = q * d;
    float const threshold_scale = qd * p;
    float const threshold_bias = qd * (1.0f - p);

    for (int k = 0; k < hist_size; k++)
    {
        float const bgf = ((float) k + 1.0f) / (b + 1.0f);
        float const tk = 1.0f / (1.0f + expf(-8.0f * bgf + 6.0f));
        int const threshold = (threshold_scale * tk + threshold_bias + 0.5f);
        int bgn = k - threshold;
        bgn = (bgn < 0) ? 0 : ((bgn < 255) ? bgn : 255);
        histogram[k] = (unsigned char) bgn;
    }

    background_line = background.data();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned char const bg = background_line[x];
            unsigned char const bgn = histogram[bg];
            background_line[x] = bgn;
        }
        background_line += background_stride;
    }
}

/*
 * sauvola = mean * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
GrayImage graySauvolaMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];

                float threshold = mean * (1.0f + k * (deviation / 128.0f - 1.0f));

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }

    return gmean;
}

/*
 * wolf = mean - k * (mean - min_v) * (1.0 - stderr / stdmax), k = 0.3
 */
GrayImage grayWolfMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char const* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        uint32_t min_gray_level = 255;
        float max_deviation = 0.0f;

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                uint32_t origin = src_line[x];
                float const deviation = gdeviation_line[x];
                max_deviation = (max_deviation < deviation) ? deviation : max_deviation;
                min_gray_level = (min_gray_level < origin) ? min_gray_level : origin;
            }
            src_line += src_stride;
            gdeviation_line += gdeviation_stride;
        }

        gmean_line = gmean.data();
        gdeviation_line = gdeviation.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float const a = 1.0f - deviation / max_deviation;
                float threshold = mean - k * a * (mean - min_gray_level);

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }

    return gmean;
}

/*
 * bradley = mean * (1.0 - k), k = 0.2
 */
GrayImage grayBradleyMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float threshold = (k < 1.0f) ? (mean * (1.0f - k)) : 0.0f;

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
        }
    }

    return gmean;
}  // grayBradleyMap

/*
 * nick = mean - k * sqrt(stdev * stdev + cnick * mean * mean), k = 0.10
 */
GrayImage grayNickMap(
    GrayImage const& src,
    int const radius,
    float const k,
    int const delta)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        float cnick = (50.0f - delta) * 0.01f;
        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float const circle = sqrtf(deviation * deviation + cnick * mean * mean);
                float threshold = mean - k * circle;

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }

    return gmean;
}

/*
 * grad = mean * k + meanG * (1.0 - k), meanG = mean(I * G) / mean(G), G = |I - mean|, k = 0.75
 */
GrayImage grayGradMap(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = gaussBlur(src, radius, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        float const mean_grad = (1.0f - coef) * binarizeGradValue(src, gmean);

        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];

                float threshold = mean_grad + mean * coef;

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
        }
    }

    return gmean;
}

/*
 * singh = (1.0 - k) * (mean + (max - min) * (1.0 - img / 255.0)), k = 0.2
 */
GrayImage graySinghMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        GrayImage graymm = grayMapContrast(src, radius);
        if (graymm.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char const* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* graymm_line = graymm.data();
        int const graymm_stride = graymm.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float const maxmin = graymm_line[x];
                float threshold = (1.0f - k) * (mean + maxmin * (1.0f - origin / 255.0f));

                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            graymm_line += graymm_stride;
        }
    }

    return gmean;
}  // graySinghMap

/*
 * WAN = (mean + max) / 2 * (1.0 + k * (stderr / 128.0 - 1.0)), k = 0.34
 */
GrayImage grayWANMap(
    GrayImage const& src,
    int const radius,
    float const k)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = grayMapMean(src, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if (radius > 0)
    {
        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return gmean;
        }
        GrayImage gmax = grayMapMax(src, radius);
        if (gmax.isNull())
        {
            return gmean;
        }

        int const w = src.width();
        int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();
        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();
        unsigned char* gmax_line = gmax.data();
        int const gmax_stride = gmax.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float const imax = gmax_line[x];

                float threshold = (mean + imax) * 0.5f * (1.0f + k * (deviation / 128.0f - 1.0f));
                threshold = (threshold < 0.0f) ? 0.0f : ((threshold < 255.0f) ? threshold : 255.0f);
                gmean_line[x] = (unsigned char) threshold;
            }
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
            gmax_line += gmax_stride;
        }
    }

    return gmean;
}

GrayImage grayMScaleMap(
    GrayImage const& src,
    int const radius,
    float const coef)
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
    if (radius < 1)
    {
        return gray;
    }

    int const w = src.width();
    int const h = src.height();
    unsigned char const* src_line = src.data();
    int const src_stride = src.stride();
    unsigned char* gray_line = gray.data();
    int const gray_stride = gray.stride();

    unsigned int whcp, l, i, j, blsz, rsz;
    float immean, imt, kover, sensitivity, sensdiv, senspos, sensinv;
    unsigned int pim, immin, immax, cnth, cntw, level = 0;
    unsigned int maskbl, maskover, tim;
    unsigned long int idx;

    whcp = (h + w) >> 1;
    blsz = 1;
    while (blsz < whcp)
    {
        level++;
        blsz <<= 1;
    }
    blsz >>= 1;
    rsz = 1;
    while (((int)rsz < radius) && (level > 1))
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
    immean = (float) (immax + immin);
    immean *= 0.5f;
    immean += 0.5f;
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

    kover = 1.5f;

    if (coef < 0.0f)
    {
        sensitivity = -coef;
        sensdiv = sensitivity;
        sensdiv += 1.0f;
        sensinv = 1.0f / sensdiv;
        senspos = sensitivity / sensdiv;
    }
    else
    {
        sensitivity = coef;
        sensdiv = sensitivity;
        sensdiv += 1.0f;
        senspos = 1.0f / sensdiv;
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
                immean = (float) (immax + immin);
                immean *= 0.5f;
                immean *= sensinv;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        idx = y * gray_stride + x;
                        imt = gray_line[idx];
                        imt *= senspos;
                        imt += immean;
                        imt += 0.5f;
                        imt = (imt < 0.0f) ? 0.0f : ((imt < 255.0f) ? imt : 255.0f);
                        gray_line[idx] = (unsigned char) imt;
                    }
                }
            }
        }
        blsz >>= 1;
    }

    return gray;
}  // grayMScaleMap

GrayImage grayEngravingMap(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return GrayImage();
    }
    GrayImage gmean = gaussBlur(src, radius, radius);
    if (gmean.isNull())
    {
        return GrayImage(src);
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char const* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        double mean_delta = 0.0;
        for (int y = 0; y < h; y++)
        {
            double mean_delta_line = 0.0;
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];

                float const delta = (origin < mean) ? (mean - origin) : (origin - mean);
                mean_delta_line += delta;
            }
            mean_delta += mean_delta_line;
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
        mean_delta /= w;
        mean_delta /= h;
        if (mean_delta > 0.0)
        {
            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    float const mean = gmean_line[x];

                    float const tline = mean / mean_delta;
                    int threshold = (int) tline;
                    float delta = tline - threshold;
                    delta = (delta < 0.5f) ? (0.5f - delta) : (delta - 0.5f);
                    delta += delta;
                    /* overlay */
                    float retval = mean;
                    if (mean > 127.5f)
                    {
                        retval = 255.0f - retval;
                        delta = 1.0f - delta;
                    }
                    retval *= delta;
                    retval += retval;
                    if (mean > 127.5f)
                    {
                        retval = 255.0f - retval;
                    }
                    retval = coef * retval + (1.0f - coef) * mean + 0.5f;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                    gmean_line[x] = (unsigned char) retval;
                }
                gmean_line += gmean_stride;
            }
        }
    }

    return gmean;
}  // grayEngraving

GrayImage grayDotsMap (
    GrayImage const& src,
    int const delta)
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
    unsigned char* gray_line = gray.data();
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

    for (y = 0; y < h; y++)
    {
        yb = y % wwidth;
        for (x = 0; x < w; x++)
        {
            xb = x % wwidth;
            threshold = ddith[yb * wwidth + xb];
            gray_line[x] = (unsigned char) threshold;
        }
        gray_line += gray_stride;
    }

    return gray;
}

// threshold MAPs as filters

void grayMapOverlay(
    GrayImage& src,
    GrayImage& gover,
    float const coef)
{
    if ((src.isNull()) || (gover.isNull()))
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    unsigned char* src_line = src.data();
    int const src_stride = src.stride();

    if ((gover.width() != w) || (gover.height() != h))
    {
        return;
    }
    unsigned char* gover_line = gover.data();
    int const gover_stride = gover.stride();

    if (coef != 0.0f)
    {
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float overlay = gover_line[x];
                float base = 255.0f - origin;
                /* overlay */
                float retval = base;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }
                retval *= overlay;
                retval += retval;
                retval /= 255.0f;
                if (base < 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }
                /* overlay */
                base = retval;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }
                retval *= overlay;
                retval += retval;
                retval /= 255.0f;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }

                retval = coef * retval + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gover_line += gover_stride;
        }
    }
} // grayMapOverlay

GrayImage grayCurveFilter(
    GrayImage& src, float const coef)
{
    GrayImage dst(src);
    grayCurveFilterInPlace(dst, coef);
    return dst;
}

void grayCurveFilterInPlace(
    GrayImage& src, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int icoef = (int) (coef * 256.0f + 0.5f);
        unsigned int const w = src.width();
        unsigned int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char pix_replace[256];

        uint64_t thres = 0;
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char const val = src_line[x];
                thres += val;
            }
            src_line += src_stride;
        }

        thres <<= 8; /* no round */
        thres /= h;
        thres /= w;

        for (unsigned int j = 0; j < 256; j++)
        {
            int64_t val = (j << 8);
            int64_t delta = (val - thres);
            int64_t dsqr = delta * delta;
            int64_t ddiv = (delta < 0) ? -thres : (65280 - thres);
            dsqr = (ddiv == 0) ? 0 : dsqr / ddiv;
            delta -= dsqr;
            delta *= icoef;
            delta += 128;
            delta >>= 8;
            val += delta;
            val += 128;
            val >>= 8;
            pix_replace[j] = (unsigned char) val;
        }

        src_line = src.data();
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char const val = src_line[x];
                src_line[x] = pix_replace[val];
            }
            src_line += src_stride;
        }
    }
}

GrayImage graySqrFilter(
    GrayImage& src, float const coef)
{
    GrayImage dst(src);
    graySqrFilterInPlace(dst, coef);
    return dst;
}

void graySqrFilterInPlace(
    GrayImage& src, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int icoef = (int) (coef * 256.0f + 0.5f);
        unsigned int const w = src.width();
        unsigned int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();
        unsigned char pix_replace[256];

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
            pix_replace[j] = (unsigned char) val;
        }

        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char const val = src_line[x];
                src_line[x] = pix_replace[val];
            }
            src_line += src_stride;
        }
    }
}

GrayImage grayGravure(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayGravureInPlace(dst, radius, coef);
    return dst;
}

void grayGravureInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gover = grayEngravingMap(src, radius, 1.0f);
        if (gover.isNull())
        {
            return;
        }

        grayMapOverlay(src, gover, coef);
    }

}  // grayGravure

GrayImage grayDots8(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayDots8InPlace(dst, radius, coef);
    return dst;
}

void grayDots8InPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        GrayImage gover = grayDotsMap(gmean, 0);
        if (gover.isNull())
        {
            return;
        }

        grayMapOverlay(src, gover, coef);
    }
}  // grayDots8

GrayImage grayScaleByLine(
    GrayImage& src,
    int const wnew,
    int const hnew)
{
    if (src.isNull())
    {
        return GrayImage();
    }

    if ((wnew > 0) && (hnew > 0))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        float const scalew = (float)w / wnew;
        float const scaleh = (float)h / hnew;

        GrayImage gscale = GrayImage(QSize(wnew, hnew));
        if (gscale.isNull())
        {
            return GrayImage(src);
        }
        unsigned char* gscale_line = gscale.data();
        int const gscale_stride = gscale.stride();

        /* biline scale */
        for (int y = 0; y < hnew; y++)
        {
            float const yd = (0.5f + y) * scaleh - 0.5f;
            int y1 = yd;
            int y2 = y1 + 1;
            float const dy1 = yd - y1;
            float const dy2 = 1.0f - dy1;
            y1 = (y1 < 0) ? 0 : ((y1 < h) ? y1 : (h - 1));
            y2 = (y2 < 0) ? 0 : ((y2 < h) ? y2 : (h - 1));
            for (int x = 0; x < wnew; x++)
            {
                float const xd = (0.5f + x) * scalew - 0.5f;
                int x1 = xd;
                int x2 = x1 + 1;
                float const dx1 = xd - x1;
                float const dx2 = 1.0f - dx1;
                x1 = (x1 < 0) ? 0 : ((x1 < w) ? x1 : (w - 1));
                x2 = (x2 < 0) ? 0 : ((x2 < w) ? x2 : (w - 1));

                float const t11 = src_line[y1 * src_stride + x1];
                float const t12 = src_line[y1 * src_stride + x2];
                float const t21 = src_line[y2 * src_stride + x1];
                float const t22 = src_line[y2 * src_stride + x2];

                float t = dy2 * (dx2 * t11 + dx1 * t12)
                        + dy1 * (dx2 * t21 + dx1 * t22);
                t = (t < 0.0f) ? 0.0f : ((t < 255.0f) ? t : 255.0f);
                gscale_line[x] = (unsigned char) t;
            }
            gscale_line += gscale_stride;
        }
        return gscale;
    }
    else
    {
        return GrayImage(src);
    }
}
GrayImage grayRISundefect(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayRISundefectInPlace(dst, radius, coef);
    return dst;
}

void grayRISundefectInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        int const wr = (w + radius - 1) / radius;
        int const hr = (h + radius - 1) / radius;

        if ((wr > 0) && (hr > 0))
        {
            GrayImage gsub = grayScaleByLine(src, wr, hr);
            if (gsub.isNull())
            {
                return;
            }

            GrayImage gref = grayScaleByLine(gsub, w, h);
            if (gref.isNull())
            {
                return;
            }
            unsigned char* gref_line = gref.data();
            int const gref_stride = gref.stride();

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    float const origin = src_line[x];
                    float const ref = gref_line[x];
                    float const target = origin + (origin - ref);

                    float retval = coef * target + (1.0f - coef) * origin + 0.5f;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                    src_line[x] = (unsigned char) retval;
                }
                src_line += src_stride;
                gref_line += gref_stride;
            }
        }
    }
} // grayRISundefect

GrayImage grayWiener(
    GrayImage const& src,
    int const radius,
    float const noise_sigma)
{
    GrayImage dst(src);
    grayWienerInPlace(dst, radius, noise_sigma);
    return dst;
}

void grayWienerInPlace(
    GrayImage& src,
    int const radius,
    float const noise_sigma)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (noise_sigma > 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        float const noise_variance = noise_sigma * noise_sigma;

        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return;
        }

        unsigned char* gdeviation_line = gdeviation.data();
        int const gdeviation_stride = gdeviation.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const deviation = gdeviation_line[x];
                float const variance = deviation * deviation;

                float const src_pixel = (float) src_line[x];
                float const delta_pixel = src_pixel - mean;
                float const delta_variance = variance - noise_variance;
                float dst_pixel = mean;
                if (delta_variance > 0.0f)
                {
                    dst_pixel += delta_pixel * delta_variance / variance;
                }
                int val = (int) (dst_pixel + 0.5f);
                val = (val < 0) ? 0 : (val < 255) ? val : 255;
                src_line[x] = (unsigned char) val;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }
} // grayWiener

GrayImage grayKnnDenoiser(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayKnnDenoiserInPlace(dst, radius, coef);
    return dst;
}

void grayKnnDenoiserInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef > 0.0))
    {
        float const threshold_weight = 0.02f;
        float const threshold_lerp = 0.66f;
        float const noise_lerpc = 0.16f;

        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        IntegralImage<uint32_t> integral_image(w, h);

        for (int y = 0; y < h; y++)
        {
            integral_image.beginRow();
            for (int x = 0; x < w; x++)
            {
                uint32_t const pixel = src_line[x];
                integral_image.push(pixel);
            }
            src_line += src_stride;
        }

        int const noise_area = ((2 * radius + 1) * (2 * radius + 1));
        float const noise_area_inv = (1.0f / (float) noise_area);
        float const noise_weight = (1.0f / (coef * coef));
        float const pixel_weight = (1.0f / 255.0f);

        src_line = src.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
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
                color += 0.5f;
                color = (color < 0.0f) ? 0.0f : ((color < 255.0f) ? color : 255.0f);
                src_line[x] = (unsigned char) color;
            }
            src_line += src_stride;
        }
    }
} // grayKnnDenoiser

GrayImage grayEMDenoiser(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayEMDenoiserInPlace(dst, radius, coef);
    return dst;
}

void grayEMDenoiserInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        GrayImage gdelta(gmean);
        if (gdelta.isNull())
        {
            return;
        }
        unsigned char* gdelta_line = gdelta.data();
        int const gdelta_stride = gdelta.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                unsigned char const origin = src_line[x];
                unsigned char const mean = gmean_line[x];
                unsigned char const dt = (origin < mean) ? (mean - origin) : (origin - mean);
                gdelta_line[x] = dt;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            gdelta_line += gdelta_stride;
        }

        GrayImage gcon = gaussBlur(gdelta, radius, radius);
        if (gcon.isNull())
        {
            return;
        }
        unsigned char* gcon_line = gcon.data();
        int const gcon_stride = gcon.stride();

        unsigned int threshold = grayBiModalTiledValue(gcon, 0, 0, w, h);

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                unsigned char const origin = src_line[x];
                unsigned char const mean = gmean_line[x];
                unsigned char const con = gcon_line[x];
                float retval = origin;
                if (con < threshold)
                {
                    retval = coef * mean + (1.0f - coef) * origin + 0.5f;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                }
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            gcon_line += gcon_stride;
        }
    }
} // grayEMDenoiser

GrayImage grayMedian(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayMedianInPlace(dst, radius, coef);
    return dst;
} // grayMedian

void grayMedianInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        unsigned int const histsize = 256;
        unsigned int median = histsize / 2;
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        unsigned int const src_stride = src.stride();
        uint64_t const rsize = radius + 1 + radius;
        uint64_t const fsize = rsize * rsize;
        uint64_t sum = 0, histogram[histsize] = {0};
        uint64_t const fsizemed = (fsize + 1) / 2;

        GrayImage gmean = GrayImage(src);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        unsigned int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                if (x == 0)
                {
                    for (unsigned int k = 0; k < histsize; k++)
                    {
                        histogram[k] = 0;
                    }
                    for (int yf = (y - radius); yf < (y + radius + 1); yf++)
                    {
                        int const yfs = (yf < 0) ? 0 : ((yf < h) ? yf : (h - 1));
                        unsigned char* f_line = src.data();
                        f_line += (yfs * src_stride);
                        for  (int xf = (x - radius); xf < (x + radius + 1); xf++)
                        {
                            int const xfs = (xf < 0) ? 0 : ((xf < w) ? xf : (w - 1));
                            histogram[f_line[xfs]]++;
                        }
                    }
                }
                else
                {
                    int const L = x - radius - 1;
                    int const xl = (L < 0) ? 0 : ((L < w) ? L : (w - 1));
                    int const R = x + radius;
                    int const xr = (R < 0) ? 0 : ((R < w) ? R : (w - 1));
                    for (int yf = (y - radius); yf < (y + radius + 1); yf++)
                    {
                        int const yfs = (yf < 0) ? 0 : ((yf < h) ? yf : (h - 1));
                        unsigned char* f_line = src.data();
                        f_line += (yfs * src_stride);
                        histogram[f_line[xl]]--;
                        histogram[f_line[xr]]++;
                    }
                }
                int median = 0;
                sum = histogram[median];
                while (sum < fsizemed)
                {
                    median++;
                    sum += histogram[median];
                }
                float retval = coef * median + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                gmean_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }

        src = GrayImage(gmean);
        gmean = GrayImage();
    }
} // grayMedianInPlace

GrayImage graySubtractBG(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    graySubtractBGInPlace(dst, radius, coef);
    return dst;
} // graySubstractBG

void graySubtractBGInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }
    unsigned char* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = grayMedian(src, radius, 1.0f);
        if (gmean.isNull())
        {
            return;
        }
        unsigned int const w = src.width();
        unsigned int const h = src.height();
        unsigned char* gmean_line = gmean.data();
        unsigned int const gmean_stride = gmean.stride();
        int const coef512 = (int)(coef * 512.0f + 0.5f);

        unsigned char gmin = gmean_line[0];
        unsigned char gmax = gmean_line[0];
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                unsigned char const mean = gmean_line[x];
                gmin = (mean < gmin) ? mean : gmin;
                gmax = (mean < gmax) ? gmax : mean;
            }
            gmean_line += gmean_stride;
        }
        int const gm = ((int) gmin + (int) gmax) >> 1;

        gmean_line = gmean.data();
        for (unsigned int y = 0; y < h; y++)
        {
            for (unsigned int x = 0; x < w; x++)
            {
                int const origin = src_line[x];
                int const mean = gmean_line[x];
                int diff = gm + origin - mean;
                int ret = ((coef512 * diff + (512 - coef512) * origin + 256) >> 9);
                ret = (ret < 0) ? 0 : (ret < 255) ? ret : 255;
                src_line[x] = (unsigned char) ret;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
        gmean = GrayImage();
    }
} // graySubtractBGInPlace

GrayImage grayDespeckle(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayDespeckleInPlace(dst, radius, coef);
    return dst;
} // grayDespeckle

void grayDespeckleInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = GrayImage(src);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int j = 0; j < radius; j++)
        {
            src_line = src.data();
            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                unsigned char* src_line1 = (y > 0) ? (src_line - src_stride) : src_line;
                unsigned char* src_line2 = (y < (h - 1)) ? (src_line + src_stride) : src_line;
                for (int x = 0; x < w; x++)
                {
                    int x1 = (x > 0) ? (x - 1) : x;
                    int x2 = (x < (w - 1)) ? (x + 1) : x;

                    int pA = src_line1[x1];
                    int pB = src_line1[x];
                    int pC = src_line1[x2];
                    int pD = src_line[x1];
                    int pE = src_line[x];
                    int pF = src_line[x2];
                    int pH = src_line2[x1];
                    int pG = src_line2[x];
                    int pI = src_line2[x2];

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
                        if((pM > 128 && gd2 > 0.0f) || (pM <= 128 && gd2 < 0.0f))
                        {
                            pR += (int) (coef * gd2 / dI + 0.5f);
                            pR = (pR < 0) ? 0 : (pR < 255) ? pR : 255;
                        }
                    }
                    else
                    {
                        pR = pM;
                    }
                    gmean_line[x] = pR;

                }
                src_line += src_stride;
                gmean_line += gmean_stride;
            }
            src_line = src.data();
            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    src_line[x] = gmean_line[x];
                }
                src_line += src_stride;
                gmean_line += gmean_stride;
            }
        }
        gmean = GrayImage(src);
        if (gmean.isNull())
        {
            return;
        }
    }
} // grayDespeckleInPlace

GrayImage grayAutoLevel(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayAutoLevelInPlace(dst, radius, coef);
    return dst;
}

void grayAutoLevelInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean;
        if (radius > 0)
        {
            gmean = gaussBlur(src, radius, radius);
        }
        else
        {
            gmean = GrayImage(src);
        }
        if (gmean.isNull())
        {
            return;
        }

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        int gmin = 256, gmax = 0;
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                gmin = (gmin < mean) ? gmin : mean;
                gmax = (gmax < mean) ? mean : gmax;
            }
            gmean_line += gmean_stride;
        }

        float const a1 = 256.0f / (gmax - gmin + 1);
        float const a0 = -a1 * gmin;

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float retval = origin * a1 + a0;
                retval = coef * retval + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
        }
    }
} // grayAutoLevel

GrayImage grayBalance(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayBalanceInPlace(dst, radius, coef);
    return dst;
}

void grayBalanceInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        int const radius2 = radius + radius;
        GrayImage gmean2 = gaussBlur(src, radius2, radius2);
        if (gmean2.isNull())
        {
            return;
        }
        unsigned char* gmean2_line = gmean2.data();
        int const gmean2_stride = gmean2.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float origin = src_line[x];
                float mean = gmean_line[x];
                float mean2 = gmean2_line[x];

                /* overlay 1 */
                float base = 255.0f - mean;
                float overlay = mean2;
                float retval1 = base;
                if (base > 127.5f)
                {
                    retval1 = 255.0f - retval1;
                    overlay = 255.0f - overlay;
                }
                retval1 *= overlay;
                retval1 += retval1;
                retval1 /= 255.0f;
                if (base > 127.5f)
                {
                    retval1 = 255.0f - retval1;
                }

                /* overlay 2 */
                base = 255.0f - mean2;
                overlay = mean;
                float retval2 = base;
                if (base > 127.5f)
                {
                    retval2 = 255.0f - retval2;
                    overlay = 255.0f - overlay;
                }
                retval2 *= overlay;
                retval2 += retval2;
                retval2 /= 255.0f;
                if (base > 127.5f)
                {
                    retval2 = 255.0f - retval2;
                }

                /* overlay 3 */
                base = origin;
                overlay = retval1;
                float retval3 = base;
                if (base > 127.5f)
                {
                    retval3 = 255.0f - retval3;
                    overlay = 255.0f - overlay;
                }
                retval3 *= overlay;
                retval3 += retval3;
                retval3 /= 255.0f;
                if (base > 127.5f)
                {
                    retval3 = 255.0f - retval3;
                }

                /* overlay 4 */
                base = retval3;
                overlay = retval2;
                float retval = base;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }
                retval *= overlay;
                retval += retval;
                retval /= 255.0f;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                }

                retval += 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                gmean_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            gmean2_line += gmean2_stride;
        }

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float mean = gmean_line[x];

                mean = coef * mean + (1.0f - coef) * origin + 0.5f;
                mean = (mean < 0.0f) ? 0.0f : (mean < 255.0f) ? mean : 255.0f;
                src_line[x] = (unsigned char) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayBalance

GrayImage grayOverBlur(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayOverBlurInPlace(dst, radius, coef);
    return dst;
}

void grayOverBlurInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float origin = src_line[x];
                float mean = gmean_line[x];

                /* overlay */
                float base = origin;
                float overlay = 255.0f - mean;
                float retval = base;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }
                retval *= overlay;
                retval += retval;
                retval /= 255.0f;
                if (base > 127.5f)
                {
                    retval = 255.0f - retval;
                    overlay = 255.0f - overlay;
                }

                retval += 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                gmean_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float mean = gmean_line[x];

                mean = coef * mean + (1.0f - coef) * origin + 0.5f;
                mean = (mean < 0.0f) ? 0.0f : (mean < 255.0f) ? mean : 255.0f;
                src_line[x] = (unsigned char) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayOverBlur

GrayImage grayRetinex(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayRetinexInPlace(dst, radius, coef);
    return dst;
}

void grayRetinexInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = 1.0f + src_line[x];
                float const mean = 1.0f + gmean_line[x];
                float const frac = origin / mean;
                float const retinex = 127.5f * frac + 0.5f;

                gmean_line[x] = (unsigned char) retinex;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float mean = gmean_line[x];

                mean = coef * mean + (1.0f - coef) * origin + 0.5f;
                mean = (mean < 0.0f) ? 0.0f : (mean < 255.0f) ? mean : 255.0f;
                src_line[x] = (unsigned char) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayReinex

GrayImage grayEqualize(
    GrayImage const& src,
    int const radius,
    float const coef,
    bool flg_blur)
{
    GrayImage dst(src);
    grayEqualizeInPlace(dst, radius, coef, flg_blur);
    return dst;
}

void grayEqualizeInPlace(
    GrayImage& src,
    int const radius,
    float const coef,
    bool flg_blur)
{
    if (src.isNull())
    {
        return;
    }

    GrayImage gmean;
    if (radius > 0)
    {
        gmean = gaussBlur(src, radius, radius);
    }
    else
    {
        gmean = GrayImage(src);
    }
    if (gmean.isNull())
    {
        return;
    }

    if (coef != 0.0f)
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        uint64_t histogram[256] = {0};
        uint64_t szi = (h * w) >> 8;
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                unsigned int const mean = gmean_line[x];
                histogram[mean]++;
            }
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

        gmean_line = gmean.data();
        if (flg_blur)
        {
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    int const origin = gmean_line[x];
                    int const remap = histogram[origin];

                    gmean_line[x] = (unsigned char) remap;
                }
                gmean_line += gmean_stride;
            }
        }
        else
        {
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    int const origin = src_line[x];
                    int const remap = histogram[origin];

                    gmean_line[x] = (unsigned char) remap;
                }
                src_line += src_stride;
                gmean_line += gmean_stride;
            }
        }

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float mean = gmean_line[x];

                mean = coef * mean + (1.0f - coef) * origin + 0.5f;
                mean = (mean < 0.0f) ? 0.0f : (mean < 255.0f) ? mean : 255.0f;
                src_line[x] = (unsigned char) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayEqualize

GrayImage grayBlur(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayBlurInPlace(dst, radius, coef);
    return dst;
}

void grayBlurInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];

                float retval = coef * mean + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayBlur

GrayImage grayComix(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayComixInPlace(dst, radius, coef);
    return dst;
}

void grayComixInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gray = GrayImage(src);
        if (gray.isNull())
        {
            return;
        }
        unsigned char* gray_line = gray.data();
        int const gray_stride = gray.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                unsigned char const origin = gray_line[x];
                unsigned char const mean = gmean_line[x];
                unsigned char const delta = (origin < mean) ? (mean - origin) : (origin - mean);
                gray_line[x] = delta;
            }
            gray_line += gray_stride;
            gmean_line += gmean_stride;
        }

        GrayImage grebound = gaussBlur(gray, radius, radius);
        if (grebound.isNull())
        {
            return;
        }
        unsigned char* grebound_line = grebound.data();
        int const grebound_stride = grebound.stride();
        gray = GrayImage();

        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float const rebound = grebound_line[x];
                float const delta = mean - origin;
                float const cre = (delta < 0.0f) ? (delta + rebound) : (delta -  rebound);
                float const fre = origin + cre;

                float retval = coef * fre + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            grebound_line += grebound_stride;
        }
        grebound = GrayImage();
        gmean = GrayImage();
    }
} // grayComix

GrayImage graySigma(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    graySigmaInPlace(dst, radius, coef);
    return dst;
}

void graySigmaInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float const delta = mean - origin;
                float const deltainv = (delta < 0.0f) ? (255.0f + delta) : (255.0f - delta);
                float const sigma = delta * deltainv / 127.5f;
                float const target = mean - sigma;

                float retval = coef * target + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // graySigma

GrayImage grayScreen(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayScreenInPlace(dst, radius, coef);
    return dst;
}

void grayScreenInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = grayEqualize(src, radius, 1.0f, true);
        if (gmean.isNull())
        {
            return;
        }

        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float mean = 255.0f - gmean_line[x];

                /* overlay */
                float retval = origin;
                if (origin > 127.5f)
                {
                    retval = 255.0f - retval;
                    mean = 255.0f - mean;
                }
                retval *= mean;
                retval += retval;
                retval /= 255.0f;
                if (origin > 127.5f)
                {
                    retval = 255.0f - retval;
                }
                retval = coef * retval + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayScreen

GrayImage grayEdgeDiv(
    GrayImage const& src,
    int const radius,
    double const kep,
    double const kbd)
{
    GrayImage dst(src);
    grayEdgeDivInPlace(dst, radius, kep, kbd);
    return dst;
}

void grayEdgeDivInPlace(
    GrayImage& src,
    int const radius,
    double const kep,
    double const kbd)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && ((kep != 0.0f) || (kbd != 0.0f)))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        double kepw = kep;
        // Adapt mode
        if (kep < 0.0)
        {
            kepw = 0.0;
            double kepl = 0.0, kd = 0.0, kdl = 0.0, gdelta;
            for (int y = 0; y < h; y++)
            {
                kdl = 0.0;
                kepl = 0.0;
                for (int x = 0; x < w; x++)
                {
                    double const mean = gmean_line[x];
                    double const origin = src_line[x];
                    gdelta = (origin < mean) ? (mean - origin) : (origin - mean);
                    kdl += gdelta;
                    kepl += (gdelta * gdelta);
                }
                kd += kdl;
                kepw += kepl;
                src_line += src_stride;
                gmean_line += gmean_stride;
            }
            kepw = (kd > 0.0) ? (kepw / kd) : 0.0;
            kepw /= 64.0;
            kepw = 1.0 - kepw;
            kepw = (kepw < 0.0) ? 0.0 : kepw;
            kepw *= kbd;

            src_line = src.data();
            gmean_line = gmean.data();
        }
        // Adapt mode end

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const origin = src_line[x];
                float retval = origin;
                if (kepw != 0.0f)
                {
                    // EdgePlus
                    // edge = I / blur (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                    float const edge = (retval + 1) / (mean + 1) - 0.5;
                    // edgeplus = I * edge, mean value = 0.5 * mean(I)
                    float const edgeplus = origin * edge;
                    // return k * edgeplus + (1 - k) * I
                    retval = kepw * edgeplus + (1.0 - kepw) * origin;
                }
                if (kbd != 0.0f)
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
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
}  // grayEdgeDiv

/*
 * Robust = 255.0 - (surround + 255.0) * sc / (surround + sc), k = 0.2
 * sc = surround - img
 * surround = blur(img, r), r = 10
 */
GrayImage grayRobust(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayRobustInPlace(dst, radius, coef);
    return dst;
}

void grayRobustInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float retval = origin;
                float const sc = mean - origin;
                float const robust = 255.0f - (mean + 255.0f) * sc / (mean + sc);
                retval = coef * robust + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
}  // grayRobust

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
GrayImage grayGrain(
    GrayImage const& src,
    int const radius,
    float const coef)
{
    GrayImage dst(src);
    grayGrainInPlace(dst, radius, coef);
    return dst;
}

void grayGrainInPlace(
    GrayImage& src,
    int const radius,
    float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef != 0.0f))
    {
        int const w = src.width();
        int const h = src.height();
        unsigned char* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        unsigned char* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                int const origin = src_line[x];
                int const mean = gmean_line[x];
                int retval = origin - mean + 127;
                retval = (retval < 0) ? 0 : (retval < 255) ? retval : 255;
                gmean_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }

        GrayImage gsigma = gaussBlur(gmean, radius, radius);
        if (!(gsigma.isNull()))
        {
            unsigned char* gsigma_line = gsigma.data();
            int const gsigma_stride = gsigma.stride();

            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    int const mean = gmean_line[x];
                    int const sigma = gsigma_line[x];
                    int retval = mean + mean - sigma;
                    retval = (retval < 0) ? 0 : (retval < 255) ? retval : 255;
                    gmean_line[x] = (unsigned char) retval;
                }
                gmean_line += gmean_stride;
                gsigma_line += gsigma_stride;
            }
        }

        src_line = src.data();
        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float retval = coef * mean + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (unsigned char) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
}  // grayGrain

unsigned int grayDominantaValue(
    GrayImage const& src)
{
    if (src.isNull())
    {
        return 127;
    }

    int const w = src.width();
    int const h = src.height();
    unsigned char const* src_line = src.data();
    int const src_stride = src.stride();

    uint64_t histogram[256] = {0};
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned int const value = src_line[x];
            histogram[value]++;
        }
        src_line += src_stride;
    }

    for (unsigned int i = 1; i < 256; i++)
    {
        histogram[i] += histogram[i - 1];
    }

    unsigned int ilow  = 0;
    for (unsigned int i = (256 >> 1); i > 0; i >>= 1)
    {
        uint64_t sum_max = 0;
        unsigned int inew = ilow;
        for (unsigned int j = ilow; j < (ilow + i); j++)
        {
            uint64_t const sum_low = (j > 0) ? histogram[j] : 0;
            uint64_t const sum_high = histogram[j + i];
            uint64_t const sum = sum_high - sum_low;
            if (sum > sum_max)
            {
                inew = j;
                sum_max = sum;
            }
        }
        ilow = inew;
    }

    return ilow;
} // grayDominantaValue

} // namespace imageproc
