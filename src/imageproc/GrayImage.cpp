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

GridAccessor<uint8_t const>
GrayImage::accessor() const
{
    return GridAccessor<uint8_t const> {data(), stride(), width(), height()};
}

GridAccessor<uint8_t>
GrayImage::accessor()
{
    return GridAccessor<uint8_t> {data(), stride(), width(), height()};
}

/*
 * mean, w = 200
 */
GrayImage grayMapMean(
    GrayImage const& src, int const radius)
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
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
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
            gray_line[x] = (uint8_t) mean;
        }
        gray_line += gray_stride;
    }

    return gray;
}  // grayMapMean

/*
 * stdev, w = 200
 */
GrayImage grayMapDeviation(
    GrayImage const& src, int const radius)
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
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
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

    double max_deviation = 0;
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
            gray_line[x] = (uint8_t) deviation;
        }
        gray_line += gray_stride;
    }

    return gray;
} // grayMapDeviation

GrayImage grayMapMax(
    GrayImage const& src, int const radius)
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
    GrayImage gmax = GrayImage(src);
    if (gmax.isNull())
    {
        return gray;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmax_line = gmax.data();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int const left = ((x - radius) < 0) ? 0 : (x - radius);
            int const right = ((x + radius + 1) < w) ? (x + radius + 1) : w;

            uint8_t const origin = gray_line[x];
            uint8_t immax = origin;
            for (int xf = left; xf < right; xf++)
            {
                uint8_t imf = gray_line[xf];
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
            uint8_t const origin = gray_line[x];
            uint8_t immax = origin;
            unsigned long int idx = top * gray_stride + x;
            for (int yf = top; yf < bottom; yf++)
            {
                uint8_t immaxf = gmax_line[idx];
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

    return gray;
}  // grayMapMax

GrayImage grayMapContrast(
    GrayImage const& src, int const radius)
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
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmax_line = gmax.data();
    uint8_t* gmin_line = gmin.data();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int const left = ((x - radius) < 0) ? 0 : (x - radius);
            int const right = ((x + radius + 1) < w) ? (x + radius + 1) : w;

            uint8_t const origin = gray_line[x];
            uint8_t immin = origin;
            uint8_t immax = origin;
            for (int xf = left; xf < right; xf++)
            {
                uint8_t imf = gray_line[xf];
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
            uint8_t const origin = gray_line[x];
            uint8_t immin = origin;
            uint8_t immax = origin;
            unsigned long int idx = top * gray_stride + x;
            for (int yf = top; yf < bottom; yf++)
            {
                uint8_t immaxf = gmax_line[idx];
                uint8_t imminf = gmin_line[idx];
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
            uint8_t threshold = (immax - immin);

            gray_line[x] = threshold;
        }
        gray_line += gray_stride;
    }

    return gray;
}  // grayMapContrast

GrayImage grayWiener(
    GrayImage const& src, int const radius, float const noise_sigma)
{
    GrayImage dst(src);
    grayWienerInPlace(dst, radius, noise_sigma);
    return dst;
}

void grayWienerInPlace(
    GrayImage& src, int const radius, float const noise_sigma)
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

        uint8_t* src_line = src.data();
        int const src_stride = src.stride();

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        GrayImage gdeviation = grayMapDeviation(src, radius);
        if (gdeviation.isNull())
        {
            return;
        }

        uint8_t* gdeviation_line = gdeviation.data();
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
                src_line[x] = (uint8_t) val;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
            gdeviation_line += gdeviation_stride;
        }
    }
} // grayWiener

GrayImage grayKnnDenoiser(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayKnnDenoiserInPlace(dst, radius, coef);
    return dst;
}

void grayKnnDenoiserInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    if ((radius > 0) && (coef > 0.0))
    {
        float const threshold_weight = 0.02f;
        float const threshold_lerp = 0.66f;
        float const noise_eps = 0.0000001f;
        float const noise_lerpc = 0.16f;

        int const w = src.width();
        int const h = src.height();
        uint8_t* src_line = src.data();
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
                src_line[x] = (uint8_t) color;
            }
            src_line += src_stride;
        }
    }
} // grayKnnDenoiser

GrayImage grayDespeckle(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayDespeckleInPlace(dst, radius, coef);
    return dst;
} // grayDespeckle

void grayDespeckleInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    GrayImage gmean;
    if ((radius > 0) && (coef != 0.0f))
    {
        gmean = GrayImage(src);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int j = 0; j < radius; j++)
        {
            src_line = src.data();
            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                uint8_t* src_line1 = (y > 0) ? (src_line - src_stride) : src_line;
                uint8_t* src_line2 = (y < (h - 1)) ? (src_line + src_stride) : src_line;
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
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayAutoLevelInPlace(dst, radius, coef);
    return dst;
}

void grayAutoLevelInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {

         GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
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
                src_line[x] = (uint8_t) retval;
            }
            src_line += src_stride;
        }
    }
} // grayAutoLevel

GrayImage grayBalance(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayBalanceInPlace(dst, radius, coef);
    return dst;
}

void grayBalanceInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        int const radius2 = radius + radius;
        GrayImage gmean2 = gaussBlur(src, radius2, radius2);
        if (gmean2.isNull())
        {
            return;
        }
        uint8_t* gmean2_line = gmean2.data();
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
                gmean_line[x] = (uint8_t) retval;
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
                src_line[x] = (uint8_t) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayBalance

