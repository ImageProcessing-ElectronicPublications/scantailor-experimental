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

#include "MorphGradientDetect.h"
#include "Morphology.h"
#include "Grayscale.h"
#include "RasterOpGeneric.h"
#include <QImage>
#include <QSize>

namespace imageproc
{

GrayImage morphGradientDetectDarkSide(GrayImage const& image, QSize const& area)
{
    GrayImage lighter(erodeGray(image, area, 0x00));
    rasterOpGeneric(
        [](uint8_t& lighter, uint8_t orig)
    {
        lighter -= orig;
    },
    lighter, image
    );
    return lighter;
}

GrayImage morphGradientDetectLightSide(GrayImage const& image, QSize const& area)
{
    GrayImage darker(dilateGray(image, area, 0xff));
    rasterOpGeneric(
        [](uint8_t& darker, uint8_t orig)
    {
        darker = orig - darker;
    },
    darker, image
    );
    return darker;
}

} // namespace imageproc
