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

#ifndef QPROCESSWATCHER_P_H
#define QPROCESSWATCHER_P_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QPointer>

class QTimer;

class QProcessWatcherPrivate : public QObject
{
    Q_OBJECT

public:
    explicit QProcessWatcherPrivate(quint32 timerInterval);
    ~QProcessWatcherPrivate();

    void setTimerInterval(quint32 msecs);
    void addProcess(const QString &processName);
    bool removeProcess(const QString &processName);

signals:
    void processStarted(const QString &processName, const QString &originalProcessName);
    void processFinished(const QString &processName);

public slots:
    void checkProcesses();
    void createTimer();

    void start();
    void stop();

private:
    void processExists(const QString &lowerProcessName,
                       const QString &originalProcessName);
    void processNotExists(const QString &lowerProcessName);

private:
    QTimer *m_checkTimer;
    quint32 m_timerInterval;

    QHash<QString, QString> m_processesMap; // <lowerName, originalName>
    QHash<QString, bool> m_processesStatus; // <lowerName, status>

    mutable QMutex m_mutex;
};

#endif // QPROCESSWATCHER_P_H
