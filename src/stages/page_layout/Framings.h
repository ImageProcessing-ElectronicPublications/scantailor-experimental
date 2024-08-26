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

#ifndef PAGE_LAYOUT_FRAMINGS_H_
#define PAGE_LAYOUT_FRAMINGS_H_

class QDomDocument;
class QDomElement;
class QString;

namespace page_layout
{

class Framings
{
public:
    Framings() : m_framingw(0.12), m_framingh(0.08) {}

    Framings(double hor, double vert) : m_framingw(hor), m_framingh(vert) {}

    Framings(QDomElement const& el);

    double getFramingWidth() const
    {
        return m_framingw;
    }
    void setFramingWidth(double value)
    {
        m_framingw = value;
    }

    double getFramingHeight() const
    {
        return m_framingh;
    }
    void setFramingHeight(double value)
    {
        m_framingh = value;
    }

    bool operator==(Framings const& other) const
    {
        return m_framingw == other.m_framingw && m_framingh == other.m_framingh;
    }

    bool operator!=(Framings const& other) const
    {
        return !(*this == other);
    }

    QDomElement toXml(QDomDocument& doc, QString const& name) const;
private:
    double m_framingw;
    double m_framingh;
};

} // namespace page_layout

#endif
