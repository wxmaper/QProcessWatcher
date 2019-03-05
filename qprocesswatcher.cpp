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

#include "qprocesswatcher.h"
#include "qprocesswatcher_p.h"

#include <QThread>
#include <QTimer>
#include <QCoreApplication>

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#include <tlhelp32.h>
#else
#include <Windows.h>
#include <TlHelp32.h>
#endif

#ifdef PROCESS_WATCHER_DEBUG
#include <QDebug>
#define PW_DEBUG(d) qDebug() << d
#else
#define PW_DEBUG(d)
#endif

QProcessWatcher::QProcessWatcher(quint32 timerInterval, QObject *parent)
    : QObject(parent),
      m_watcher(new QProcessWatcherPrivate(timerInterval)),
      m_watcherThread(new QThread)
{
    m_watcher->moveToThread(m_watcherThread);

    connect(this, &QProcessWatcher::doWatchProcesses,
            m_watcher, &QProcessWatcherPrivate::checkProcesses);

    connect(m_watcher, &QProcessWatcherPrivate::processStarted,
            this, &QProcessWatcher::processStarted);
    connect(m_watcher, &QProcessWatcherPrivate::processFinished,
            this, &QProcessWatcher::processFinished);

    connect(m_watcherThread, &QThread::finished,
            m_watcher, &QProcessWatcherPrivate::deleteLater);
    connect(m_watcherThread, &QThread::started,
            m_watcher, &QProcessWatcherPrivate::createTimer);

    m_watcherThread->start();
}

QProcessWatcher::~QProcessWatcher()
{
    PW_DEBUG(Q_FUNC_INFO);

    m_watcherThread->quit();

    if (!m_watcherThread->wait(2000)) {
        m_watcherThread->terminate();
        m_watcherThread->wait();
    }
}

void QProcessWatcher::setWatchingInterval(quint32 msecs)
{
    m_watcher->setTimerInterval(msecs);
}

void QProcessWatcher::addProcess(const QString &processName)
{
    m_watcher->addProcess(processName);
}

bool QProcessWatcher::removeProcess(const QString &processName)
{
    return m_watcher->removeProcess(processName);
}

void QProcessWatcher::watchProcesses()
{
    emit doWatchProcesses();
}

// ===================== //
// ProcessWatcherPrivate //
// ===================== //

QProcessWatcherPrivate::QProcessWatcherPrivate(quint32 timerInterval)
    : QObject(),
      m_checkTimer(nullptr),
      m_timerInterval(timerInterval)
{
}

QProcessWatcherPrivate::~QProcessWatcherPrivate()
{
    PW_DEBUG(Q_FUNC_INFO);

    if (m_checkTimer) {
        m_checkTimer->stop();
    }
}

void QProcessWatcherPrivate::setTimerInterval(quint32 msecs)
{
    QMutexLocker locker(&m_mutex);
    m_timerInterval = msecs;

    if (m_checkTimer != nullptr && m_checkTimer->isActive()) {
        stop();
        start();
    }
}

void QProcessWatcherPrivate::addProcess(const QString &processName)
{
    PW_DEBUG(QString("add watched process `%1`")
             .arg(processName));

    QMutexLocker locker(&m_mutex);
    stop();

    const QString lowerProcessName = processName.toLower();
    m_processesMap.insert(lowerProcessName, processName);
    m_processesStatus.insert(lowerProcessName, false);

    start();
}

bool QProcessWatcherPrivate::removeProcess(const QString &processName)
{
    PW_DEBUG(QString("remove watched process `%1`")
             .arg(processName));

    QMutexLocker locker(&m_mutex);
    stop();

    const QString lowerProcessName = processName.toLower();

    bool result = m_processesMap.contains(lowerProcessName);
    if (result) {
        m_processesMap.remove(lowerProcessName);
        m_processesStatus.remove(lowerProcessName);
    }

    start();

    return result;
}

void QProcessWatcherPrivate::checkProcesses()
{
    QMutexLocker locker(&m_mutex);

    QStringList processesList = m_processesMap.keys();
    int size = processesList.size();
    if (size == 0)
        return;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry)) {
        QString originalProcessName;
        QString lowerProcessName;
        int index;

        do {
            originalProcessName = QString::fromWCharArray(entry.szExeFile);
            lowerProcessName = originalProcessName.toLower();

            if ((index = processesList.indexOf(lowerProcessName)) >= 0) {
                processExists(lowerProcessName, originalProcessName);
                processesList.removeAt(index);
                size--;
            }
        } while (size > 0 && Process32Next(snapshot, &entry));
        // We don't need check all running processes if processesList is empty
    }

    CloseHandle(snapshot);

    for (int i = 0; i < size; i++) {
        processNotExists(processesList.at(i));
    }
}

void QProcessWatcherPrivate::createTimer()
{
    QMutexLocker locker(&m_mutex);

    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(m_timerInterval);

    connect(m_checkTimer, &QTimer::timeout,
            this, &QProcessWatcherPrivate::checkProcesses);

    start();
}

void QProcessWatcherPrivate::start()
{
    if (m_checkTimer == nullptr) {
        return;
    }

    m_checkTimer->setInterval(m_timerInterval);
    QTimer::singleShot(0, m_checkTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
}

void QProcessWatcherPrivate::stop()
{
    if (m_checkTimer == nullptr) {
        return;
    }

    QTimer::singleShot(0, m_checkTimer, &QTimer::stop);
}

void QProcessWatcherPrivate::processExists(const QString &lowerProcessName,
                                           const QString &originalProcessName)
{
    if (m_processesStatus.value(lowerProcessName) == false) {
        m_processesStatus.insert(lowerProcessName, true);
        emit processStarted(m_processesMap.value(lowerProcessName), originalProcessName);

        PW_DEBUG(QString("process `%1` started with name `%2`")
                 .arg(m_processesMap.value(lowerProcessName))
                 .arg(originalProcessName));
    }
}

void QProcessWatcherPrivate::processNotExists(const QString &lowerProcessName)
{
    if (m_processesStatus.value(lowerProcessName) == true) {
        m_processesStatus.insert(lowerProcessName, false);
        emit processFinished(m_processesMap.value(lowerProcessName));

        PW_DEBUG(QString("process `%1` finished")
                 .arg(m_processesMap.value(lowerProcessName)));
    }
}
