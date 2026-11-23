//========================================================================
//
// ngPost Deluxe: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
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

#include "PostList.h"
#include <QJsonArray>
#include <QMetaObject>
#include <QThread>

bool PostEntry::isFinished() const
{
    return state == State::DONE || state == State::DONE_WITH_ERRORS
            || state == State::STOPPED || state == State::CANCELLED;
}

bool PostEntry::isActive() const
{
    return state == State::COMPRESSING || state == State::PAR2
            || state == State::UPLOADING || state == State::PAUSED;
}

int PostEntry::progressPercent() const
{
    if (state == State::DONE || state == State::DONE_WITH_ERRORS)
        return 100;
    if (nbArticles > 0) // same as the progress bar of the window
        return static_cast<int>(qMin<uint>(100, nbUploaded * 100 / nbArticles));
    if (sizeTotal > 0)
        return static_cast<int>(qMin<quint64>(100, sizeDone * 100 / sizeTotal));
    return 0;
}

QString PostEntry::stateKey(State state)
{
    switch (state)
    {
    case State::WAITING:          return "waiting";
    case State::COMPRESSING:      return "compressing";
    case State::PAR2:             return "par2";
    case State::UPLOADING:        return "uploading";
    case State::PAUSED:           return "paused";
    case State::DONE:             return "done";
    case State::DONE_WITH_ERRORS: return "done_with_errors";
    case State::STOPPED:          return "stopped";
    case State::CANCELLED:        return "cancelled";
    }
    return "unknown";
}

QJsonObject PostEntry::toJson(bool withSecrets, bool withLog) const
{
    QJsonObject obj;
    obj["id"]         = id;
    obj["name"]       = name;
    obj["state"]      = stateKey(state);
    obj["progress"]   = progressPercent();
    obj["sizeTotal"]  = static_cast<double>(sizeTotal);
    obj["sizeDone"]   = static_cast<double>(sizeDone);
    obj["articles"]   = static_cast<int>(nbArticles);
    obj["uploaded"]   = static_cast<int>(nbUploaded);
    obj["failed"]     = static_cast<int>(nbFailed);
    obj["speed"]      = speed;
    obj["eta"]        = static_cast<double>(etaSec);
    obj["durationMs"] = static_cast<double>(durationMs);
    obj["groups"]     = groups;
    obj["origin"]     = origin;
    obj["repost"]     = isRepost;
    obj["priority"]   = priority == 0 ? QString("low") : (priority == 2 ? QString("high") : QString("normal"));
    obj["paused"]     = paused;
    if (prepared)
        obj["prepared"] = prepared == 2 ? QString("ready") : QString("compressing");
    if (queuePos >= 0)
        obj["queuePos"] = queuePos;
    obj["nbFiles"]    = files.size();
    obj["compressed"] = !archiveName.isEmpty();
    obj["hasPassword"]= !password.isEmpty();
    if (added.isValid())    obj["added"]    = added.toString(Qt::ISODate);
    if (started.isValid())  obj["started"]  = started.toString(Qt::ISODate);
    if (finished.isValid()) obj["finished"] = finished.toString(Qt::ISODate);
    if (withSecrets)
    {   // file names, nzb, archive and password: only for the owner (never in the diagnostics)
        obj["nzb"]      = nzbPath;
        obj["archive"]  = archiveName;
        obj["password"] = password;
        obj["files"]    = QJsonArray::fromStringList(files);
    }
    if (withLog)
    {
        QJsonArray lines;
        for (const LogLine &l : log)
        {
            QJsonObject line;
            line["time"]  = l.time.toString(Qt::ISODate);
            line["level"] = l.level;
            line["text"]  = l.text;
            lines.append(line);
        }
        obj["log"] = lines;
        obj["logDropped"] = logDropped;
    }
    return obj;
}


PostList::PostList(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PostEntry::LogLine>("PostEntry::LogLine");
}

int PostList::add(const PostEntry &entry)
{
    PostEntry e = entry;
    e.id = _nextId++;
    if (!e.added.isValid())
        e.added = QDateTime::currentDateTime();
    e.log.clear();
    e.logLineOpen = false;
    e.logDropped  = 0;
    e.speedSamples.clear();
    e.sampleStepSec = 1;
    e.sampleMs = 0;
    e.sampleBytes = 0;
    _entries.append(e);
    emit postAdded(e.id);
    return e.id;
}

