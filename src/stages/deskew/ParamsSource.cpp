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
#include "ParamsSource.h"

namespace deskew
{

ParamsSource::ParamsSource()
    : m_fov(0.7)
    , m_photo(false)
{
}

ParamsSource::ParamsSource(double const& fov, bool const& photo)
    : m_fov(fov)
    , m_photo(photo)
{
}

ParamsSource::ParamsSource(QDomElement const& el)
    : m_photo(el.attribute("photo") == "1")
{
    double fov = el.attribute("fov").toDouble();
    m_fov = (fov < 0.0) ? 0.0 : fov;
}

QDomElement
ParamsSource::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("photo", (m_photo) ? "1" : "0");
    el.setAttribute("fov", m_fov);
    return el;
}

bool
ParamsSource::operator==(ParamsSource const& other) const
{
    if (m_photo != other.m_photo)
    {
        return false;
    }
    if (m_fov != other.m_fov)
    {
        return false;
    }

    return true;
}

bool
ParamsSource::operator!=(ParamsSource const& other) const
{
    return !(*this == other);
}

} // namespace deskew
