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

// List of the posts (page Posts): add, refresh, stop, priority, order, pause (member functions of NgPost, split from NgPost.cpp)

void NgPost::_postListAdd(PostingJob *job)
{
    if (!job || job->postId() >= 0)
        return;
    PostEntry e;
    e.name        = job->nzbName();
    e.nzbPath     = job->nzbFilePath();
    e.archiveName = job->hasCompressed() ? job->rarName() : QString();
    e.password    = job->rarPass();
    e.groups      = job->groups();
    for (const QFileInfo &fi : job->originalFiles())
        e.files << QDir::toNativeSeparators(fi.absoluteFilePath());
    e.isRepost    = job->isRepost();
    e.origin      = job->isRepost() ? QString("repost") : (job->_resumed ? QString("queue") : historyKind(job));
    e.priority    = job->priority();
    e.paused      = job->isHeld() || job->isPaused() || job->isPauseRequested();
    job->setPostId(_postList->add(e));

    connect(_postListTimer, &QTimer::timeout, this, &NgPost::_onPostListTimer, Qt::UniqueConnection);
    if (!_postListTimer->isActive())
        _postListTimer->start(1000);
}

void NgPost::_postListRefresh(PostingJob *job)
{
    if (!job || job->postId() < 0)
        return;
    PostEntry s;
    s.id = job->postId();
    switch (job->phase())
    {
    case PostingJob::Phase::WAITING:     s.state = PostEntry::State::WAITING;     break;
    case PostingJob::Phase::COMPRESSING: s.state = PostEntry::State::COMPRESSING; break;
    case PostingJob::Phase::PAR2:        s.state = PostEntry::State::PAR2;        break;
    case PostingJob::Phase::UPLOADING:
        s.state = job->isPaused() ? PostEntry::State::PAUSED : PostEntry::State::UPLOADING;
        break;
    case PostingJob::Phase::DONE:        s.state = PostEntry::State::UPLOADING;   break; // finish() gives the real end
    }
    if (job != _activeJob && !job->hasPostStarted())
    {   // still in the queue: the next post can be compressed in advance, it is not started for the user
        s.prepared = job->isPacked() ? 2 : (job == _packingJob && job->hasPacking() ? 1 : 0);
        s.state    = PostEntry::State::WAITING;
    }
    s.sizeTotal   = job->totalSizeBytes();
    s.sizeDone    = job->uploadedSizeBytes();
    s.nbArticles  = job->nbArticlesTotal();
    s.nbUploaded  = job->nbArticlesUploaded();
    s.nbFailed    = job->nbArticlesFailed();
    s.speed       = job->avgSpeedBytes();
    s.etaSec      = s.state == PostEntry::State::UPLOADING ? job->etaSeconds() : -1;
    s.durationMs  = job->jobDurationMs();
    s.priority    = job->priority();
    s.paused      = job->isHeld() || job->isPaused() || job->isPauseRequested();
    s.name        = job->nzbName();
    s.nzbPath     = job->nzbFilePath();
    if (job->hasCompressed())
        s.archiveName = job->rarName();
    _postList->update(s);
}

void NgPost::_postListFinish(PostingJob *job)
{
    if (!job || job->postId() < 0)
        return;
    _postListRefresh(job);
#ifdef __USE_HMI__
    // GUI: the page Posts offers to post the missing Articles again (known before the list says the post is over)
    if (_hmi && job->hasPostFinished() && !job->repostInfo().isEmpty())
        _missingOfPost.insert(job->postId(), job->repostInfo());
#endif
    PostEntry::State state = !job->hasPostFinished() ? PostEntry::State::STOPPED :
                             (job->nbArticlesFailed() ? PostEntry::State::DONE_WITH_ERRORS : PostEntry::State::DONE);
    _postList->finish(job->postId(), state);
    _dumpPostList();
}

void NgPost::postListJobGone(PostingJob *job)
{
    if (!job || job->postId() < 0)
        return;
    const PostEntry *e = _postList->entry(job->postId());
    if (!e || e->isFinished())
        return;
    bool started = job->hasPostStarted() || job->phase() != PostingJob::Phase::WAITING;
    _postList->finish(job->postId(), started ? PostEntry::State::STOPPED : PostEntry::State::CANCELLED);
    _dumpPostList();
}

void NgPost::_onPostListTimer()
{
    _postListRefresh(_activeJob);
    if (_packingJob && _packingJob != _activeJob)
        _postListRefresh(_packingJob);
    if (!_activeJob && !_packingJob)
        _postListTimer->stop();
}

void NgPost::_dumpPostList() const
{   // tests: NGPOST_POSTLIST_DUMP=<file> writes the list (with the log) each time a post ends
    QString path = qEnvironmentVariable("NGPOST_POSTLIST_DUMP");
    if (path.isEmpty())
        return;
    QJsonArray posts;
    for (const PostEntry &e : _postList->entries())
        posts.append(e.toJson(true, true));
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QJsonDocument(QJsonObject{{"posts", posts}}).toJson());
}

PostingJob *NgPost::_jobOfPost(int postId) const
{
    if (postId < 0)
        return nullptr;
    if (_activeJob && _activeJob->postId() == postId)
        return _activeJob;
    for (PostingJob *job : _pendingJobs)
        if (job->postId() == postId)
            return job;
    return nullptr;
}

bool NgPost::stopPost(int postId)
{
    PostingJob *job = _jobOfPost(postId);
    if (!job)
        return false;
    emit job->stopPosting(); // a waiting post is cancelled, a running one is stopped
    return true;
}

