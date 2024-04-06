/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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
#include <QByteArray>
#include <QString>
#include "Params.h"
#include "ColorGrayscaleOptions.h"
#include "BlackWhiteOptions.h"
#include "BlackKmeansOptions.h"
#include "XmlMarshaller.h"
#include "XmlUnmarshaller.h"

namespace output
{

Params::Params()
    :   m_despeckleLevel(DESPECKLE_CAUTIOUS),
        m_despeckleFactor(despeckleLevelToFactor(DESPECKLE_CAUTIOUS))
{
}

Params::Params(QDomElement const& el)
    :   m_despeckleLevel(despeckleLevelFromString(el.attribute("despeckleLevel"))),
        m_despeckleFactor(el.attribute("despeckleFactor").toDouble())
{
    QDomElement const cp(el.namedItem("color-params").toElement());
    m_colorParams.setColorMode(parseColorMode(cp.attribute("colorMode")));
    m_colorParams.setColorGrayscaleOptions(
        ColorGrayscaleOptions(
            cp.namedItem("color-or-grayscale").toElement()
        )
    );
    m_colorParams.setBlackWhiteOptions(
        BlackWhiteOptions(cp.namedItem("bw").toElement())
    );
    m_colorParams.setBlackKmeansOptions(
        BlackKmeansOptions(cp.namedItem("kmeans").toElement())
    );
}

QDomElement
Params::toXml(QDomDocument& doc, QString const& name) const
{
    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute("despeckleLevel", despeckleLevelToString(m_despeckleLevel));
    el.setAttribute("despeckleFactor", m_despeckleFactor);

    QDomElement cp(doc.createElement("color-params"));
    cp.setAttribute(
        "colorMode",
        formatColorMode(m_colorParams.colorMode())
    );

    cp.appendChild(
        m_colorParams.colorGrayscaleOptions().toXml(
            doc, "color-or-grayscale"
        )
    );
    cp.appendChild(m_colorParams.blackWhiteOptions().toXml(doc, "bw"));
    cp.appendChild(m_colorParams.blackKmeansOptions().toXml(doc, "kmeans"));

    el.appendChild(cp);

    return el;
}

ColorParams::ColorMode
Params::parseColorMode(QString const& str)
{
    if (str == "bw")
    {
        return ColorParams::BLACK_AND_WHITE;
    }
    else if (str == "bitonal")
    {
        // Backwards compatibility.
        return ColorParams::BLACK_AND_WHITE;
    }
    else if (str == "colorOrGray")
    {
        return ColorParams::COLOR_GRAYSCALE;
    }
    else if (str == "mixed")
    {
        return ColorParams::MIXED;
    }
    else
    {
        return ColorParams::BLACK_AND_WHITE;
    }
}

QString
Params::formatColorMode(ColorParams::ColorMode const mode)
{
    char const* str = "";
    switch (mode)
    {
    case ColorParams::BLACK_AND_WHITE:
        str = "bw";
        break;
    case ColorParams::COLOR_GRAYSCALE:
        str = "colorOrGray";
        break;
    case ColorParams::MIXED:
        str = "mixed";
        break;
    }
    return QString::fromLatin1(str);
}

} // namespace output
