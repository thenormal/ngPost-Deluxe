//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>

#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QList>

class QUrl;
class QNetworkAccessManager;
class QNetworkReply;
class SmtpClient;

/*!
 * \brief Notifications when posts end: system notification (GUI), Discord, Telegram, Slack, ntfy, Gotify, Pushover, email and a generic webhook.
 * The addresses and tokens are secrets: they are never written in the log. A failed sending only writes a line
 * in the log, it never stops a post.
 */
class Notifier : public QObject
{
    Q_OBJECT
public:
    enum class State { OK, ERRORS, FAILED };

    struct Settings {
        bool    onSuccess  = true;
        bool    onErrors   = true;
#ifdef __USE_HMI__
        bool    tray       = true;   //!< the program with a window shows a system notification, unless the user turns it off
#else
        bool    tray       = false;  //!< command line / Docker: no system notification, only the addresses that are set
#endif
        bool    allDone    = true;  //!< one message when all the posts are done (false: one after each post)
        QString discord;            //!< webhook address of Discord
        QString telegramToken;
        QString telegramChat;
        QString webhook;            //!< address that receives a JSON POST {"title", "text"}
        QString slack;              //!< incoming webhook address of Slack
        QString ntfy;               //!< address of the ntfy topic (https://ntfy.sh/mytopic or your own server)
        QString ntfyToken;          //!< optional access token of ntfy
        QString gotifyUrl;          //!< address of the Gotify server
        QString gotifyToken;        //!< token of the Gotify application
        QString pushoverToken;      //!< token of the Pushover application
        QString pushoverUser;       //!< user (or group) key of Pushover
        QString emailHost;          //!< SMTP server
        quint16 emailPort = 587;
        QString emailSecurity = "starttls"; //!< starttls (587), ssl (465) or none
        QString emailUser, emailPass;
        QString emailFrom;          //!< empty: the user, if it is an address
        QString emailTo;            //!< one or more addresses separated by a comma

        bool hasTelegram() const { return !telegramToken.isEmpty() && !telegramChat.isEmpty(); }
        bool hasGotify() const   { return !gotifyUrl.isEmpty() && !gotifyToken.isEmpty(); }
        bool hasPushover() const { return !pushoverToken.isEmpty() && !pushoverUser.isEmpty(); }
        QString effectiveEmailFrom() const { return !emailFrom.trimmed().isEmpty() ? emailFrom.trimmed() : (emailUser.contains('@') ? emailUser.trimmed() : QString()); }
        bool hasEmail() const;
        bool hasRemote() const { return !discord.isEmpty() || !webhook.isEmpty() || hasTelegram() || !slack.isEmpty() || !ntfy.isEmpty()
                                        || hasGotify() || hasPushover() || hasEmail(); }
        bool isActive() const  { return tray || hasRemote(); }
    };

    struct Result {
        QString name;
        State   state = State::OK;
        QString size;
        QString duration;
        QString speed;
        uint    nbFailed = 0;
        uint    nbTotal  = 0;
    };

    explicit Notifier(QObject *parent = nullptr);

    void setAppName(const QString &name) { _appName = name; }
    Settings &settings() { return _settings; }
    const Settings &settings() const { return _settings; }

    //! a post has ended; allDone: no other post is waiting
    void jobEnded(const Result &result, bool allDone);
    //! sends a test message to every channel configured, returns what was done
    QString sendTest(const QString &appName);
    //! number of messages being sent
    int pending() const { return _pending; }
    //! for the diagnostic report (never the addresses or the tokens): messages sent, failed, and the last failure ("Discord: ... (12:30:05)")
    int nbSent() const { return _nbSent; }
    int nbFailed() const { return _nbFailed; }
    const QString &lastFailure() const { return _lastFailure; }

    static QString message(const QList<Result> &results, const QString &appName, QString *title = nullptr);
    static QJsonObject discordBody(const QString &text);

signals:
    void trayMessage(const QString &title, const QString &text);
    void log(const QString &line);
    void allSent();   //!< nothing is being sent any more

private slots:
    void _onReplyFinished();

private:
    bool _wanted(State state) const;
    void _send(const QString &title, const QString &text);
    void _post(const QString &channel, const QUrl &url, const QByteArray &body, const QByteArray &contentType = "application/json",
               const QList<QPair<QByteArray, QByteArray>> &headers = QList<QPair<QByteArray, QByteArray>>());
    void _sendEmail(const QString &title, const QString &text);
    void _failed(const QString &channel, const QString &why);

    Settings                _settings;
    QNetworkAccessManager  *_net;
    QList<Result>           _waiting;  //!< results kept until all the posts are done
    int                     _pending = 0;
    int                     _nbSent = 0, _nbFailed = 0;
    QString                 _lastFailure;
    QString                 _appName;
};

#endif // NOTIFIER_H
