//========================================================================
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//========================================================================

#include "UpdateChecker.h"
#include "VersionNumber.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace {
const char *sApi  = "https://api.github.com/repos/thenormal/ngPost-Deluxe/releases/latest";
const char *sSite = "https://github.com/thenormal/ngPost-Deluxe/";
}

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent), _net(new QNetworkAccessManager(this)), _current(), _running(false)
{}

void UpdateChecker::check(const QString &currentVersion)
{
    if (_running)
        return;
    _current = currentVersion;
    QUrl url(qEnvironmentVariableIsSet("NGPOST_UPDATE_API") ? qEnvironmentVariable("NGPOST_UPDATE_API") : QString(sApi));
    if (!url.isValid() || (url.scheme() != "https" && url.scheme() != "http"))
        return;
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", QByteArray("ngPost-Deluxe/") + currentVersion.toUtf8()); // GitHub refuses requests without it
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setTransferTimeout(10000); // never wait more than 10 s
    QNetworkReply *reply = _net->get(req);
    _running = true;
    connect(reply, &QNetworkReply::finished, this, &UpdateChecker::_onFinished);
}

void UpdateChecker::_onFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    _running = false;
    if (reply->error() != QNetworkReply::NoError)
        return; // no network, GitHub busy...: nothing to say, it will be tried again later

    const QJsonObject o = QJsonDocument::fromJson(reply->readAll()).object();
    if (o.isEmpty() || o.value("draft").toBool() || o.value("prerelease").toBool())
        return;
    const QString version = VersionNumber::clean(o.value("tag_name").toString());
    if (version.isEmpty())
        return;
    emit answered();
    if (!VersionNumber::isNewer(version, _current))
        return;
    // the page that is opened must be one of the project (unless the address of the check was replaced for a test)
    QString page = o.value("html_url").toString();
    if (!qEnvironmentVariableIsSet("NGPOST_UPDATE_API") && !page.startsWith(sSite))
        page = QString("%1releases").arg(sSite);
    emit newerVersion(version, page);
}
