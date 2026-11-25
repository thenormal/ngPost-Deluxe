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

#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QStringList>
#include <QDateTime>

class QTimer;

/*!
 * \brief Minimal SMTP client to send one text message (for the notifications by email): implicit TLS (port 465), STARTTLS (port 587)
 * or no encryption, AUTH PLAIN / LOGIN. The password and the addresses are never part of an error text.
 */
class SmtpClient : public QObject
{
    Q_OBJECT
public:
    enum class Security { StartTls, Ssl, None };

    struct Config {
        QString     host;
        quint16     port = 587;
        Security    security = Security::StartTls;
        QString     user, pass;
        QString     from;
        QStringList to;
    };

    explicit SmtpClient(QObject *parent = nullptr);

    //! sends the message; finished() is emitted once, with an error that has no secret in it
    void send(const Config &config, const QString &subject, const QString &text);

    static Security securityFromString(const QString &s);   //!< "ssl", "starttls" (default), "none"
    static QString  securityToString(Security s);
    static QStringList splitAddresses(const QString &list); //!< separated by , ; or spaces
    //! the message (headers + base64 body), lines end with CRLF
    static QByteArray buildMessage(const QString &from, const QStringList &to, const QString &subject, const QString &text,
                                   const QDateTime &when, const QString &hostForId);

signals:
    void finished(bool ok, const QString &error);

private slots:
    void _onReadyRead();
    void _onConnected();
    void _onEncrypted();
    void _onError();
    void _onTimeout();

private:
    enum class Step { Greeting, Ehlo, StartTls, EhloTls, Auth, AuthUser, AuthPass, MailFrom, RcptTo, Data, Body, Quit, Done };

    void _command(const QByteArray &line);
    void _handle(int code, const QStringList &lines);
    void _done(bool ok, const QString &error);
    void _afterHello();
    void _sendMailFrom();

    QSslSocket   *_socket;
    QTimer       *_timer;
    Config        _config;
    QByteArray    _message;
    QByteArray    _buffer;
    QStringList   _reply;
    Step          _step = Step::Done;
    bool          _auth = false, _plain = false, _login = false, _startTls = false;
    int           _rcpt = 0;
    bool          _finished = true;
};

#endif // SMTPCLIENT_H
