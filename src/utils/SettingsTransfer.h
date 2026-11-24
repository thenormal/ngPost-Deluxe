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


#ifndef SETTINGSTRANSFER_H
#define SETTINGSTRANSFER_H

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

/*!
 * \brief Export and import of the settings and the profiles (to change computer or to share a configuration).
 * Pure text functions on the lines of the configuration file ("key = value" and "[server]" blocks): no access to the program state,
 * so everything is testable. What is exported is a list of the settings that can move from one computer to another:
 * never a password, a token or a private address (servers, fixed archive password, proxy, upload address, Discord / Telegram /
 * webhook), never a path of the computer (folders, programs), never a command, and never the history or the queue.
 */
class SettingsTransfer
{
public:
    struct ExportOptions {
        bool general       = true;   //!< the settings of the program, the options of Quick Post and Auto Posting
        bool profiles      = true;   //!< the profiles and the favorite newsgroups
        bool notifications = true;   //!< which notifications (never the addresses or tokens)
        bool servers       = false;  //!< host, port, user, connections (never the password)
    };

    struct Server {
        QString host, port, ssl, user, connections, enabled, nzbCheck;
        QString identity() const;    //!< user@host:port (lower case), what makes a server the same as another
    };

    //! the content of an exported file
    struct Parsed {
        bool                            valid = false;   //!< the file starts like a file of settings
        QList<QPair<QString, QString>>  settings;        //!< key (lower case) and value
        QList<QPair<QString, QString>>  notifications;
        QStringList                     profiles;        //!< the lines "name|compress|..." of PostProfile
        QString                         favoriteGroups;  //!< comma separated
        QList<Server>                   servers;
        int                             ignored = 0;     //!< lines of keys that are not exchanged (path, password, unknown)
    };

    struct ImportOptions {
        bool general = true, profiles = true, notifications = true, servers = true;
        bool replaceProfiles = false; //!< a profile with the same name: replaced (otherwise the existing one is kept)
    };

    struct Summary {
        int settings = 0;          //!< settings written (new or changed)
        int notifications = 0;
        int profilesNew = 0, profilesSame = 0; //!< same = a profile with this name exists already
        int serversNew = 0, serversPresent = 0;
        int serversNeedPassword = 0; //!< new servers that have a user: the password must be typed
        int ignored = 0;
    };

    static constexpr const char *HEADER = "# ngPost Deluxe settings";

    //! the keys exchanged (lower case)
    static bool isGeneralKey(const QString &key);
    static bool isNotificationKey(const QString &key);
    static bool isSecretOrLocalKey(const QString &key); //!< never exchanged: passwords, tokens, paths, commands

    //! the text of the file to write, from the text of the configuration file
    static QString exportText(const QString &configText, const ExportOptions &options, const QString &appName,
                              const QString &appVersion, const QDateTime &when);

    static Parsed parse(const QString &exportedText);

    //! what an import would do on this configuration (nothing is changed)
    static Summary summarize(const Parsed &parsed, const QString &configText, const ImportOptions &options);

    //! the configuration text with the imported parts merged in; \a summary (optional) says what was done
    static QString merge(const QString &configText, const Parsed &parsed, const ImportOptions &options, Summary *summary = nullptr);
};

#endif // SETTINGSTRANSFER_H
