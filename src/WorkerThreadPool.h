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

#ifndef WORKERTHREADPOOL_H_
#define WORKERTHREADPOOL_H_

#include "BackgroundTask.h"
#include "FilterResult.h"
#include <QObject>
#include <QSettings>
#include <memory>

class QThreadPool;

class WorkerThreadPool : public QObject
{
    Q_OBJECT
public:
    WorkerThreadPool(QObject* parent = nullptr);

    virtual ~WorkerThreadPool();

    /**
     * \brief Waits for pending jobs to finish and stop the thread.
     *
     * The destructor also performs these tasks, so this method is only
     * useful to prematuraly stop task processing.
     */
    void shutdown();

    bool hasSpareCapacity() const;

    void submitTask(BackgroundTaskPtr const& task);
signals:
    void taskResult(BackgroundTaskPtr const& task, FilterResultPtr const& result);
private:
    class TaskResultEvent;

    virtual void customEvent(QEvent* event) override;

    void updateNumberOfThreads();

    QThreadPool* m_pPool;
    QSettings m_settings;
};

#endif
