/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2009  Joseph Artsimovich <joseph_a@mail.ru>

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

#ifndef PALETTEREADER_H
#define PALETTEREADER_H

#include <QDomDocument>
#include <QPalette>

class PaletteReader
{
public:
    /**
     * \brief Read palette from xml.
     *
     * Read palette from xml dom document. Return true on success.
     * Doesn't change palette on failure.
     */
     bool readPalette(const QDomDocument& doc, QPalette& pal);
private:
    /**
     * \brief Read palette group from xml.
     *
     * Read palette group from xml dom document. Return true on success.
     */
     bool readPaletteGroup(const QDomElement& groupElement, QPalette::ColorGroup colorGroup, QPalette& pal);
    /**
     * \brief Read palette from xml.
     *
     * Read palette role from xml dom document. Return true on success.
     */
     bool readPaletteRole(const QDomElement& roleElement, QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole, QPalette& pal);
};

#endif //PALETTEREADER_H
