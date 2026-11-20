// PostRequest: what the user wants to post (Guided Post), its checks and the remembered choices
#include "PostRequest.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

int main()
{
    using Kind  = PostRequest::Kind;
    using After = PostRequest::After;

    // which page posts it
    PostRequest r;
    CHECK(!r.usesAutoPosting());
    r.kind = Kind::EachItem;    CHECK(r.usesAutoPosting());
    r.kind = Kind::ScanFolder;  CHECK(r.usesAutoPosting());
    r.kind = Kind::WatchFolder; CHECK(r.usesAutoPosting());

    // checks
    PostRequest one;
    QString why;
    CHECK(!one.isValid(&why) && !why.isEmpty());          // nothing to post
    one.paths << "  ";
    CHECK(!one.isValid());
    one.paths << "/data/file.mkv";
    CHECK(one.isValid());
    one.after = After::Delete;
    CHECK(!one.isValid());                                 // only Auto Posting moves or deletes
    PostRequest each;
    each.kind = Kind::EachItem;
    each.paths << "/data/a" << "/data/b";
    each.after = After::MoveToPosted;
    CHECK(each.isValid());
    PostRequest scan;
    scan.kind = Kind::ScanFolder;
    CHECK(!scan.isValid());                                // no folder
    scan.folder = "/data/in";
    CHECK(scan.isValid());
    PostRequest watch;
    watch.kind   = Kind::WatchFolder;
    watch.folder = "/data/in";
    CHECK(watch.isValid());
    watch.profile.compress = false;
    CHECK(!watch.isValid());                               // watching needs compression

    // kinds
    Kind k;
    for (Kind kind : {Kind::OneRelease, Kind::EachItem, Kind::ScanFolder, Kind::WatchFolder})
        CHECK(PostRequest::kindFromKey(PostRequest::kindKey(kind), k) && k == kind);
    CHECK(PostRequest::kindFromKey(" WATCH ", k) && k == Kind::WatchFolder);
    CHECK(!PostRequest::kindFromKey("other", k));

    // remembered choices: everything but the files
    PostRequest c;
    c.kind = Kind::EachItem;
    c.after = After::MoveToPosted;
    c.moveTo = "/mnt/done|old";                            // a | in the path is kept
    c.genNfo = true; c.nfoInPost = true; c.nfoPassword = false; c.nfoMedia = false;
    c.profile.name = "My profile"; c.profile.compress = false; c.profile.par2Pct = 15;
    c.paths << "/data/a";
    PostRequest back;
    CHECK(PostRequest::fromChoicesLine(c.choicesLine(), back));
    CHECK(back.kind == Kind::EachItem && back.after == After::MoveToPosted && back.moveTo == "/mnt/done|old");
    CHECK(back.genNfo && back.nfoInPost && !back.nfoPassword && !back.nfoMedia);
    CHECK(back.profile.name == "My profile" && !back.profile.compress && back.profile.par2Pct == 15);
    CHECK(back.paths.isEmpty());

    // a broken line changes nothing
    PostRequest untouched;
    untouched.kind = Kind::ScanFolder;
    CHECK(!PostRequest::fromChoicesLine("", untouched));
    CHECK(!PostRequest::fromChoicesLine("bad|0|1|1|1|1|P|1|1|1|1|10|0|", untouched));
    CHECK(!PostRequest::fromChoicesLine("scan|7|1|1|1|1|P|1|1|1|1|10|0|", untouched));
    CHECK(!PostRequest::fromChoicesLine("scan|0|1|1|1|1|P|1|1|1|1|999|0|", untouched));
    CHECK(untouched.kind == Kind::ScanFolder && untouched.after == After::Nothing);

    // options of the last session (Quick Post / Auto Posting): no name, same fields as a profile
    {
        PostProfile opt;
        opt.compress = true; opt.genName = false; opt.genPass = true; opt.par2 = false; opt.par2Pct = 25; opt.keepRar = true;
        CHECK(opt.optionsLine() == "1|0|1|0|25|1");
        PostProfile back;
        CHECK(PostProfile::fromOptionsLine(opt.optionsLine(), back));
        CHECK(back.name.isEmpty() && back.compress && !back.genName && back.genPass && !back.par2 && back.par2Pct == 25 && back.keepRar);
        PostProfile keep = opt;
        CHECK(!PostProfile::fromOptionsLine("", keep));
        CHECK(!PostProfile::fromOptionsLine("1|0|1", keep));
        CHECK(!PostProfile::fromOptionsLine("1|0|1|0|999|1", keep));
        CHECK(keep.par2Pct == 25);
    }

    if (failures == 0)
        std::printf("OK\n");
    return failures ? 1 : 0;
}
