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

#include "config.h"
#include "Application.h"
#include "MainWindow.h"
//#include "PngMetadataLoader.h"
#include "TiffMetadataLoader.h"
//#include "JpegMetadataLoader.h"
#include "FastImageMetadataLoader.h"
#include "foundation/MultipleTargetsSupport.h"
#include <boost/range/adaptor/reversed.hpp>
#include <QMetaType>
#include <QtPlugin>
#include <QLocale>
#include <QSettings>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <Qt>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QImageReader>
#endif
#include <string.h>

#include "CommandLine.h"

int main(int argc, char** argv)
{
    Application app(argc, argv);
    QIcon icon(":icons/appicon-about.png");
    app.setWindowIcon(icon);
#ifdef _WIN32
    // Get rid of all references to Qt's installation directory.
    app.setLibraryPaths(QStringList(app.applicationDirPath()));
#endif

    // parse command line arguments
    CommandLine cli(app.arguments());
    CommandLine::set(cli);

    if (cli.hasHelp())
    {
        cli.printHelp();
        return 0;
    }

    QString const translation("scantailor-experimental_"+QLocale::system().name());
    QString const translation_qtbase("qtbase_"+QLocale::system().name());
    QTranslator translator, translator_qtbase;
    bool found_translation = false, found_translation_qtbase = false;

    // Try loading translations from different paths.
    QStringList const translation_dirs(
        QString::fromUtf8(TRANSLATION_DIRS).split(QChar(':'), QStringSkipEmptyParts)
    );
    for (QString const& path : translation_dirs)
    {
        QString absolute_path;
        if (QDir::isAbsolutePath(path))
        {
            absolute_path = path;
        }
        else
        {
            absolute_path = app.applicationDirPath();
            absolute_path += QChar('/');
            absolute_path += path;
        }
        absolute_path += QChar('/');

        if(!found_translation)
        {
            if (translator.load(absolute_path+translation))
            {
                found_translation = true;
            }
        }

        if(!found_translation_qtbase)
        {
            if (translator_qtbase.load(absolute_path+translation_qtbase))
            {
                found_translation_qtbase = true;
            }
        }

        if(found_translation && found_translation_qtbase) break;
    }

    app.installTranslator(&translator);
    app.installTranslator(&translator_qtbase);

    // Plugin search paths.
    QStringList const plugin_dirs(
        QString::fromUtf8(PLUGIN_DIRS).split(QChar(':'), QStringSkipEmptyParts)
    );
    // Reversing, as QCoreApplication::addLibraryPath() prepends the new path to the list.
    for (QString const& path : boost::adaptors::reverse(plugin_dirs))
    {
        QString absolute_path;
        if (QDir::isAbsolutePath(path))
        {
            absolute_path = path;
        }
        else
        {
            absolute_path = app.applicationDirPath();
            absolute_path += QChar('/');
            absolute_path += path;
        }
        app.addLibraryPath(absolute_path);
    }

    // This information is used by QSettings.
    app.setApplicationName("Scan Tailor Experimental");
    app.setOrganizationName("Scan Tailor");
    app.setOrganizationDomain("scantailor.sourceforge.net");
    QSettings settings;

    //PngMetadataLoader::registerMyself();
    TiffMetadataLoader::registerMyself();
    //JpegMetadataLoader::registerMyself();
    FastImageMetadataLoader::registerMyself();

    MainWindow* main_wnd = new MainWindow();
    main_wnd->setAttribute(Qt::WA_DeleteOnClose);
    if (settings.value("mainWindow/maximized") == false)
    {
        main_wnd->show();
    }
    else
    {
        main_wnd->showMaximized();
    }

    if (!cli.projectFile().isEmpty())
    {
        main_wnd->openProject(cli.projectFile());
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(0);
#endif

    return app.exec();
}
