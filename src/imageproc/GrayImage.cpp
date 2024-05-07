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

#include "GrayImage.h"
#include "Grayscale.h"
#include <new>
#include <stdexcept>

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
    GrayImage const& src, QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMapMean: invalid window_size");
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

    IntegralImage<uint32_t> integral_image(w, h);

    for (int y = 0; y < h; ++y)
    {
        integral_image.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = src_line[x];
            integral_image.push(pixel);
        }
        src_line += src_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

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
    GrayImage const& src, QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMapDeviation: invalid window_size");
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

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    uint32_t min_gray_level = 255;

    for (int y = 0; y < h; ++y)
    {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = src_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
            min_gray_level = std::min(min_gray_level, pixel);
        }
        src_line += src_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    double max_deviation = 0;

    for (int y = 0; y < h; ++y)
    {
        int const top = ((y - window_lower_half) < 0) ? 0 : (y - window_lower_half);
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;

        for (int x = 0; x < w; ++x)
        {
            int const left = ((x - window_left_half) < 0) ? 0 : (x - window_left_half);
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
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
    GrayImage const& src, QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMapMax: invalid window_size");
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
    GrayImage gmax = GrayImage(src);
    if (gmax.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmax_line = gmax.data();

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int const left = ((x - window_left_half) < 0) ? 0 : (x - window_left_half);
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;

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
        int const top = ((y - window_lower_half) < 0) ? 0 : (y - window_lower_half);
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;

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
    GrayImage const& src, QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMapContrast: invalid window_size");
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
    GrayImage gmax = GrayImage(src);
    if (gmax.isNull())
    {
        return GrayImage();
    }
    GrayImage gmin = GrayImage(src);
    if (gmin.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();
    uint8_t* gmax_line = gmax.data();
    uint8_t* gmin_line = gmin.data();

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int const left = ((x - window_left_half) < 0) ? 0 : (x - window_left_half);
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;

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
        int const top = ((y - window_lower_half) < 0) ? 0 : (y - window_lower_half);
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;

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

GrayImage grayKnnDenoiser(
    GrayImage const& src, int const radius, float const coef)
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

    if ((radius > 0) && (coef > 0.0))
    {
        float const threshold_weight = 0.02f;
        float const threshold_lerp = 0.66f;
        float const noise_eps = 0.0000001f;
        float const noise_lerpc = 0.16f;

        int const w = src.width();
        int const h = src.height();
        uint8_t const* src_line = src.data();
        int const src_stride = src.stride();


        uint8_t* gray_line = gray.data();
        int const gray_stride = gray.stride();

        IntegralImage<uint32_t> integral_image(w, h);

        for (int y = 0; y < h; ++y)
        {
            integral_image.beginRow();
            for (int x = 0; x < w; ++x)
            {
                uint32_t const pixel = gray_line[x];
                integral_image.push(pixel);
            }
            gray_line += gray_stride;
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
                gray_line[x] = (uint8_t) color;
            }
            src_line += src_stride;
            gray_line += gray_stride;
        }
    }
	return gray;

} // grayKnnDenoiser

} // namespace imageproc
