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

#include "SmtpClient.h"
#include <QTimer>
#include <QHostInfo>
#include <QRegularExpression>
#include <QUuid>

SmtpClient::SmtpClient(QObject *parent) :
    QObject(parent), _socket(new QSslSocket(this)), _timer(new QTimer(this))
{
    _timer->setSingleShot(true);
    connect(_timer, &QTimer::timeout, this, &SmtpClient::_onTimeout);
    connect(_socket, &QIODevice::readyRead, this, &SmtpClient::_onReadyRead);
    connect(_socket, &QAbstractSocket::connected, this, &SmtpClient::_onConnected);
    connect(_socket, &QSslSocket::encrypted, this, &SmtpClient::_onEncrypted);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(_socket, &QAbstractSocket::errorOccurred, this, &SmtpClient::_onError);
#else
    connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &SmtpClient::_onError);
#endif
}

SmtpClient::Security SmtpClient::securityFromString(const QString &s)
{
    QString t = s.trimmed().toLower();
    if (t == "ssl" || t == "tls" || t == "ssl/tls")
        return Security::Ssl;
    if (t == "none" || t == "off" || t == "no")
        return Security::None;
    return Security::StartTls;
}

QString SmtpClient::securityToString(Security s)
{
    switch (s)
    {
    case Security::Ssl:  return "ssl";
    case Security::None: return "none";
    default:             return "starttls";
    }
}

QStringList SmtpClient::splitAddresses(const QString &list)
{
    QStringList out;
    for (const QString &a : list.split(QRegularExpression("[,;\\s]+"), QString::SkipEmptyParts))
        if (a.contains('@') && !out.contains(a))
            out << a;
    return out;
}

static QString encodeHeader(const QString &s)
{
    bool ascii = true;
    for (const QChar &c : s)
        if (c.unicode() > 126 || c.unicode() < 32)
            ascii = false;
    if (ascii)
        return s;
    return "=?UTF-8?B?" + QString::fromLatin1(s.toUtf8().toBase64()) + "?=";
}

static QByteArray base64Lines(const QByteArray &data)
{
    QByteArray b64 = data.toBase64(), out;
    for (int i = 0; i < b64.size(); i += 76)
        out += b64.mid(i, 76) + "\r\n";
    return out;
}

QByteArray SmtpClient::buildMessage(const QString &from, const QStringList &to, const QString &subject, const QString &text,
                                    const QDateTime &when, const QString &hostForId)
{
    QByteArray m;
    auto line = [&m](const QString &s){ m += s.toUtf8() + "\r\n"; };
    line("From: " + from);
    line("To: " + to.join(", "));
    line("Subject: " + encodeHeader(subject));
    line("Date: " + QLocale::c().toString(when.toUTC(), "ddd, dd MMM yyyy hh:mm:ss") + " +0000");
    line("Message-ID: <" + QUuid::createUuid().toString().mid(1, 36) + "@" + (hostForId.isEmpty() ? QString("ngpost") : hostForId) + ">");
    line("MIME-Version: 1.0");
    line("Content-Type: text/plain; charset=utf-8");
    line("Content-Transfer-Encoding: base64");
    line(QString());
    m += base64Lines(text.toUtf8());
    return m;
}

void SmtpClient::send(const Config &config, const QString &subject, const QString &text)
{
    _config   = config;
    _message  = buildMessage(config.from, config.to, subject, text, QDateTime::currentDateTime(), config.host);
    _buffer.clear();
    _reply.clear();
    _auth = _plain = _login = _startTls = false;
    _rcpt = 0;
    _finished = false;
    _step = Step::Greeting;
    _timer->start(20000); // never wait more than 20 s for the whole sending
    if (config.security == Security::Ssl)
        _socket->connectToHostEncrypted(config.host, config.port);
    else
        _socket->connectToHost(config.host, config.port);
}

void SmtpClient::_onConnected() {}

void SmtpClient::_onEncrypted()
{
    // after STARTTLS: say hello again, on the encrypted connection (implicit TLS: the server speaks first)
    if (_step == Step::EhloTls)
        _command("EHLO ngpost");
}

void SmtpClient::_onError()
{
    if (_finished)
        return;
    _done(false, tr("can't connect to the mail server (%1)").arg(_socket->error() == QAbstractSocket::SslHandshakeFailedError
                                                                     ? tr("secure connection refused") : tr("connection error")));
}

void SmtpClient::_onTimeout()
{
    _done(false, tr("the mail server did not answer in time"));
}

void SmtpClient::_command(const QByteArray &line)
{
    _socket->write(line + "\r\n");
}

