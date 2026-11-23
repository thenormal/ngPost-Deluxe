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
#include "QueueOrder.h"

namespace QueueOrder
{

int clampPriority(int priority)
{
    return priority < LOW ? LOW : (priority > HIGH ? HIGH : priority);
}

QString priorityKey(int priority)
{
    switch (clampPriority(priority))
    {
    case LOW:  return "low";
    case HIGH: return "high";
    default:   return "normal";
    }
}

int priorityFromKey(const QString &key)
{
    const QString k = key.trimmed().toLower();
    if (k == "low" || k == "0")
        return LOW;
    if (k == "high" || k == "2")
        return HIGH;
    return NORMAL;
}

int nbLocked(const QVector<QueueSlot> &queue)
{
    int nb = 0;
    while (nb < queue.size() && queue.at(nb).locked)
        ++nb;
    return nb;
}

int insertIndex(const QVector<QueueSlot> &queue, int priority)
{
    priority = clampPriority(priority);
    for (int i = nbLocked(queue); i < queue.size(); ++i)
        if (queue.at(i).priority < priority)
            return i;
    return queue.size();
}

int nextIndex(const QVector<QueueSlot> &queue)
{
    if (nbLocked(queue) > 0)
        return 0;
    for (int i = 0; i < queue.size(); ++i)
        if (!queue.at(i).paused)
            return i;
    return -1;
}

int moveIndex(const QVector<QueueSlot> &queue, int from, Move move)
{
    if (from < 0 || from >= queue.size() || queue.at(from).locked)
        return -1;
    const int first = nbLocked(queue), last = queue.size() - 1;
    int to = from;
    switch (move)
    {
    case Move::UP:     to = from - 1; break;
    case Move::DOWN:   to = from + 1; break;
    case Move::TOP:    to = first;    break;
    case Move::BOTTOM: to = last;     break;
    }
    if (to < first || to > last || to == from)
        return -1;
    return to;
}

int dropIndex(const QVector<QueueSlot> &queue, int from, int before)
{
    if (from < 0 || from >= queue.size() || queue.at(from).locked)
        return -1;
    const int first = nbLocked(queue);
    if (before < first)
        before = first;
    if (before > queue.size())
        before = queue.size();
    int to = before > from ? before - 1 : before; // index once the post is taken out
    if (to == from)
        return -1;
    return to;
}

int reprioritizeIndex(const QVector<QueueSlot> &queue, int from, int priority)
{
    if (from < 0 || from >= queue.size() || queue.at(from).locked)
        return from;
    QVector<QueueSlot> others = queue;
    others.remove(from);
    return insertIndex(others, priority);
}

} // namespace QueueOrder
