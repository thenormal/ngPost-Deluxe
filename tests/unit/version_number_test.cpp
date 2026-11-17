// comparison of the version numbers used by the new version notice
#include "VersionNumber.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

int main()
{
    using namespace VersionNumber;
    CHECK(isNewer("1.4", "1.3"));
    CHECK(isNewer("v1.4", "1.3"));
    CHECK(isNewer("1.10", "1.9"));      // numbers, not text
    CHECK(isNewer("1.3.1", "1.3"));
    CHECK(isNewer("2.0", "1.99"));
    CHECK(!isNewer("1.3", "1.3"));
    CHECK(!isNewer("1.3.0", "1.3"));
    CHECK(!isNewer("1.2", "1.3"));
    CHECK(!isNewer("1.4-rc1", "1.3"));  // release candidates are never announced
    CHECK(!isNewer("abc", "1.3"));
    CHECK(!isNewer("", "1.3"));
    CHECK(!isNewer("1.4", ""));
    CHECK(clean(" v1.3 ") == "1.3");
    CHECK(clean("1.3-rc2").isEmpty());
    CHECK(clean("1..3").isEmpty());
    if (failures == 0)
        std::printf("OK\n");
    return failures ? 1 : 0;
}
