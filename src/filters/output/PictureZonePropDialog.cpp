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

#include "PictureZonePropDialog.h"
#include "Property.h"
#include "PictureLayerProperty.h"

namespace output
{

PictureZonePropDialog::PictureZonePropDialog(
    IntrusivePtr<PropertySet> const& props, QWidget* parent)
    :   QDialog(parent),
        m_ptrProps(props)
{
    ui.setupUi(this);

    switch (m_ptrProps->locateOrDefault<PictureLayerProperty>()->layer())
    {
    case PictureLayerProperty::ZONENOOP:
        break;
    case PictureLayerProperty::ZONEERASER:
        ui.zoneeraser->setChecked(true);
        break;
    case PictureLayerProperty::ZONEPAINTER:
        ui.zonepainter->setChecked(true);
        break;
    case PictureLayerProperty::ZONECLEAN:
        ui.zoneclean->setChecked(true);
        break;
    case PictureLayerProperty::ZONEFG:
        ui.zonefg->setChecked(true);
        break;
    case PictureLayerProperty::ZONEBG:
        ui.zonebg->setChecked(true);
        break;
    case PictureLayerProperty::ZONEMASK:
        ui.zonemask->setChecked(true);
        break;
    case PictureLayerProperty::ZONENOKMEANS:
        ui.zonenokmeans->setChecked(true);
        break;
    }

    connect(ui.zoneeraser, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonepainter, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zoneclean, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonefg, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonebg, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonemask, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonenokmeans, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
}

void
PictureZonePropDialog::itemToggled(bool selected)
{
    PictureLayerProperty::Layer layer = PictureLayerProperty::ZONENOOP;

    QObject* const obj = sender();
    if (obj == ui.zoneeraser)
    {
        layer = PictureLayerProperty::ZONEERASER;
    }
    else if (obj == ui.zonepainter)
    {
        layer = PictureLayerProperty::ZONEPAINTER;
    }
    else if (obj == ui.zoneclean)
    {
        layer = PictureLayerProperty::ZONECLEAN;
    }
    else if (obj == ui.zonefg)
    {
        layer = PictureLayerProperty::ZONEFG;
    }
    else if (obj == ui.zonebg)
    {
        layer = PictureLayerProperty::ZONEBG;
    }
    else if (obj == ui.zonemask)
    {
        layer = PictureLayerProperty::ZONEMASK;
    }
    else if (obj == ui.zonenokmeans)
    {
        layer = PictureLayerProperty::ZONENOKMEANS;
    }

    m_ptrProps->locateOrCreate<PictureLayerProperty>()->setLayer(layer);

    emit updated();
}

} // namespace output
