/*****************************************************************************
**                                                                          **
** Copyright (C) 2019 WxMaper (https://wxmaper.ru/)                         **
**                                                                          **
** This file is part of the QProcessWatcher class.                          **
**                                                                          **
** BEGIN LICENSE: MPL 2.0                                                   **
**                                                                          **
** This Source Code Form is subject to the terms of the Mozilla Public      **
** License, v. 2.0. If a copy of the MPL was not distributed with this      **
** file, You can obtain one at http://mozilla.org/MPL/2.0/.                 **
**                                                                          **
** END LICENSE                                                              **
**                                                                          **
*****************************************************************************/

#ifndef QPROCESSWATCHER_H
#define QPROCESSWATCHER_H

#include <QThread>

#ifndef Q_OS_WIN
#error "Use this header only in Windows"
#endif

class QProcessWatcherPrivate;

/*!
 * The QProcessWatcher class provides an simple interface for monitoring state of
 * processes (for Windows only).
 *
 * Use addProcess() method for add a process for monitoring by passing a
 * case insensitive process name.
 *
 * The signal processStarted() will be emitted in two cases:
 * 1. If the process is already started before calling addProcess() method.
 * 2. When the first copy of process is started.
 *
 * The signal processFinished() will be emitted when all copies of monitored
 * process is finished.
 */
class QProcessWatcher : public QObject
{
    Q_OBJECT

public:
    /*!
     * Constructs a new process watcher object with the given `parent` and
     * interval in milliseconds between checks the process state
     * specified by `timerInterval`.
     *
     * By default timerInterval value is 1000 ms (1 sec).
     */
    explicit QProcessWatcher(quint32 timerInterval = 1000, QObject *parent = nullptr);

    /*!
     * Destroys the process watcher and all tasks if exists.
     */
    ~QProcessWatcher();

    /*!
     * Sets interval in milliseconds between checks the process state.
     * All timers will be restarted.
     */
    void setWatchingInterval(quint32 msecs);

    /*!
     * Adds `processName` for checking the process state.
     * The `processName` is not added if it is already being monitored by the process watcher.
     *
     * The processStarted() signal will be emitted when first process with name `processName`
     * is started or if process has already been started.
     *
     * The processFinished() signal will be emitted when all processes with name `processName`
     * is finished.
     */
    void addProcess(const QString &processName);

    /*!
     * Removes the specified `processName` from the process watcher.
     * If the watch is successfully removed, true is returned.
     */
    bool removeProcess(const QString &processName);

public slots:
    /*!
     * Use this slot for forced check state of all watched processes.
     * The doWatchProcesses() signal will be emitted.
     */
    void watchProcesses();

signals:
    /*!
     * This signal emitted only after calling a watchProcesses() slot.
     */
    void doWatchProcesses();

    /*!
     * This signal emitted when first watched process with name `processName`
     * is started or if process has already been started.
     * The `originalProcessName` contains a original process name of running process.
     */
    void processStarted(const QString &processName, const QString &originalProcessName);

    /*!
     * This signal emitted when all copies of watched process with name `processName`
     * is finished.
     */
    void processFinished(const QString &processName);

private:
    Q_DISABLE_COPY(QProcessWatcher)
    QProcessWatcherPrivate *m_watcher;
    QThread *m_watcherThread;
};

#endif // QPROCESSWATCHER_H
