/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "NewOpenProjectPanel.h"
#include "RecentProjects.h"
#include "Utils.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QRegion>
#include <QSettings>
#include <QString>
#include <QFileInfo>
#include <QLabel>
#include <Qt>
#include <QDebug>
#include <algorithm>

NewOpenProjectPanel::NewOpenProjectPanel(QWidget* parent)
    :	QWidget(parent)
{
    setupUi(this);

    recentProjectsGroup->setLayout(new QVBoxLayout);

    RecentProjects rp;
    rp.read();
    if (!rp.validate())
    {
        // Some project files weren't found.
        // Write the list without them.
        rp.write();
    }
    if (rp.isEmpty())
    {
        recentProjectsGroup->setVisible(false);
    }
    else
    {
        rp.enumerate([this](QString const& path)
        {
            addRecentProject(path);
        });
    }

    connect(
        newProjectButton, SIGNAL(clicked(bool)),
        this, SIGNAL(newProject())
    );
    connect(
        openProjectButton, SIGNAL(clicked(bool)),
        this, SIGNAL(openProject())
    );
}

void
NewOpenProjectPanel::addRecentProject(QString const& file_path)
{
    QFileInfo const file_info(file_path);
    QString base_name(file_info.baseName());
    if (base_name.isEmpty())
    {
        base_name = QChar('_');
    }

    const int max_length = 30;
    if (base_name.length() > max_length)
    {
        base_name.truncate(max_length);
        base_name.append("...");
    }

    QPushButton* button = new QPushButton(recentProjectsGroup);
    button->setText(base_name);

    button->setToolTip(file_path);
    recentProjectsGroup->layout()->addWidget(button);

    connect(
        button, SIGNAL(clicked()),
        this, SLOT(recentProjectButtonClicked())
    );
}

void
NewOpenProjectPanel::recentProjectButtonClicked()
{
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
        openRecentProject(button->toolTip());
}

void
NewOpenProjectPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    // In fact Qt doesn't draw QWidget's background, unless
    // autoFillBackground property is set, so we can safely
    // draw our borders and shadows in the margins area.

    int left = 0, top = 0, right = 0, bottom = 0;
    layout()->getContentsMargins(&left, &top, &right, &bottom);

    QRect const widget_rect(rect());
    QRect const except_margins(
        widget_rect.adjusted(left, top, -right, -bottom)
    );

    int const border = 1; // Solid line border width.
    int const shadow = std::min(right, bottom) - border;

    QPainter painter(this);

    // Draw the border.
    painter.setPen(QPen(palette().windowText(), border));

    // Note that we specify the coordinates excluding
    // pen width.
    painter.drawRect(except_margins);

    QColor const dark(Qt::darkGray);
    QColor const light(Qt::transparent);

    if (shadow <= 0)
    {
        return;
    }

    // Let's adjust the margins to exclude borders.
    left -= border;
    right -= border;
    top -= border;
    bottom -= border;

    // This rectangle extends 1 pixel into each shadow area.
    QRect const extended(
        except_margins.adjusted(
            -border - 1, -border - 1, border + 1, border + 1
        )
    );

    // Right shadow.
    {
        QRect rect(widget_rect);
        rect.setWidth(shadow);
        rect.moveLeft(extended.right());
        rect.adjust(0, top + shadow, 0, -bottom);
        QLinearGradient grad(rect.topLeft(), rect.topRight());
        grad.setColorAt(0, dark);
        grad.setColorAt(1, light);
        painter.fillRect(rect, grad);
    }

    // Down shadow.
    {
        QRect rect(widget_rect);
        rect.setHeight(shadow);
        rect.moveTop(extended.bottom());
        rect.adjust(left + shadow, 0, -right, 0);
        QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
        grad.setColorAt(0, dark);
        grad.setColorAt(1, light);
        painter.fillRect(rect, grad);
    }

    // Bottom-right corner.
    {
        QRect rect(0, 0, shadow, shadow);
        rect.moveTopLeft(extended.bottomRight());
        QRadialGradient grad(rect.topLeft(), shadow);
        grad.setColorAt(0, dark);
        grad.setColorAt(1, light);
        painter.fillRect(rect, grad);
    }

    // Top-right corner.
    {
        QRect rect(0, 0, shadow, shadow);
        rect.moveTopLeft(extended.topRight() + QPoint(0, border));
        QRadialGradient grad(rect.bottomLeft(), shadow);
        grad.setColorAt(0, dark);
        grad.setColorAt(1, light);
        painter.fillRect(rect, grad);
    }

    // Bottom-left corner.
    {
        QRect rect(0, 0, shadow, shadow);
        rect.moveTopLeft(extended.bottomLeft() + QPoint(border, 0));
        QRadialGradient grad(rect.topRight(), shadow);
        grad.setColorAt(0, dark);
        grad.setColorAt(1, light);
        painter.fillRect(rect, grad);
    }
}