void SmtpClient::_onReadyRead()
{
    _buffer += _socket->readAll();
    int eol;
    while ((eol = _buffer.indexOf("\r\n")) >= 0 || (eol = _buffer.indexOf('\n')) >= 0)
    {
        QString line = QString::fromUtf8(_buffer.left(eol)).trimmed();
        _buffer.remove(0, _buffer.at(eol) == '\r' ? eol + 2 : eol + 1);
        _reply << line;
        if (line.size() >= 4 && line.at(3) == '-')
            continue; // more lines in this reply
        bool ok = false;
        int code = line.left(3).toInt(&ok);
        QStringList lines = _reply;
        _reply.clear();
        if (!ok)
        {
            _done(false, tr("the mail server answered something unexpected"));
            return;
        }
        _handle(code, lines);
        if (_finished)
            return;
    }
}

void SmtpClient::_afterHello()
{
    if (!_config.user.isEmpty() && (_plain || _login))
    {
        if (_plain)
        {
            _step = Step::Auth;
            QByteArray cred;
            cred.append('\0');
            cred += _config.user.toUtf8();
            cred.append('\0');
            cred += _config.pass.toUtf8();
            _command("AUTH PLAIN " + cred.toBase64());
        }
        else
        {
            _step = Step::AuthUser;
            _command("AUTH LOGIN");
        }
    }
    else
        _sendMailFrom();
}

void SmtpClient::_sendMailFrom()
{
    _step = Step::MailFrom;
    _command("MAIL FROM:<" + _config.from.toUtf8() + ">");
}

void SmtpClient::_handle(int code, const QStringList &lines)
{
    auto fail = [this](const QString &what){ _done(false, what); };
    switch (_step)
    {
    case Step::Greeting:
        if (code != 220) return fail(tr("the mail server refused the connection"));
        _step = Step::Ehlo;
        _command("EHLO ngpost");
        break;
    case Step::Ehlo:
    case Step::EhloTls:
    {
        if (code != 250) return fail(tr("the mail server refused the greeting"));
        _plain = _login = _startTls = false;
        for (const QString &l : lines)
        {
            QString u = l.mid(4).toUpper();
            if (u.startsWith("STARTTLS")) _startTls = true;
            if (u.startsWith("AUTH"))
            {
                if (u.contains("PLAIN")) _plain = true;
                if (u.contains("LOGIN")) _login = true;
            }
        }
        if (_step == Step::Ehlo && _config.security == Security::StartTls)
        {
            if (!_startTls) return fail(tr("the mail server does not offer a secure connection (STARTTLS)"));
            _step = Step::StartTls;
            _command("STARTTLS");
            break;
        }
        if (!_config.user.isEmpty() && !_plain && !_login)
            return fail(tr("the mail server does not offer a login method that is supported"));
        _afterHello();
        break;
    }
    case Step::StartTls:
        if (code != 220) return fail(tr("the mail server refused the secure connection"));
        _step = Step::EhloTls; // the EHLO is sent when the encryption is ready
        _socket->startClientEncryption();
        break;
    case Step::Auth:
        if (code != 235) return fail(tr("the mail server refused the login"));
        _sendMailFrom();
        break;
    case Step::AuthUser:
        if (code != 334) return fail(tr("the mail server refused the login"));
        _step = Step::AuthPass;
        _command(_config.user.toUtf8().toBase64());
        break;
    case Step::AuthPass:
        if (code != 334) return fail(tr("the mail server refused the login"));
        _step = Step::Auth;
        _command(_config.pass.toUtf8().toBase64());
        break;
    case Step::MailFrom:
        if (code != 250) return fail(tr("the mail server refused the sender"));
        _step = Step::RcptTo;
        _rcpt = 0;
        _command("RCPT TO:<" + _config.to.at(_rcpt++).toUtf8() + ">");
        break;
    case Step::RcptTo:
        if (code != 250 && code != 251) return fail(tr("the mail server refused a recipient"));
        if (_rcpt < _config.to.size())
            _command("RCPT TO:<" + _config.to.at(_rcpt++).toUtf8() + ">");
        else
        {
            _step = Step::Data;
            _command("DATA");
        }
        break;
    case Step::Data:
        if (code != 354) return fail(tr("the mail server refused the message"));
        _step = Step::Body;
        _socket->write(_message + ".\r\n"); // the body is base64: no line can start with a dot
        break;
    case Step::Body:
        if (code != 250) return fail(tr("the mail server refused the message"));
        _step = Step::Quit;
        _command("QUIT");
        _done(true, QString());
        break;
    default:
        break;
    }
}

void SmtpClient::_done(bool ok, const QString &error)
{
    if (_finished)
        return;
    _finished = true;
    _timer->stop();
    _step = Step::Done;
    if (_socket->state() != QAbstractSocket::UnconnectedState)
    {
        _socket->flush();
        _socket->disconnectFromHost();
    }
    emit finished(ok, error);
}