bool PostList::update(const PostEntry &s)
{
    PostEntry *e = _entry(s.id);
    if (!e || e->isFinished())
        return false;
    bool changed = e->state != s.state || e->sizeTotal != s.sizeTotal || e->sizeDone != s.sizeDone
            || e->nbArticles != s.nbArticles || e->nbUploaded != s.nbUploaded || e->nbFailed != s.nbFailed
            || e->etaSec != s.etaSec || qAbs(e->speed - s.speed) > 0.5
            || e->priority != s.priority || e->paused != s.paused || e->prepared != s.prepared
            || (!s.name.isEmpty() && e->name != s.name) || (!s.nzbPath.isEmpty() && e->nzbPath != s.nzbPath)
            || (!s.archiveName.isEmpty() && e->archiveName != s.archiveName);
    if (e->state == PostEntry::State::WAITING && s.state != PostEntry::State::WAITING && !e->started.isValid())
        e->started = QDateTime::currentDateTime();
    if (_sampleSpeed(*e, s))
        changed = true;
    e->state      = s.state;
    e->sizeTotal  = s.sizeTotal;
    e->sizeDone   = s.sizeDone;
    e->nbArticles = s.nbArticles;
    e->nbUploaded = s.nbUploaded;
    e->nbFailed   = s.nbFailed;
    e->speed      = s.speed;
    e->etaSec     = s.etaSec;
    e->durationMs = s.durationMs;
    e->priority   = s.priority;
    e->paused     = s.paused;
    e->prepared   = s.state == PostEntry::State::WAITING ? s.prepared : 0;
    if (!s.name.isEmpty())        e->name        = s.name;
    if (!s.nzbPath.isEmpty())     e->nzbPath     = s.nzbPath;
    if (!s.archiveName.isEmpty()) e->archiveName = s.archiveName;
    if (changed)
        emit postChanged(e->id);
    return true;
}

bool PostList::_sampleSpeed(PostEntry &e, const PostEntry &s)
{
    if (s.state != PostEntry::State::UPLOADING)
    {   // pauses and other phases are not in the graph: the next sample starts again from zero
        e.sampleMs = 0;
        return false;
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!e.sampleMs || s.sizeDone < e.sampleBytes)
    {
        e.sampleMs    = now;
        e.sampleBytes = s.sizeDone;
        return false;
    }
    const qint64 dt = now - e.sampleMs;
    if (dt < 900)
        return false;
    e.speedSamples.append(static_cast<float>((s.sizeDone - e.sampleBytes) * 1000.0 / dt));
    e.sampleMs    = now;
    e.sampleBytes = s.sizeDone;
    if (e.speedSamples.size() > sMaxSpeedSamples)
    {   // a long post: the oldest part keeps its shape with a coarser step
        QVector<float> merged;
        merged.reserve(e.speedSamples.size() / 2 + 1);
        for (int i = 0; i + 1 < e.speedSamples.size(); i += 2)
            merged.append((e.speedSamples.at(i) + e.speedSamples.at(i + 1)) / 2);
        if (e.speedSamples.size() % 2)
            merged.append(e.speedSamples.last());
        e.speedSamples = merged;
        e.sampleStepSec *= 2;
    }
    return true;
}

bool PostList::finish(int id, PostEntry::State state)
{
    PostEntry *e = _entry(id);
    if (!e || e->isFinished())
        return false;
    if (state != PostEntry::State::DONE && state != PostEntry::State::DONE_WITH_ERRORS
            && state != PostEntry::State::STOPPED)
        state = PostEntry::State::CANCELLED;
    e->state    = state;
    e->etaSec   = -1;
    e->queuePos = -1;
    e->finished = QDateTime::currentDateTime();
    if (state == PostEntry::State::DONE || state == PostEntry::State::DONE_WITH_ERRORS)
    {
        if (e->sizeTotal)
            e->sizeDone = e->sizeTotal;
    }
    emit postChanged(id);
    return true;
}

