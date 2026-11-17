//========================================================================
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//========================================================================

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;

/*!
 * \brief Looks for a newer release on GitHub. It only tells: nothing is downloaded or installed.
 * One request to the "latest release" page of the project (GitHub only sees the IP address). The drafts and the
 * pre-releases (release candidates) are never "latest". A failure is silent: the check is tried again later.
 * For the tests the address can be replaced with the environment variable NGPOST_UPDATE_API.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void check(const QString &currentVersion);   //!< asks GitHub (answers with the signals below)

signals:
    void answered();                                              //!< a valid answer was received (newer or not)
    void newerVersion(const QString &version, const QString &url); //!< a newer release exists: its number and its page
    void log(const QString &line);

private slots:
    void _onFinished();

private:
    QNetworkAccessManager *_net;
    QString                _current;
    bool                   _running;
};

#endif // UPDATECHECKER_H
