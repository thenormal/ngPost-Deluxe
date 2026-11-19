// speed limit: the slots of time reserved by the connections before sending an Article
#include "SpeedLimiter.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

static const qint64 S  = 1000000000LL; // 1 s in ns
static const qint64 MB = 1024 * 1024;

int main()
{
    // no limit: never wait
    qint64 next = 0;
    CHECK(SpeedLimiter::delayNs(10 * S, next, 700000, 0) == 0);
    CHECK(SpeedLimiter::reserve(700000) == 0);

    // 1 MB/s, at most 1 s of credit: the first MB goes at once, then one MB per second
    next = 0;
    const qint64 now = 100 * S;
    CHECK(SpeedLimiter::delayNs(now, next, MB, MB) == 0); // uses the credit
    CHECK(SpeedLimiter::delayNs(now, next, MB, MB) == 0); // the credit is used up exactly now
    CHECK(SpeedLimiter::delayNs(now, next, MB, MB) == S);
    CHECK(SpeedLimiter::delayNs(now, next, MB / 2, MB) == 2 * S);
    CHECK(SpeedLimiter::delayNs(now, next, MB, MB) == 2 * S + S / 2);

    // 10 connections asking at the same time: the slots follow one another, so 10 MB take about 10 s whatever the number of connections
    next = 0;
    qint64 last = 0;
    for (int i = 0; i < 10; ++i)
        last = SpeedLimiter::delayNs(now, next, MB, MB);
    CHECK(last == 8 * S);
    CHECK(next == now + 9 * S);

    // a long pause is not saved up: after 1 min, still only 1 s of credit
    CHECK(SpeedLimiter::delayNs(now + 60 * S, next, MB, MB) == 0);
    CHECK(SpeedLimiter::delayNs(now + 60 * S, next, MB, MB) == 0);
    CHECK(SpeedLimiter::delayNs(now + 60 * S, next, MB, MB) == S);

    // the limit can be changed at any time, 0 or less turns it off
    SpeedLimiter::setLimit(5 * MB);
    CHECK(SpeedLimiter::limit() == 5 * MB);
    SpeedLimiter::setLimit(-3);
    CHECK(SpeedLimiter::limit() == 0);

    // the real clock (just started, so no credit yet): 1 MB/s, the second MB waits about 1 s, the third about 2 s
    SpeedLimiter::setLimit(MB);
    CHECK(SpeedLimiter::reserve(MB) == 0);
    const int wait1 = SpeedLimiter::reserve(MB);
    const int wait2 = SpeedLimiter::reserve(MB);
    CHECK(wait1 >= 990 && wait1 <= 1001);
    CHECK(wait2 >= 1990 && wait2 <= 2001);
    SpeedLimiter::setLimit(0);

    if (failures == 0)
        std::printf("OK\n");
    return failures ? 1 : 0;
}
