#include "SpeedLimiter.h"

#include <QAtomicInteger>
#include <QElapsedTimer>
#include <QMutex>

namespace
{
QAtomicInteger<qint64> sLimit(0);
QMutex                 sMutex;
qint64                 sNextFreeNs = 0;
QElapsedTimer         &limiterClock()
{
    static QElapsedTimer timer;
    if (!timer.isValid())
        timer.start();
    return timer;
}
}

void SpeedLimiter::setLimit(qint64 bytesPerSec)
{
    const qint64 limit = bytesPerSec > 0 ? bytesPerSec : 0;
    if (sLimit.fetchAndStoreRelaxed(limit) != limit)
    {
        QMutexLocker lock(&sMutex);
        sNextFreeNs = 0; // a new limit: the slots already given are not carried over
    }
}

qint64 SpeedLimiter::limit() { return sLimit.loadRelaxed(); }

qint64 SpeedLimiter::delayNs(qint64 nowNs, qint64 &nextFreeNs, qint64 bytes, qint64 bytesPerSec)
{
    if (bytesPerSec <= 0 || bytes <= 0)
        return 0;
    // time not used for a while is not saved up: at most sMaxBurstNs of credit
    if (nextFreeNs < nowNs - sMaxBurstNs)
        nextFreeNs = nowNs - sMaxBurstNs;
    const qint64 start = qMax(nowNs, nextFreeNs);
    nextFreeNs += static_cast<qint64>(1e9 * static_cast<double>(bytes) / static_cast<double>(bytesPerSec));
    return start - nowNs;
}

int SpeedLimiter::reserve(qint64 bytes)
{
    const qint64 limit = sLimit.loadRelaxed();
    if (limit <= 0)
        return 0;
    QMutexLocker lock(&sMutex);
    const qint64 delay = delayNs(limiterClock().nsecsElapsed(), sNextFreeNs, bytes, limit);
    return static_cast<int>((delay + 999999) / 1000000);
}