int NgPost::nbMissingArticles(int postId) const
{
    auto it = _missingOfPost.constFind(postId);
    return it == _missingOfPost.constEnd() ? 0 : it->nbArticles();
}

bool NgPost::repostMissing(int postId)
{
    auto it = _missingOfPost.find(postId);
    if (it == _missingOfPost.end())
        return false;
    RepostInfo info = it.value();
    QString problem = queueProblemMissing(info);
    if (!problem.isEmpty())
    {
        _error(tr("Can't post again the missing Articles of %1: %2").arg(QFileInfo(info.nzbPath).fileName(), problem));
        return false;
    }
    _missingOfPost.erase(it);
    PostingJob *job = PostingJob::createRepostJob(this, info, nullptr);
    if (!job)
        return false;
    job->setOrigin("quick");
    startPostingJob(job);
    return true;
}

void NgPost::forgetMissing(int postId)
{
    auto it = _missingOfPost.find(postId);
    if (it == _missingOfPost.end())
        return;
    queueForget(it->queueId); // they can't be posted again anymore
    _missingOfPost.erase(it);
}

bool NgPost::setPostPriority(int postId, int priority)
{
    PostingJob *job = _jobOfPost(postId);
    int idx = job ? _pendingJobs.indexOf(job) : -1;
    if (idx < 0)
        return false; // only a waiting post
    priority = QueueOrder::clampPriority(priority);
    if (job->priority() == priority)
        return true;
    int to = QueueOrder::reprioritizeIndex(_queueSlots(), idx, priority);
    job->setPriority(priority);
    if (to != idx)
    {
        _pendingJobs.removeAt(idx);
        _pendingJobs.insert(to, job);
    }
    _refreshPost(job);
    _queueChanged();
    return true;
}

bool NgPost::canMovePost(int postId, QueueOrder::Move move) const
{
    PostingJob *job = _jobOfPost(postId);
    int idx = job ? _pendingJobs.indexOf(job) : -1;
    return idx >= 0 && QueueOrder::moveIndex(_queueSlots(), idx, move) >= 0;
}

bool NgPost::movePost(int postId, QueueOrder::Move move)
{
    PostingJob *job = _jobOfPost(postId);
    int idx = job ? _pendingJobs.indexOf(job) : -1;
    if (idx < 0)
        return false;
    int to = QueueOrder::moveIndex(_queueSlots(), idx, move);
    if (to < 0)
        return false;
    _pendingJobs.removeAt(idx);
    _pendingJobs.insert(to, job);
    _queueChanged();
    return true;
}

bool NgPost::canDragPost(int postId) const
{
    PostingJob *job = _jobOfPost(postId);
    int idx = job ? _pendingJobs.indexOf(job) : -1;
    return idx >= 0 && !_queueSlots().at(idx).locked;
}

bool NgPost::movePostBefore(int postId, int beforePostId)
{
    PostingJob *job = _jobOfPost(postId);
    int idx = job ? _pendingJobs.indexOf(job) : -1;
    if (idx < 0)
        return false;
    PostingJob *before = beforePostId >= 0 ? _jobOfPost(beforePostId) : nullptr;
    int beforeIdx = before ? _pendingJobs.indexOf(before) : -1;
    if (beforeIdx < 0)
        beforeIdx = _pendingJobs.size();
    int to = QueueOrder::dropIndex(_queueSlots(), idx, beforeIdx);
    if (to < 0)
        return false;
    _pendingJobs.removeAt(idx);
    _pendingJobs.insert(to, job);
    _queueChanged();
    return true;
}

bool NgPost::isPostPrepared(int postId) const
{
    PostingJob *job = _jobOfPost(postId);
    return job && job == _packingJob && job->hasPacking();
}

bool NgPost::isPostPaused(int postId) const
{
    PostingJob *job = _jobOfPost(postId);
    if (!job)
        return false;
    if (job == _activeJob)
        return job->isPaused() || job->isPauseRequested();
    return job->isHeld();
}

bool NgPost::canPausePost(int postId) const
{
    PostingJob *job = _jobOfPost(postId);
    if (!job || isPostPaused(postId))
        return false;
    if (job == _activeJob)
        return true;
    return !isPostPrepared(postId);
}

bool NgPost::pausePost(int postId)
{
    if (!canPausePost(postId))
        return false;
    PostingJob *job = _jobOfPost(postId);
    if (job == _activeJob)
    {
        if (!job->hasPostStarted())
            _postList->appendLog(postId, tr("Paused: it stops when the upload starts"), PostEntry::LOG_WARN);
        pause(); // (the other posts wait: one post at a time)
        return true;
    }
    job->setHeld(true);
    _postList->appendLog(postId, tr("Paused: the next posts go before it"), PostEntry::LOG_WARN);
    _refreshPost(job);
    _queueChanged();
    return true;
}

bool NgPost::resumePost(int postId)
{
    PostingJob *job = _jobOfPost(postId);
    if (!job || !isPostPaused(postId))
        return false;
    if (job == _activeJob)
    {
        resume(); // (the post writes "Resume posting" in its log)
        return true;
    }
    _postList->appendLog(postId, tr("Resumed"), PostEntry::LOG_INFO);
    job->setHeld(false);
    _refreshPost(job);
    _queueChanged();
    if (!_activeJob)
        _startNextJob();
    return true;
}
