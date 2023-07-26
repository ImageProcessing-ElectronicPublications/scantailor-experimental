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

#ifndef OUTPUT_TABBED_IMAGE_VIEW_H_
#define OUTPUT_TABBED_IMAGE_VIEW_H_

#include "ImageViewTab.h"
#include <QTabWidget>
#include <map>

namespace output
{

class TabbedImageView : public QTabWidget
{
    Q_OBJECT
public:
    TabbedImageView(QWidget* parent = 0);

    void addTab(QWidget* widget, QString const& label, ImageViewTab tab);
public slots:
    void setCurrentTab(ImageViewTab tab);
signals:
    void tabChanged(ImageViewTab tab);
private slots:
    void tabChangedSlot(int idx);
private:
    std::map<QWidget*, ImageViewTab> m_registry;
};

} // namespace output

#endif
