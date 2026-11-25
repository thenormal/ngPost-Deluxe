//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================

#include "NgPost.h"
#include "utils/SpeedLimiter.h"
#include "utils/PostedMover.h"
#include <QSaveFile>
#include "utils/Diagnostics.h"
#include "PostingJob.h"
#include "PostQueue.h"
#include "utils/PostList.h"
#include "FoldersMonitorForNewFiles.h"
#include "nntp/NntpArticle.h"
#include "nntp/NntpServerParams.h"
#include "FileUploader.h"
#include "NzbCheck.h"
#ifdef __USE_HMI__
  #include "hmi/MainWindow.h"
  #include "hmi/PostingWidget.h"
  #include "hmi/AutoPostWidget.h"
#endif

#include <cmath>
#ifdef __USE_HMI__
  #include <QApplication>
  #include <QMessageBox>
  #include <QPalette>
  #include <QFile>
  #include <QSettings>
  #include <QStyle>
#else
  #include <QCoreApplication>
#endif
#include <QThread>
#include <QSysInfo>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  #include <QRandomGenerator>
#endif
#include <QTextStream>
#include <QCommandLineParser>
#include <QDebug>
#include <iostream>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QDir>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QStandardPaths>
#include <QNetworkProxy>
#ifdef __USE_TMP_RAM__
  #include <QStorageInfo>
#endif

// Auto Posting: watched folders, files moved or deleted after the post (member functions of NgPost, split from NgPost.cpp)

void NgPost::_startMonitoring(const QString &folderPath)
{
    qDebug() << "Start Monitoring " << folderPath << " (delay scan: " << FoldersMonitorForNewFiles::sMSleep << "ms)";
    _monitorThread = new QThread();
    _monitorThread->setObjectName("Monitoring");
    _folderMonitor = new FoldersMonitorForNewFiles(folderPath);
    _folderMonitor->moveToThread(_monitorThread);
    connect(_folderMonitor, &FoldersMonitorForNewFiles::newFileToProcess, this, &NgPost::onNewFileToProcess, Qt::QueuedConnection);
    _monitorThread->start();
}

void NgPost::_stopMonitoring()
{
    if (_folderMonitor)
    {
        _folderMonitor->stopListening();
        _monitorThread->quit();
        _monitorThread->wait();
        delete _folderMonitor;
        delete _monitorThread;
        _folderMonitor = nullptr;
        _monitorThread = nullptr;
    }
}

void NgPost::onNewFileToProcess(const QFileInfo & fileInfo)
{
    if (isPostedFolder(fileInfo))
    {
        if (_debug)
            _log(tr("Ignoring the Posted folder %1").arg(fileInfo.absoluteFilePath()));
        return;
    }
    if (fileInfo.isDir())
    {
        if (_monitorIgnoreDir)
        {
            if (_debug)
                _log(tr("MONITOR_IGNORE_DIR ON => Ignoring new incoming folder %1").arg(fileInfo.absoluteFilePath()));
            return;
        }
    }
    else if (!_monitorExtensions.isEmpty() && !_monitorExtensions.contains(fileInfo.suffix()))
    {
#ifdef __DEBUG__
        qDebug() << "MONITOR_EXTENSIONS ON => Ignoring new incoming file: " << fileInfo.absoluteFilePath()
                 << ", _monitorExtensions: " << _monitorExtensions
                 << ", size: " << _monitorExtensions.size()
                 << ", suffix: " << fileInfo.suffix();
#endif
        if (_debug)
            _log(tr("MONITOR_EXTENSIONS ON => Ignoring new incoming file %1").arg(fileInfo.absoluteFilePath()));
        return;
    }

    if (_monitorPaused)
    {
        for (const QFileInfo &waiting : _pausedFiles)
            if (waiting.absoluteFilePath() == fileInfo.absoluteFilePath())
                return;
        _pausedFiles << fileInfo;
        _log(tr("Watching is paused: %1 waits until it is resumed").arg(fileInfo.absoluteFilePath()));
        return;
    }

#ifdef __USE_HMI__
    if (_hmi)
    {
        _hmi->updateAutoPostingParams();
        _hmi->setJobLabel(QString());
        _delAuto = _hmi->autoWidget()->deleteFilesOncePosted();
        _movePosted = _hmi->autoWidget()->movePostedTo();
        _hmi->autoWidget()->newFileToProcess(fileInfo);
    }
#endif
    _log(tr("Processing new incoming file: %1").arg(fileInfo.absoluteFilePath()));
    _post(fileInfo, _monitor_nzb_folders ? QDir(fileInfo.absolutePath()).dirName() : QString());
}

