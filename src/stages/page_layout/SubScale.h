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

#ifndef PAGE_LAYOUT_SUBSCALE_H_
#define PAGE_LAYOUT_SUBSCALE_H_

class QDomDocument;
class QDomElement;
class QString;

namespace page_layout
{

class SubScale
{
public:
    SubScale() : m_width(1.0), m_height(1.0) {}

    SubScale(double hor, double vert) : m_width(hor), m_height(vert) {}

    SubScale(QDomElement const& el);

    double getWidth() const
    {
        return m_width;
    }
    void setWidth(double value)
    {
        m_width = value;
    }

    double getHeight() const
    {
        return m_height;
    }
    void setHeight(double value)
    {
        m_height = value;
    }

    bool operator==(SubScale const& other) const
    {
        return m_width == other.m_width && m_height == other.m_height;
    }

    bool operator!=(SubScale const& other) const
    {
        return !(*this == other);
    }

    QDomElement toXml(QDomDocument& doc, QString const& name) const;
private:
    double m_width;
    double m_height;
};

} // namespace page_layout

#endif
