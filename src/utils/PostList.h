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

#ifndef POSTLIST_H
#define POSTLIST_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>
#include <QVector>

//! state of one post, as the user sees it (it doesn't depend on the classes that do the posting)
struct PostEntry
{
    enum class State {WAITING, COMPRESSING, PAR2, UPLOADING, PAUSED, DONE, DONE_WITH_ERRORS, STOPPED, CANCELLED};
    //! level of a line of the log (same values as NgPost::LOG_LEVEL, plus the errors)
    enum LogLevel {LOG_INFO = 0, LOG_PHASE, LOG_OK, LOG_WARN, LOG_DEBUG, LOG_ERROR};

    struct LogLine
    {
        QDateTime time;
        int       level = LOG_INFO;
        QString   text;
    };

    int         id = -1;
    QString     name;           //!< name of the post (nzb name, without the extension)
    QString     nzbPath;
    QString     archiveName;    //!< empty: no compression
    QString     password;       //!< archive password (empty: none)
    QString     groups;
    QStringList files;          //!< files and folders given to post
    QString     origin;         //!< quick, choose, scan, watch, guided, cli, queue
    bool        isRepost = false;
    int         priority = 1;   //!< QueueOrder::LOW, NORMAL or HIGH
    bool        paused   = false; //!< paused by the user (a waiting post is skipped, the running one stops uploading)
    int         queuePos = -1;  //!< place in the queue of the posts that wait (0: next), -1: not waiting
    int         prepared = 0;   //!< a waiting post compressed in advance (while the current one uploads): 1 being compressed, 2 ready

    State   state       = State::WAITING;
    quint64 sizeTotal   = 0;    //!< bytes to upload (0 until the files are packed)
    quint64 sizeDone    = 0;
    uint    nbArticles  = 0;
    uint    nbUploaded  = 0;    //!< Articles done (the failed ones included)
    uint    nbFailed    = 0;
    double  speed       = 0.;   //!< average upload speed in bytes per second
    qint64  etaSec      = -1;   //!< remaining time of the upload (-1: not known)
    qint64  durationMs  = 0;    //!< duration of the post (without the pauses)

    QDateTime added;
    QDateTime started;
    QDateTime finished;

    QVector<LogLine> log;
    bool             logLineOpen = false; //!< the last line is not finished (text written without a new line)
    int              logDropped  = 0;     //!< oldest lines removed to keep the log small

    bool isFinished() const;   //!< done, stopped or cancelled
    bool isActive() const;     //!< compressing, par2, uploading or paused
    int  progressPercent() const;
    QJsonObject toJson(bool withSecrets = false, bool withLog = false) const;

    static QString stateKey(State state); //!< "waiting", "uploading"... (not translated: for the API and the tests)
};

//! the posts of the session (waiting, running and finished), in the order they will be posted.
//! It is filled by NgPost: the GUI, the web panel and the API only read it and listen to its signals.
//! All its methods must be called from its thread (appendLog can be called from any thread).
class PostList : public QObject
{
    Q_OBJECT

public:
    static constexpr int sMaxLogLines = 3000; //!< lines kept for each post

    explicit PostList(QObject *parent = nullptr);

    int  add(const PostEntry &entry);                   //!< returns the id given to the new post
    bool update(const PostEntry &snapshot);             //!< state and progress of snapshot.id (the log is not touched)
    bool finish(int id, PostEntry::State state);        //!< DONE, DONE_WITH_ERRORS, STOPPED or CANCELLED
    void appendLog(int id, const QString &text, int level = PostEntry::LOG_INFO, bool newline = true);
    bool remove(int id);                                //!< only a finished post can be removed
    int  removeFinished();                              //!< returns the number of posts removed
    //! the posts that wait, in the order they will start: they go at the end of the list in this order
    //! (the other posts keep their place). Emits orderChanged when something moved.
    void setQueueOrder(const QVector<int> &waitingIds);

    const QVector<PostEntry> &entries() const { return _entries; }
    const PostEntry *entry(int id) const;               //!< valid until the list changes (nullptr: unknown id)
    int  indexOf(int id) const;
    int  count() const { return _entries.size(); }
    int  countActive() const;      //!< waiting or running
    QJsonObject toJson(bool withSecrets = false) const;

signals:
    void postAdded(int id);
    void postChanged(int id);
    void postRemoved(int id);
    void orderChanged();
    void logAdded(int id, const PostEntry::LogLine &line, bool continuesLine);

private:
    PostEntry *_entry(int id);

    QVector<PostEntry> _entries;
    int                _nextId = 1;
};

Q_DECLARE_METATYPE(PostEntry::LogLine)

#endif // POSTLIST_H