void NgPost::closeAllMonitoringJobs()
{
    auto it = _pendingJobs.begin();
    while (it != _pendingJobs.end())
    {
        PostingJob *job = *it;
        if (!job->fromGuiForm())
        {
            it = _pendingJobs.erase(it);
            if (_debug)
                _error(tr("Cancelling monitoring job: %1").arg(job->getFirstOriginalFile()));
            if (job == _packingJob)
                _packingJob = nullptr;
            delete  job;
        }
        else
            ++it;
    }
    if (_activeJob && !_activeJob->fromGuiForm())
    {
        emit _activeJob->stopPosting();
        if (_debug)
            _error(tr("Stopping monitoring job: %1").arg(_activeJob->getFirstOriginalFile()));

    }
    _queueChanged();
}

void NgPost::pauseMonitoring()
{
    if (!_folderMonitor || _monitorPaused)
        return;
    _monitorPaused = true;
    _log(tr("Watching paused: the new files wait, the posts already started go on"));
}

void NgPost::resumeMonitoring()
{
    if (!_monitorPaused)
        return;
    _monitorPaused = false;
    _log(tr("Watching resumed"));
    const QList<QFileInfo> waiting = _pausedFiles;
    _pausedFiles.clear();
    for (const QFileInfo &fi : waiting)
        if (fi.exists())
            onNewFileToProcess(fi);
}

void NgPost::clearPausedMonitoring()
{
    _monitorPaused = false;
    _pausedFiles.clear();
}

int NgPost::nbMonitoringJobs() const
{
    int nb = 0;
    if (_activeJob && !_activeJob->fromGuiForm())
        ++nb;
    for (PostingJob *job : _pendingJobs)
        if (!job->fromGuiForm())
            ++nb;
    return nb;
}

void NgPost::watchStatsToday(int &ok, int &withErrors)
{
    if (_watchDay != QDate::currentDate())
    {
        _watchDay = QDate::currentDate();
        _watchOk = _watchErrors = 0;
    }
    ok = _watchOk;
    withErrors = _watchErrors;
}

bool NgPost::hasMonitoringPostingJobs() const
{
    if (_activeJob)
    {
        if (!_activeJob->fromGuiForm())
            return true;
    }
    else if (_pendingJobs.size())
    {
        for (PostingJob *job : _pendingJobs)
        {
            if (!job->fromGuiForm())
                return true;
        }
    }
    return false;
}

bool NgPost::isPostedFolder(const QFileInfo &fi) const
{
    return PostedMover::isPostedFolder(_movePosted, fi);
}

void NgPost::setMovePosted(const QString &folder)
{
    _movePosted = folder;
    if (_activeJob)
        _activeJob->setMovePostedTo(folder);
    for (PostingJob *job : _pendingJobs)
        job->setMovePostedTo(folder);
}

void NgPost::setDelFilesAfterPosted(bool delFiles)
{
    // Thread safe, only main thread is using this (NgPost or HMI)
    if (_activeJob)
        _activeJob->setDelFilesAfterPosted(delFiles);
    for (PostingJob *job : _pendingJobs)
        job->setDelFilesAfterPosted(delFiles);
}

void NgPost::addMonitoringFolder(const QString &dirPath)
{
    if (_folderMonitor)
        _folderMonitor->addFolder(dirPath);
}
