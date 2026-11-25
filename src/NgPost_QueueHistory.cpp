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

// Saved queue of the posts and history file (member functions of NgPost, split from NgPost.cpp)

QStringList NgPost::historyHeader()
{
    return {tr("date"), tr("nzb name"), tr("size"), tr("avg. speed"), tr("archive name"), tr("archive pass"),
            tr("groups"), tr("from"), tr("status"), tr("files"), tr("duration"), tr("nzb file"), tr("input size"),
            tr("servers"), tr("options"), tr("kind"), tr("bytes")};
}

QString NgPost::historyProfile(const PostingJob *job) const
{
    QStringList keys;
    if (job->hasCompressed())
    {
        keys << "compress";
        if (job->hasRarPass())
            keys << "password";
    }
    if (job->hasPar2())
        keys << QString("par2:%1").arg(job->par2Pct());
    if (job->hasCompressed() && job->keepsRar())
        keys << "keep";
    return keys.isEmpty() ? QString("none") : keys.join("+");
}

QString NgPost::historyKind(const PostingJob *job) const
{
#ifdef __USE_HMI__
    if (_hmi)
    {
        if (job->origin() == "quick")
            return "quick";
        return _autoMode == "scan" ? "scan" : _autoMode == "watch" ? "watch" : "choose";
    }
#endif
    Q_UNUSED(job)
    return "cli";
}

QString NgPost::queueFile() const
{
    if (_configPath.isEmpty())
        return QString();
    return QFileInfo(_configPath).absoluteDir().absoluteFilePath("ngPost_queue.json");
}

PostQueue *NgPost::postQueue()
{
    if (_postQueue)
        return _postQueue;
    if (_queueTried || !_queueEnabled)
        return nullptr;
    QString path = queueFile();
    if (path.isEmpty())
        return nullptr; // no config file: nowhere to save it (like the history)
    _queueTried = true;
    PostQueue *queue = new PostQueue();
    QString err;
    bool ok = queue->open(path, err);
    if (!err.isEmpty())
        _error(tr("Queue: %1").arg(err));
    if (!ok)
    {
        delete queue;
        return nullptr;
    }
    _postQueue = queue;
    return _postQueue;
}

void NgPost::queueSaveJob(PostingJob *job)
{
    if (job->queueId().isEmpty() || job->isRepost() || !_postQueue)
        return;
    _postQueue->put(job->toQueueEntry());
}

void NgPost::queueJobEnded(PostingJob *job)
{
    const QString id = job->queueId();
    if (id.isEmpty() || !_postQueue)
        return;

    if (job->isRepost())
    {
        // the entry is the "missing Articles" of an earlier post: it follows what is still missing in the nzb
        if (!job->isRepostApplied())
            return; // the repost didn't start (no connection...): nothing changed
        const RepostInfo &left = job->repostInfo();
        if (left.isEmpty())
            _postQueue->remove(id);
        else
        {
            QJsonObject entry;
            entry.insert("id", id);
            entry.insert("state", PostQueue::sStateMissing);
            entry.insert("added", QDateTime::currentDateTime().toString(Qt::ISODate));
            entry.insert("missing", PostQueue::repostToJson(left));
            _postQueue->put(entry);
        }
    }
    else if (_closingApp)
        return; // the application is closing: the post stays in the queue (and keeps its id)
    else if (job->hasPostFinished() && !job->repostInfo().isEmpty())
    {
        QJsonObject entry;
        entry.insert("id", id);
        entry.insert("state", PostQueue::sStateMissing);
        entry.insert("added", QDateTime::currentDateTime().toString(Qt::ISODate));
        entry.insert("missing", PostQueue::repostToJson(job->repostInfo()));
        _postQueue->put(entry);
    }
    else if (!job->hasPostFinished() && job->keepInQueue())
    {
        _log(tr("The post %1 stays in the queue: it can be posted again at the next start.").arg(job->nzbName()));
    }
    else
        _postQueue->remove(id); // posted, stopped by the user, or failed (compression error...)

    job->setQueueId(QString()); // handled only once
}

QString NgPost::queueProblemMissing(const RepostInfo &info)
{
    if (info.isEmpty())
        return tr("nothing to post");
    if (!QFileInfo::exists(info.nzbPath))
        return tr("the nzb file is not there anymore: %1").arg(info.nzbPath);
    for (const RepostFile &file : info.files)
    {
        QFileInfo fi(file.path);
        if (!fi.exists())
            return tr("the file is not there anymore: %1").arg(file.path);
        if (fi.size() != file.size || (file.mtimeMs && fi.lastModified().toMSecsSinceEpoch() != file.mtimeMs))
            return tr("the file has been modified since the post: %1").arg(file.path);
    }
    return QString();
}

