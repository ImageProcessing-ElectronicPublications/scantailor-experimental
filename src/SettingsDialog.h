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

#ifndef SETTINGS_DIALOG_H_
#define SETTINGS_DIALOG_H_

#include "ui_SettingsDialog.h"
#include <QDialog>

class AccelerationPlugin;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = 0);

    virtual ~SettingsDialog();
signals:
    /**
     * This signal should be used by clients instead of accepted(),
     * as settings are actually updated from a slot connected to accepted().
     */
    void settingsUpdated();

    void stylesheetChanged(const QString stylesheetFilePath);
private slots:
    void accept();

    void reject();

    void on_stylesheetCombo_currentIndexChanged(int index);
private:
    void saveOldSettings();

    void setupStylesheetsCombo();
private:
    Ui::SettingsDialog ui;
    AccelerationPlugin* m_pOpenCLPlugin;
    QString m_oldStylesheetFilePath;
};

#endif
