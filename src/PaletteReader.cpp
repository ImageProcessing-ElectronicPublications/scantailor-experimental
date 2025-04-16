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

#include "PaletteReader.h"

bool
PaletteReader::readPalette(const QDomDocument& doc, QPalette& pal)
{
    QPalette newPal(pal);
    QDomNodeList groups = doc.elementsByTagName("group");

    for(int i = 0; i < groups.length(); i++)
    {
        QDomNode groupNode = groups.at(i);
        QPalette::ColorGroup paletteGroup = QPalette::Normal;

        if (!groupNode.isElement())
        {
            return false;
        }

        QDomElement groupElement = groupNode.toElement();

        QString groupName = groupElement.attribute("name");

        if (groupName == "Active")
        {
            paletteGroup = QPalette::Active;
        }
        else if (groupName == "Disabled")
        {
            paletteGroup = QPalette::Disabled;
        }
        else if (groupName == "Inactive")
        {
            paletteGroup = QPalette::Inactive;
        }
        else
        {
            continue;
        }

        if (!readPaletteGroup(groupElement, paletteGroup, newPal))
        {
            return false;
        }
    }

    pal = newPal;

    return true;
}

bool
PaletteReader::readPaletteGroup(const QDomElement& groupElement, QPalette::ColorGroup colorGroup, QPalette& pal)
{
    QDomNodeList roles = groupElement.elementsByTagName("role");

    for(int i = 0; i < roles.length(); i++)
    {
        QDomNode roleNode = roles.at(i);
        QPalette::ColorRole paletteRole = QPalette::NoRole;

        if (!roleNode.isElement())
        {
            return false;
        }

        QDomElement roleElement = roleNode.toElement();

        QString roleName = roleElement.attribute("name");

        if (roleName == "Highlight")
        {
            paletteRole = QPalette::Highlight;
        }

        if (!readPaletteRole(roleElement, colorGroup, paletteRole, pal))
        {
            return false;
        }
    }

    return true;
}

bool
PaletteReader::readPaletteRole(const QDomElement& roleElement, QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole, QPalette& pal)
{
    QColor color(roleElement.text());

    pal.setColor(colorGroup, colorRole, color);

    return true;
}