void PostList::appendLog(int id, const QString &text, int level, bool newline)
{
    if (QThread::currentThread() != thread())
    {   // the connections run in their own threads
        QMetaObject::invokeMethod(this, [this, id, text, level, newline](){ appendLog(id, text, level, newline); },
                                  Qt::QueuedConnection);
        return;
    }
    PostEntry *e = _entry(id);
    if (!e)
        return;

    // a text can hold several lines; the last one stays open when there is no new line
    QStringList parts = text.split('\n');
    bool endsWithNewline = text.endsWith('\n');
    if (endsWithNewline)
        parts.removeLast();
    for (int i = 0; i < parts.size(); ++i)
    {
        QString part = parts.at(i);
        part.remove('\r');
        bool lastPart   = i == parts.size() - 1;
        bool closesLine = !lastPart || newline || endsWithNewline;
        bool continues  = e->logLineOpen && !e->log.isEmpty();
        if (continues)
        {
            PostEntry::LogLine &line = e->log.last();
            line.text += part;
            if (level == PostEntry::LOG_ERROR || level == PostEntry::LOG_WARN)
                line.level = level;
            emit logAdded(id, line, true);
        }
        else
        {
            if (part.trimmed().isEmpty() && closesLine)
                continue; // empty lines between the messages are not kept
            PostEntry::LogLine line;
            line.time  = QDateTime::currentDateTime();
            line.level = level;
            line.text  = part;
            e->log.append(line);
            if (e->log.size() > sMaxLogLines)
            {
                int extra = e->log.size() - sMaxLogLines;
                e->log.remove(0, extra);
                e->logDropped += extra;
            }
            emit logAdded(id, e->log.last(), false);
        }
        e->logLineOpen = !closesLine;
    }
}

bool PostList::remove(int id)
{
    int idx = indexOf(id);
    if (idx < 0 || !_entries.at(idx).isFinished())
        return false;
    _entries.remove(idx);
    emit postRemoved(id);
    return true;
}

void PostList::setQueueOrder(const QVector<int> &waitingIds)
{
    QVector<PostEntry> others, waiting;
    others.reserve(_entries.size());
    for (const PostEntry &e : _entries)
        if (!waitingIds.contains(e.id))
        {
            others.append(e);
            others.last().queuePos = -1;
        }
    for (int i = 0; i < waitingIds.size(); ++i)
    {
        int idx = indexOf(waitingIds.at(i));
        if (idx < 0)
            continue;
        waiting.append(_entries.at(idx));
        waiting.last().queuePos = i;
    }
    QVector<PostEntry> ordered = others + waiting;

    bool moved = ordered.size() != _entries.size();
    QVector<int> posChanged;
    for (int i = 0; i < ordered.size() && !moved; ++i)
        if (ordered.at(i).id != _entries.at(i).id)
            moved = true;
    for (const PostEntry &e : ordered)
    {
        int idx = indexOf(e.id);
        if (idx >= 0 && _entries.at(idx).queuePos != e.queuePos)
            posChanged << e.id;
    }
    _entries = ordered;
    if (moved)
        emit orderChanged();
    for (int id : posChanged)
        emit postChanged(id);
}

int PostList::removeFinished()
{
    QList<int> ids;
    for (const PostEntry &e : _entries)
        if (e.isFinished())
            ids << e.id;
    for (int id : ids)
        remove(id);
    return ids.size();
}

const PostEntry *PostList::entry(int id) const
{
    int idx = indexOf(id);
    return idx < 0 ? nullptr : &_entries.at(idx);
}

PostEntry *PostList::_entry(int id)
{
    int idx = indexOf(id);
    return idx < 0 ? nullptr : &_entries[idx];
}

int PostList::indexOf(int id) const
{
    for (int i = 0; i < _entries.size(); ++i)
        if (_entries.at(i).id == id)
            return i;
    return -1;
}

int PostList::countActive() const
{
    int nb = 0;
    for (const PostEntry &e : _entries)
        if (!e.isFinished())
            ++nb;
    return nb;
}

QJsonObject PostList::toJson(bool withSecrets) const
{
    QJsonArray posts;
    for (const PostEntry &e : _entries)
        posts.append(e.toJson(withSecrets, false));
    QJsonObject obj;
    obj["posts"]  = posts;
    obj["active"] = countActive();
    return obj;
}
