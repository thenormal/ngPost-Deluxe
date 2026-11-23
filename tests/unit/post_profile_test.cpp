// PostProfile and PostTargets: lines of the config file (old and new profiles), name templates
#include "PostProfile.h"
#include <QCoreApplication>
#include <QDateTime>
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // a profile of an older version: no targets, written back the same
    PostProfile old;
    CHECK(PostProfile::fromLine("Film|1|1|0|1|5|1", old));
    CHECK(old.name == "Film" && old.compress && old.genName && !old.genPass && old.par2 && old.par2Pct == 5 && old.keepRar);
    CHECK(old.targets.isEmpty());
    CHECK(old.toLine() == "Film|1|1|0|1|5|1");

    // a complete profile: the special characters don't break the line of the config file
    PostProfile p;
    p.name = "Series";
    p.targets.groups       = QStringList{"alt.binaries.test", "alt.binaries.misc"};
    p.targets.servers      = QStringList{"me@news.example.com:563", "news.other.net:119"};
    p.targets.nzbDir       = "C:/NZB = files|new #1, 100%";
    p.targets.nameTemplate = "{name} [{date}] #{n}";
    const QString line = p.toLine();
    CHECK(!line.contains('=') && !line.contains('\n') && !line.contains('#'));
    CHECK(line.startsWith("Series|1|1|1|1|10|0|"));
    CHECK(line.split('|').size() == 11);
    PostProfile back;
    CHECK(PostProfile::fromLine(line, back));
    CHECK(back.name == "Series" && back.targets == p.targets);
    CHECK(back.toLine() == line);

    // only some parts
    PostProfile some;
    some.name = "Groups only";
    some.targets.groups = QStringList{"alt.binaries.test"};
    PostProfile someBack;
    CHECK(PostProfile::fromLine(some.toLine(), someBack));
    CHECK(someBack.targets.groups == QStringList{"alt.binaries.test"} && someBack.targets.servers.isEmpty()
          && someBack.targets.nzbDir.isEmpty() && someBack.targets.nameTemplate.isEmpty());

    // targets of the last session (quick_targets): own line, upper case groups are made lower case
    PostTargets t = PostTargets::fromLine(p.targets.toLine());
    CHECK(t == p.targets);
    CHECK(PostTargets::fromLine("ALT.Binaries.Test||").groups == QStringList{"alt.binaries.test"});
    CHECK(PostTargets::fromLine("").isEmpty());

    // the options of the last session keep the targets
    PostProfile session;
    session.targets.groups = QStringList{"alt.binaries.test"};
    CHECK(PostProfile::fromOptionsLine("0|0|0|1|20|0", session));
    CHECK(!session.compress && session.par2 && session.par2Pct == 20 && session.targets.groups.size() == 1);
    CHECK(!PostProfile::fromOptionsLine("bad", session));

    // name templates
    const QDateTime when(QDate(2026, 11, 23), QTime(15, 4));
    CHECK(PostTargets::applyTemplate("", "Movie", when, 3) == "Movie");
    CHECK(PostTargets::applyTemplate("{name} [{date}]", "Movie", when, 3) == "Movie [2026-11-23]");
    CHECK(PostTargets::applyTemplate("{NAME}_{time}_{n}", "Movie", when, 7) == "Movie_15-04_7");
    CHECK(PostTargets::applyTemplate("Show {n}", "Movie", when, 12) == "Show 12");
    CHECK(PostTargets::applyTemplate("{name}: a/b?*\"<>|", "Movie", when, 1) == "Movie ab");
    CHECK(PostTargets::applyTemplate("{name}...", "Movie", when, 1) == "Movie");
    CHECK(PostTargets::applyTemplate("///", "Movie", when, 1) == "Movie"); // nothing left: the name
    CHECK(PostTargets::usesCounter("Ep {N}") && !PostTargets::usesCounter("{name}"));

    if (failures)
        std::printf("%d check(s) failed\n", failures);
    else
        std::printf("all checks passed\n");
    return failures ? 1 : 0;
}