GrayImage grayEqualize(
    GrayImage const& src, int const radius, float const coef, bool flg_blur)
{
    GrayImage dst(src);
    grayEqualizeInPlace(dst, radius, coef, flg_blur);
    return dst;
}

void grayEqualizeInPlace(
    GrayImage& src, int const radius, float const coef, bool flg_blur)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
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

    if (coef != 0.0f)
    {
        uint8_t* gmean_line = gmean.data();
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

                    gmean_line[x] = (uint8_t) remap;
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

                    gmean_line[x] = (uint8_t) remap;
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
                src_line[x] = (uint8_t) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayEqualize

GrayImage grayRetinex(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayRetinexInPlace(dst, radius, coef);
    return dst;
}

void grayRetinexInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = 1.0f + src_line[x];
                float const mean = 1.0f + gmean_line[x];
                float const frac = origin / mean;
                float const retinex = 127.5f * frac + 0.5f;
                float const target = (retinex < 0.0f) ? 0.0f : (retinex < 255.0f) ? retinex : 255.0f;

                gmean_line[x] = (uint8_t) retinex;
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
                src_line[x] = (uint8_t) mean;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayReinex

GrayImage grayBlur(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayBlurInPlace(dst, radius, coef);
    return dst;
}

void grayBlurInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];

                float retval = coef * mean + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (uint8_t) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayBlur

GrayImage graySigma(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    graySigmaInPlace(dst, radius, coef);
    return dst;
}

void graySigmaInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {

        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
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
                src_line[x] = (uint8_t) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // graySigma

GrayImage grayScreen(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayScreenInPlace(dst, radius, coef);
    return dst;
}

void grayScreenInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {

        GrayImage gmean = grayEqualize(src, radius, 1.0f, true);
        if (gmean.isNull())
        {
            return;
        }

        uint8_t* gmean_line = gmean.data();
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
                src_line[x] = (uint8_t) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
} // grayScreen

GrayImage grayEdgeDiv(
    GrayImage const& src, int const radius,
    double const kep, double const kbd)
{
    GrayImage dst(src);
    grayEdgeDivInPlace(dst, radius, kep, kbd);
    return dst;
}

void grayEdgeDivInPlace(
    GrayImage& src, int const radius,
    double const kep, double const kbd)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && ((kep != 0.0f) || (kbd != 0.0f)))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
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
                src_line[x] = (int) retval;
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
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayRobustInPlace(dst, radius, coef);
    return dst;
}

void grayRobustInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
                float const mean = gmean_line[x];
                float retval = origin;
                float const sc = mean - origin;
                float const robust = 255.0 - (mean + 255.0) * sc / (mean + sc);
                retval = coef * robust + (1.0 - coef) * origin + 0.5f;
                retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
                src_line[x] = (int) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
}  // grayRobust

GrayImage grayGravure(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayGravureInPlace(dst, radius, coef);
    return dst;
}

void grayGravureInPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
        if (gmean.isNull())
        {
            return;
        }
        uint8_t* gmean_line = gmean.data();
        int const gmean_stride = gmean.stride();

        double mean_delta = 0.0;
        for (int y = 0; y < h; y++)
        {
            double mean_delta_line = 0.0;
            for (int x = 0; x < w; x++)
            {
                float const mean = gmean_line[x];
                float const origin = src_line[x];

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
            src_line = src.data();
            gmean_line = gmean.data();
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    float const origin = src_line[x];
                    float const mean = gmean_line[x];

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
                    retval = coef * retval + (1.0f - coef) * origin + 0.5f;
                    retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                    src_line[x] = (uint8_t) retval;
                }
                src_line += src_stride;
                gmean_line += gmean_stride;
            }
        }
    }
}  // grayGravure

GrayImage grayDots8(
    GrayImage const& src, int const radius, float const coef)
{
    GrayImage dst(src);
    grayDots8InPlace(dst, radius, coef);
    return dst;
}

void grayDots8InPlace(
    GrayImage& src, int const radius, float const coef)
{
    if (src.isNull())
    {
        return;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* src_line = src.data();
    int const src_stride = src.stride();

    if ((radius > 0) && (coef != 0.0f))
    {
        GrayImage gmean = gaussBlur(src, radius, radius);
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

        for (y = 0; y < h; y++)
        {
            yb = y % wwidth;
            for (x = 0; x < w; x++)
            {
                xb = x % wwidth;
                threshold = ddith[yb * wwidth + xb];
                gmean_line[x] = (uint8_t) threshold;
            }
            gmean_line += gmean_stride;
        }

        gmean_line = gmean.data();
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float const origin = src_line[x];
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
                retval = coef * retval + (1.0f - coef) * origin + 0.5f;
                retval = (retval < 0.0f) ? 0.0f : (retval < 255.0f) ? retval : 255.0f;
                src_line[x] = (uint8_t) retval;
            }
            src_line += src_stride;
            gmean_line += gmean_stride;
        }
    }
}  // grayDots8

unsigned int grayDominantaValue(
    GrayImage const& src)
{
    if (src.isNull())
    {
        return 127;
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
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
