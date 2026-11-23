// QueueOrder: order of the posts that wait (priority, moves, drag and drop, pause, the post being prepared)
// and PostList::setQueueOrder (the page Posts shows the waiting posts in that order)
#include "QueueOrder.h"
#include "PostList.h"
#include <QCoreApplication>
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

using namespace QueueOrder;

static QueueSlot slot(int priority, bool paused = false, bool locked = false)
{
    QueueSlot s;
    s.priority = priority;
    s.paused   = paused;
    s.locked   = locked;
    return s;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // priorities: keys, unknown values
    CHECK(priorityKey(HIGH) == "high" && priorityKey(NORMAL) == "normal" && priorityKey(LOW) == "low");
    CHECK(priorityFromKey("high") == HIGH && priorityFromKey("LOW") == LOW && priorityFromKey("???") == NORMAL);
    CHECK(clampPriority(7) == HIGH && clampPriority(-3) == LOW);

    // a new post goes after the posts of the same or a higher priority
    QVector<QueueSlot> q;
    CHECK(insertIndex(q, NORMAL) == 0);
    q = {slot(HIGH), slot(NORMAL), slot(NORMAL), slot(LOW)};
    CHECK(insertIndex(q, NORMAL) == 3);
    CHECK(insertIndex(q, HIGH) == 1);
    CHECK(insertIndex(q, LOW) == 4);
    // ... even when the user moved a low post on top: the order of the user stays
    q = {slot(LOW), slot(HIGH), slot(NORMAL)};
    CHECK(insertIndex(q, HIGH) == 0);
    CHECK(insertIndex(q, NORMAL) == 0);
    // nothing goes before the post being prepared
    q = {slot(LOW, false, true), slot(NORMAL)};
    CHECK(nbLocked(q) == 1);
    CHECK(insertIndex(q, HIGH) == 1);

    // the next post: the first one that is not paused, the post being prepared first
    q = {slot(NORMAL, true), slot(NORMAL, true), slot(LOW)};
    CHECK(nextIndex(q) == 2);
    q = {slot(NORMAL, true)};
    CHECK(nextIndex(q) == -1);
    CHECK(nextIndex({}) == -1);
    q = {slot(LOW, false, true), slot(HIGH)};
    CHECK(nextIndex(q) == 0);

    // moves
    q = {slot(NORMAL), slot(NORMAL), slot(NORMAL), slot(NORMAL)};
    CHECK(moveIndex(q, 2, Move::UP) == 1);
    CHECK(moveIndex(q, 2, Move::DOWN) == 3);
    CHECK(moveIndex(q, 2, Move::TOP) == 0);
    CHECK(moveIndex(q, 2, Move::BOTTOM) == 3);
    CHECK(moveIndex(q, 0, Move::UP) == -1);
    CHECK(moveIndex(q, 0, Move::TOP) == -1);
    CHECK(moveIndex(q, 3, Move::DOWN) == -1);
    CHECK(moveIndex(q, 3, Move::BOTTOM) == -1);
    CHECK(moveIndex(q, 9, Move::UP) == -1);
    // the post being prepared doesn't move and nothing goes above it
    q = {slot(NORMAL, false, true), slot(NORMAL), slot(NORMAL)};
    CHECK(moveIndex(q, 0, Move::DOWN) == -1);
    CHECK(moveIndex(q, 1, Move::UP) == -1);
    CHECK(moveIndex(q, 2, Move::TOP) == 1);
    // a paused post can be moved
    q = {slot(NORMAL), slot(NORMAL, true)};
    CHECK(moveIndex(q, 1, Move::UP) == 0);

    // drag and drop: "before" is the index in the queue as it is now
    q = {slot(NORMAL), slot(NORMAL), slot(NORMAL), slot(NORMAL)};
    CHECK(dropIndex(q, 3, 0) == 0);
    CHECK(dropIndex(q, 0, 4) == 3);  // at the end
    CHECK(dropIndex(q, 0, 2) == 1);  // before the third one
    CHECK(dropIndex(q, 1, 1) == -1); // dropped on itself
    CHECK(dropIndex(q, 1, 2) == -1); // right after itself: same place
    CHECK(dropIndex(q, 1, 99) == 3);
    q = {slot(NORMAL, false, true), slot(NORMAL), slot(NORMAL)};
    CHECK(dropIndex(q, 2, 0) == 1);  // can't go before the post being prepared
    CHECK(dropIndex(q, 0, 3) == -1); // the post being prepared can't be dragged

    // a new priority: the post goes among the posts of its new priority
    q = {slot(HIGH), slot(NORMAL), slot(NORMAL), slot(LOW)};
    CHECK(reprioritizeIndex(q, 3, HIGH) == 1);
    CHECK(reprioritizeIndex(q, 0, LOW) == 3);
    CHECK(reprioritizeIndex(q, 1, NORMAL) == 2); // same priority: after the other normal posts

    // the page Posts: the waiting posts at the end, in the order of the queue
    PostList list;
    int orderChanged = 0;
    QObject::connect(&list, &PostList::orderChanged, [&](){ ++orderChanged; });
    PostEntry a; a.name = "a";
    PostEntry b; b.name = "b";
    PostEntry c; c.name = "c";
    PostEntry d; d.name = "d";
    int idA = list.add(a), idB = list.add(b), idC = list.add(c), idD = list.add(d);
    PostEntry s; s.id = idA; s.state = PostEntry::State::UPLOADING;
    list.update(s); // a is posted, b c d wait
    list.setQueueOrder({idD, idB, idC});
    CHECK(orderChanged == 1);
    CHECK(list.indexOf(idA) == 0 && list.indexOf(idD) == 1 && list.indexOf(idB) == 2 && list.indexOf(idC) == 3);
    CHECK(list.entry(idA)->queuePos == -1 && list.entry(idD)->queuePos == 0 && list.entry(idC)->queuePos == 2);
    list.setQueueOrder({idD, idB, idC});
    CHECK(orderChanged == 1); // nothing moved
    // d starts: it leaves the queue and stays after a
    list.setQueueOrder({idB, idC});
    CHECK(orderChanged == 1 && list.indexOf(idD) == 1 && list.entry(idD)->queuePos == -1);
    CHECK(list.entry(idB)->queuePos == 0);
    // priority and pause are part of the state of a post
    PostEntry p; p.id = idC; p.state = PostEntry::State::WAITING; p.priority = HIGH; p.paused = true;
    CHECK(list.update(p));
    CHECK(list.entry(idC)->priority == HIGH && list.entry(idC)->paused);
    QJsonObject json = list.entry(idC)->toJson();
    CHECK(json.value("priority").toString() == "high" && json.value("paused").toBool());
    CHECK(json.value("queuePos").toInt() == 1);
    // finished: no place in the queue anymore
    list.finish(idC, PostEntry::State::CANCELLED);
    CHECK(list.entry(idC)->queuePos == -1);

    if (failures)
        std::printf("%d failure(s)\n", failures);
    else
        std::printf("queue_order_test: all good\n");
    return failures ? 1 : 0;
}
