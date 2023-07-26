/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2015  Joseph Artsimovich <joseph_a@mail.ru>

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

#ifndef IMAGEMETADATA_H_
#define IMAGEMETADATA_H_

#include <QSize>

class ImageMetadata
{
    // Member-wise copying is OK.
public:
    ImageMetadata() {}

    ImageMetadata(QSize size) : m_size(size) {}

    QSize const& size() const
    {
        return m_size;
    }

    void setSize(QSize const& size)
    {
        m_size = size;
    }

    bool operator==(ImageMetadata const& other) const;

    bool operator!=(ImageMetadata const& other) const
    {
        return !(*this == other);
    }
private:
    QSize m_size;
};

#endif
