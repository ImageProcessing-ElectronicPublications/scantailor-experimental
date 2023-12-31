/*
	Scan Tailor - Interactive post-processing tool for scanned pages.
	Copyright (C)  Joseph Artsimovich <joseph_a@mail.ru>

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

#include "OpenGLSupport.h"
#include "config.h"
#include <QSettings>
#ifdef ENABLE_OPENGL
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#endif

bool
OpenGLSupport::supported()
{
#ifndef ENABLE_OPENGL
    return false;
#else
    QSurfaceFormat format;
    format.setSamples(2);
    format.setAlphaBufferSize(8);

    QOpenGLContext context;
    context.setFormat(format);
    if (!context.create())
    {
        return false;
    }
    format = context.format();

    if (format.samples() < 2)
    {
        return false;
    }
    if (!format.hasAlpha())
    {
        return false;
    }

    return true;
#endif
}

QString
OpenGLSupport::deviceName()
{
    QString name;
#ifdef ENABLE_OPENGL
    QOpenGLContext context;
    QOffscreenSurface surface;
    if (context.create() && (surface.create(), true) && context.makeCurrent(&surface))
    {
        name = QString::fromUtf8((char const*)context.functions()->glGetString(GL_RENDERER));
        context.doneCurrent();
    }
#endif
    return name;
}