void NgPost::queueForget(const QString &id)
{
    if (!id.isEmpty() && _postQueue)
        _postQueue->remove(id);
}

void NgPost::queueSetEnabled(bool enabled)
{
    _queueEnabled = enabled;
    if (enabled)
    {
        _queueTried = false; // opened again by the next post
        return;
    }
    if (_postQueue)
        _postQueue->clear(); // removes the file
    delete _postQueue;
    _postQueue = nullptr;
}

int NgPost::resumeQueue()
{
    PostQueue *queue = postQueue();
    if (!queue || queue->size() == 0)
        return 0;

    int nbJobs = 0;
    const QList<QJsonObject> entries = queue->entries(); // copy: entries are removed below
    _log(tr("Queue: %1 unfinished post(s) to post again").arg(entries.size()));
    for (const QJsonObject &entry : entries)
    {
        const QString id    = entry.value("id").toString();
        const QString state = entry.value("state").toString();
        if (state == PostQueue::sStateMissing)
        {
            RepostInfo info = PostQueue::repostFromJson(entry.value("missing").toObject());
            info.queueId = id;
            QString problem = queueProblemMissing(info);
            if (!problem.isEmpty())
            {
                _error(tr("Queue: can't post again the missing Articles of %1: %2").arg(
                           QFileInfo(info.nzbPath).fileName(), problem));
                queue->remove(id);
                continue;
            }
            _log(tr("Queue: posting again the %1 missing Articles of %2").arg(info.nbArticles()).arg(
                     QFileInfo(info.nzbPath).fileName()));
            startPostingJob(PostingJob::createRepostJob(this, info, nullptr), true);
            ++nbJobs;
        }
        else
        {
            QString error;
            PostingJob *job = PostingJob::fromQueueEntry(this, entry, nullptr, error);
            if (!job)
            {
                _error(tr("Queue: can't post again %1: %2").arg(
                           QFileInfo(entry.value("nzb").toString()).fileName(), error));
                queue->remove(id);
                continue;
            }
            _log(tr("Queue: posting again from the beginning %1").arg(job->nzbName()));
            job->setHeld(false); // command line: nobody can resume a paused post, it is posted
            startPostingJob(job, true); // same order as before (whatever the priorities)
            ++nbJobs;
        }
    }
    return nbJobs;
}

qint64 NgPost::inputSize(const QFileInfoList &files)
{
    qint64 size = 0;
    for (const QFileInfo &fi : files)
    {
        if (fi.isDir())
        {
            QDirIterator it(fi.absoluteFilePath(), QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                it.next();
                size += it.fileInfo().size();
            }
        }
        else
            size += fi.size();
    }
    return size;
}

QString NgPost::duplicateInPost(const QString &nzbName, const QFileInfoList &files) const
{
    QString path = historyFile();
    if (!_checkDuplicates || path.isEmpty())
        return QString();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return QString();

    const qint64 size = inputSize(files);
    QString found;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    while (!stream.atEnd())
    {
        QStringList f = stream.readLine().split(_historyFieldSeparator);
        // only the posts that are finished without errors (the lines written by the original ngPost have no status)
        if (f.size() < 9 || f.value(1).trimmed() != nzbName || !(f.value(8).trimmed() == "OK" || f.value(8).trimmed().isEmpty()))
            continue;
        if (f.size() > 12 && !f.value(12).trimmed().isEmpty())
        {
            if (f.value(12).trimmed().toLongLong() != size)
                continue;
        }
        else if (f.size() > 9 && !f.value(9).trimmed().isEmpty() && f.value(9).trimmed().toInt() != files.size())
            continue; // lines written before ngPost Deluxe 1.3: the number of files is the only other clue
        found = f.value(0).trimmed();
    }
    return found;
}

QString NgPost::historyFile() const
{
    if (!_historyEnabled)
        return QString();
    if (!_postHistoryFile.isEmpty())
        return _postHistoryFile;
    if (_configPath.isEmpty())
        return QString();
    return QFileInfo(_configPath).absoluteDir().absoluteFilePath("ngPost_history.csv");
}
