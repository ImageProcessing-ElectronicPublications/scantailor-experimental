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
#include "SubScale.h"

namespace page_layout
{

SubScale::SubScale(QDomElement const& el)
    : m_width(el.attribute("width").toDouble())
    , m_height(el.attribute("height").toDouble())
{
    if ((m_width < 0.25) || (m_width > 2.0))
    {
        m_width = 1.0;
    }
    if ((m_height < 0.25) || (m_height > 2.0))
    {
        m_height = 1.0;
    }
}

QDomElement
SubScale::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("width", m_width);
    el.setAttribute("height", m_height);
    return el;
}

} // namespace page_layout
