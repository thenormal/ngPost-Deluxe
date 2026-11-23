// PostList: the posts of the session (states, progress, log of each post, removal of the finished ones)
#include "PostList.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QThread>
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

using State = PostEntry::State;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    PostList list;
    int added = 0, changed = 0, removed = 0, logs = 0;
    QObject::connect(&list, &PostList::postAdded,   [&](int){ ++added; });
    QObject::connect(&list, &PostList::postChanged, [&](int){ ++changed; });
    QObject::connect(&list, &PostList::postRemoved, [&](int){ ++removed; });
    QObject::connect(&list, &PostList::logAdded,    [&](int, const PostEntry::LogLine &, bool){ ++logs; });

    // ids are unique and kept in the order of the posts
    PostEntry a; a.name = "first";  a.password = "secret"; a.files << "/data/a.mkv";
    PostEntry b; b.name = "second";
    int idA = list.add(a), idB = list.add(b);
    CHECK(idA > 0 && idB > idA);
    CHECK(added == 2 && list.count() == 2 && list.countActive() == 2);
    CHECK(list.indexOf(idA) == 0 && list.indexOf(idB) == 1);
    CHECK(list.entry(idA)->state == State::WAITING && list.entry(idA)->added.isValid());
    CHECK(!list.entry(idA)->started.isValid());
    CHECK(list.entry(12345) == nullptr);

    // progress: a change is signalled, the same values are not
    PostEntry s; s.id = idA; s.state = State::UPLOADING; s.sizeTotal = 1000; s.sizeDone = 250;
    s.nbArticles = 4; s.nbUploaded = 1; s.speed = 100.;
    CHECK(list.update(s));
    CHECK(changed == 1);
    CHECK(list.entry(idA)->started.isValid());
    CHECK(list.entry(idA)->progressPercent() == 25 && list.entry(idA)->isActive());
    CHECK(list.update(s) && changed == 1);
    s.state = State::PAUSED;
    CHECK(list.update(s) && changed == 2 && list.entry(idA)->isActive());
    CHECK(list.entry(idA)->name == "first"); // an empty name in the snapshot doesn't erase it

    // log: lines, text without a new line continues the last line, empty lines are dropped
    list.appendLog(idA, "Compressing files...");
    list.appendLog(idA, "*", PostEntry::LOG_INFO, false);
    list.appendLog(idA, "*", PostEntry::LOG_INFO, false);
    list.appendLog(idA, " done");
    list.appendLog(idA, "two\nlines\n");
    list.appendLog(idA, "");
    list.appendLog(idA, "broken", PostEntry::LOG_ERROR);
    const PostEntry *ea = list.entry(idA); // (pointer valid until the next post is added)
    CHECK(ea->log.size() == 5);
    CHECK(ea->log.at(0).text == "Compressing files...");
    CHECK(ea->log.at(1).text == "** done");
    CHECK(ea->log.at(2).text == "two" && ea->log.at(3).text == "lines");
    CHECK(ea->log.last().level == PostEntry::LOG_ERROR && ea->log.last().text == "broken");
    CHECK(list.entry(idB)->log.isEmpty()); // each post has its own log
    list.appendLog(9999, "nobody");        // unknown post: ignored
    CHECK(logs >= 6);

    // log written from another thread arrives in the right order
    {
        QThread th;
        QObject worker;
        worker.moveToThread(&th);
        th.start();
        QMetaObject::invokeMethod(&worker, [&](){ for (int i = 0; i < 5; ++i) list.appendLog(idB, QString("t%1").arg(i)); },
                                  Qt::BlockingQueuedConnection);
        th.quit(); th.wait();
        app.processEvents();
        const PostEntry *eb = list.entry(idB);
        CHECK(eb->log.size() == 5 && eb->log.first().text == "t0" && eb->log.last().text == "t4");
    }

    // the log is kept small
    PostEntry big; big.name = "big";
    int idBig = list.add(big);
    for (int i = 0; i < PostList::sMaxLogLines + 50; ++i)
        list.appendLog(idBig, QString("line %1").arg(i));
    CHECK(list.entry(idBig)->log.size() == PostList::sMaxLogLines);
    CHECK(list.entry(idBig)->logDropped == 50);
    CHECK(list.entry(idBig)->log.first().text == "line 50");

    // end of the posts
    CHECK(!list.remove(idA));                        // a running post can't be removed
    CHECK(list.finish(idA, State::DONE_WITH_ERRORS));
    CHECK(list.entry(idA)->isFinished() && list.entry(idA)->finished.isValid());
    CHECK(list.entry(idA)->progressPercent() == 100 && list.entry(idA)->sizeDone == 1000);
    CHECK(!list.finish(idA, State::DONE));           // only once
    CHECK(!list.update(s));                          // a finished post doesn't move anymore
    CHECK(list.finish(idB, State::UPLOADING));       // not an end state: cancelled
    CHECK(list.entry(idB)->state == State::CANCELLED);
    CHECK(list.countActive() == 1);

    // JSON for the API: no secrets unless asked
    QJsonObject pub = list.entry(idA)->toJson();
    CHECK(pub.value("state").toString() == "done_with_errors");
    CHECK(pub.value("hasPassword").toBool() && !pub.contains("password") && !pub.contains("files"));
    QJsonObject priv = list.entry(idA)->toJson(true, true);
    CHECK(priv.value("password").toString() == "secret" && priv.value("files").toArray().size() == 1);
    CHECK(priv.value("log").toArray().size() == 5);
    CHECK(list.toJson().value("posts").toArray().size() == 3 && list.toJson().value("active").toInt() == 1);

    // removal of the finished posts
    CHECK(list.removeFinished() == 2 && removed == 2);
    CHECK(list.count() == 1 && list.entries().first().id == idBig);
    CHECK(list.remove(idBig) == false);

    if (failures)
        std::printf("%d check(s) failed\n", failures);
    else
        std::printf("all checks passed\n");
    return failures ? 1 : 0;
}
