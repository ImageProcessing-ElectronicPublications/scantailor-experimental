/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2008  Joseph Artsimovich <joseph_a@mail.ru>

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

#include <stdexcept>
#include <stdint.h>
#include <QImage>
#include "AdjustBrightness.h"

namespace imageproc
{

void adjustBrightness(
    QImage& rgb_image,
    QImage const& brightness,
    float const wr,
    float const wb)
{
    switch (rgb_image.format())
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        break;
    default:
        throw std::invalid_argument("adjustBrightness: not (A)RGB32");
    }

    if (brightness.format() != QImage::Format_Indexed8
            || !brightness.allGray())
    {
        throw std::invalid_argument("adjustBrightness: brightness not grayscale");
    }

    if (rgb_image.size() != brightness.size())
    {
        throw std::invalid_argument("adjustBrightness: image and brightness have different sizes");
    }

    uint32_t* rgb_line = reinterpret_cast<uint32_t*>(rgb_image.bits());
    int const rgb_wpl = rgb_image.bytesPerLine() / 4;

    uint8_t const* br_line = brightness.bits();
    int const br_bpl = brightness.bytesPerLine();

    int const width = rgb_image.width();
    int const height = rgb_image.height();

    float const wg = 1.0f - wr - wb;
    float const wu = (1.0f - wb);
    float const wv = (1.0f - wr);
    float const r_wg = 1.0f / wg;
    float const r_wu = 1.0f / wu;
    float const r_wv = 1.0f / wv;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint32_t RGB = rgb_line[x];
            float const R = (RGB >> 16) & 0xFF;
            float const G = (RGB >> 8) & 0xFF;
            float const B = RGB & 0xFF;

            float const Y = wr * R + wg * G + wb * B;
            float const U = (B - Y) * r_wu;
            float const V = (R - Y) * r_wv;

            float new_Y = br_line[x];
            float new_R = new_Y + V * wv;
            float new_B = new_Y + U * wu;
            float new_G = (new_Y - new_R * wr - new_B * wb) * r_wg;

            RGB &= 0xFF000000; // preserve alpha
            RGB |= uint32_t(qBound(0, int(new_R + 0.5f), 255)) << 16;
            RGB |= uint32_t(qBound(0, int(new_G + 0.5f), 255)) << 8;
            RGB |= uint32_t(qBound(0, int(new_B + 0.5f), 255));
            rgb_line[x] = RGB;
        }
        rgb_line += rgb_wpl;
        br_line += br_bpl;
    }
}

void adjustBrightnessYUV(
    QImage& rgb_image,
    QImage const& brightness)
{
    adjustBrightness(rgb_image, brightness, 0.299f, 0.114f);
}

void adjustBrightnessGrayscale(
    QImage& rgb_image,
    QImage const& brightness)
{
    adjustBrightness(rgb_image, brightness, 11.0f/32.0f, 5.0f/32.0f);
}

} // namespace imageproc
