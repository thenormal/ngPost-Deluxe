#ifndef SPEEDLIMITER_H
#define SPEEDLIMITER_H

#include <QtGlobal>

/*!
 * \brief Global limit of the upload speed, shared by all the connections of all the posts.
 * Before sending an Article, a connection reserves a slot of time for its size:
 * the slots follow one another at the speed chosen, so the average speed stays
 * close to the limit whatever the number of connections.
 * Thread safe (the connections run in several threads). With no limit it costs nothing.
 */
class SpeedLimiter
{
public:
    //! limit in bytes per second (0: no limit). Can be changed at any time, even while posting
    static void setLimit(qint64 bytesPerSec);
    static qint64 limit();

    //! reserve the time to send \a bytes: returns how many ms to wait before sending them (0: send now)
    static int reserve(qint64 bytes);

    //! the same computation without clock and lock, for the tests: \a nextFreeNs is updated
    static qint64 delayNs(qint64 nowNs, qint64 &nextFreeNs, qint64 bytes, qint64 bytesPerSec);

    static constexpr qint64 sMaxBurstNs = 1000000000LL; //!< unused time kept as credit: at most 1 s
};

#endif // SPEEDLIMITER_H
