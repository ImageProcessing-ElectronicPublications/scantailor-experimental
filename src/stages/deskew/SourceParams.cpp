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

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include "SourceParams.h"

namespace deskew
{

SourceParams::SourceParams()
    : m_focus(1400.0)
    , m_photo(false)
{
}

SourceParams::SourceParams(double const& focus, bool const& photo)
    : m_focus(focus)
    , m_photo(photo)
{
}

SourceParams::SourceParams(QDomElement const& el)
    : m_photo(el.attribute("photo") == "1")
{
    double focus = el.attribute("focus").toDouble();
    m_focus = (focus > 0.0) ? focus : 1400.0;
}

QDomElement
SourceParams::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("photo", (m_photo) ? "1" : "0");
    el.setAttribute("focus", m_focus);
    return el;
}

bool
SourceParams::operator==(SourceParams const& other) const
{
    if (m_photo != other.m_photo)
    {
        return false;
    }
    if (m_focus != other.m_focus)
    {
        return false;
    }

    return true;
}

bool
SourceParams::operator!=(SourceParams const& other) const
{
    return !(*this == other);
}

} // namespace deskew
