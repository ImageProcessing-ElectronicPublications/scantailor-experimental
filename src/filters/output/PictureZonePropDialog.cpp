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
    case PictureLayerProperty::ZONEERASER1:
        ui.zoneeraser1->setChecked(true);
        break;
    case PictureLayerProperty::ZONEPAINTER2:
        ui.zonepainter2->setChecked(true);
        break;
    case PictureLayerProperty::ZONEERASER3:
        ui.zoneeraser3->setChecked(true);
        break;
    case PictureLayerProperty::ZONEFG:
        ui.zonefg->setChecked(true);
        break;
    case PictureLayerProperty::ZONEBG:
        ui.zonebg->setChecked(true);
        break;
    }

    connect(ui.zoneeraser1, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonepainter2, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zoneeraser3, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonefg, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
    connect(ui.zonebg, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
}

void
PictureZonePropDialog::itemToggled(bool selected)
{
    PictureLayerProperty::Layer layer = PictureLayerProperty::ZONENOOP;

    QObject* const obj = sender();
    if (obj == ui.zoneeraser1)
    {
        layer = PictureLayerProperty::ZONEERASER1;
    }
    else if (obj == ui.zonepainter2)
    {
        layer = PictureLayerProperty::ZONEPAINTER2;
    }
    else if (obj == ui.zoneeraser3)
    {
        layer = PictureLayerProperty::ZONEERASER3;
    }
    else if (obj == ui.zonefg)
    {
        layer = PictureLayerProperty::ZONEFG;
    }
    else if (obj == ui.zonebg)
    {
        layer = PictureLayerProperty::ZONEBG;
    }

    m_ptrProps->locateOrCreate<PictureLayerProperty>()->setLayer(layer);

    emit updated();
}

} // namespace output
