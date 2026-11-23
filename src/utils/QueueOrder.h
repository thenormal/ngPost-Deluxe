//========================================================================
//
// ngPost Deluxe: order of the posts that wait (priority, moves, pause)
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
#ifndef QUEUEORDER_H
#define QUEUEORDER_H

#include <QVector>
#include <QString>

//! what the rules need to know about one post that waits
struct QueueSlot
{
    int  priority = 1;      //!< QueueOrder::LOW, NORMAL or HIGH
    bool paused   = false;  //!< kept in the queue: the next posts go before it
    bool locked   = false;  //!< already being prepared (compressed) for the next start: it stays first
};

/*!
 * \brief The rules of the queue, without the posts themselves (so they can be tested alone).
 * The queue is the list of the posts that wait, in the order they start:
 *  - a new post goes after the posts with the same or a higher priority (before the lower ones);
 *  - the user can move a post: it stays where it is put, whatever its priority;
 *  - the next post is the first one that is not paused;
 *  - a locked post (the next post, compressed while the current one is uploading) stays first:
 *    it can't be moved or paused and nothing can go before it.
 */
namespace QueueOrder
{
enum Priority {LOW = 0, NORMAL = 1, HIGH = 2};
enum class Move {UP, DOWN, TOP, BOTTOM};

int  clampPriority(int priority);
QString priorityKey(int priority);          //!< "low", "normal", "high" (not translated: json and tests)
int  priorityFromKey(const QString &key);   //!< NORMAL if unknown

int  nbLocked(const QVector<QueueSlot> &queue);                    //!< locked posts at the head
int  insertIndex(const QVector<QueueSlot> &queue, int priority);   //!< where a new post goes
int  nextIndex(const QVector<QueueSlot> &queue);                   //!< the post to start (-1: none)
//! index where the post goes when it is moved (-1: it can't move that way)
int  moveIndex(const QVector<QueueSlot> &queue, int from, Move move);
//! drag and drop: the post at \p from is put before the post at \p before (queue.size(): at the end).
//! Returns its new index (-1: it can't be moved there)
int  dropIndex(const QVector<QueueSlot> &queue, int from, int before);
//! a post whose priority changes goes back among the posts of its new priority (index after it is taken out)
int  reprioritizeIndex(const QVector<QueueSlot> &queue, int from, int priority);
}

#endif // QUEUEORDER_H
