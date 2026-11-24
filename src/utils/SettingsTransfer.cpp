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


#include "SettingsTransfer.h"
#include "PostProfile.h"
#include <QHash>
#include <QSet>

namespace {
const QSet<QString> &generalKeys()
{
    static const QSet<QString> keys = {
        "lang", "theme", "check_updates",
        "quick_options", "quick_profile", "quick_targets", "auto_options", "auto_profile", "auto_targets", "auto_mode",
        "guided_post", "guided_skip_where", "move_posted",
        "gen_nfo", "nfo_in_post", "nfo_password", "nfo_media", "nfo_style", "nfo_ascii", "nfo_text",
        "limit_speed", "max_speed", "warn_duplicates", "queue",
        "groups", "group_policy", "thread", "article_size", "retry", "msg_id", "obfuscate", "nzb_rm_accents",
        "auto_close_tabs", "resume_wait", "no_resume_auto", "sock_timeout", "prepare_packing",
        "pack", "length_name", "length_pass", "rar_size", "rar_max", "keep_rar", "rar_no_root_folder",
        "rar_extra", "par2_pct", "par2_args",
        "monitor_nzb_folders", "monitor_extensions", "monitor_ignore_dir", "gen_from", "field_separator"};
    return keys;
}
const QSet<QString> &notificationKeys()
{
    static const QSet<QString> keys = {"notify_ok", "notify_errors", "notify_tray", "notify_when"};
    return keys;
}
const QSet<QString> &serverKeys()
{
    static const QSet<QString> keys = {"host", "port", "ssl", "user", "pass", "connection", "enabled", "nzbcheck"};
    return keys;
}

struct Line { bool isKey = false; QString key, value; };

//! "key = value" (the value keeps its "=" and spaces inside); comments, empty lines and "[server]" are not keys
Line splitLine(const QString &raw)
{
    Line l;
    QString t = raw.trimmed();
    if (t.isEmpty() || t.startsWith('#') || t.startsWith('/') || t.startsWith('['))
        return l;
    int eq = t.indexOf('=');
    if (eq <= 0)
        return l;
    l.isKey = true;
    l.key   = t.left(eq).trimmed().toLower();
    l.value = t.mid(eq + 1).trimmed();
    return l;
}

QString lower(const QString &s) { return s.trimmed().toLower(); }

//! the servers of a configuration text
QList<SettingsTransfer::Server> readServers(const QString &text)
{
    QList<SettingsTransfer::Server> out;
    bool in = false;
    for (const QString &raw : text.split('\n'))
    {
        QString t = raw.trimmed();
        if (t == "[server]")
        {
            out << SettingsTransfer::Server();
            in = true;
            continue;
        }
        Line l = splitLine(raw);
        if (!in || !l.isKey || !serverKeys().contains(l.key))
            continue;
        SettingsTransfer::Server &s = out.last();
        if (l.key == "host") s.host = l.value;
        else if (l.key == "port") s.port = l.value;
        else if (l.key == "ssl") s.ssl = l.value;
        else if (l.key == "user") s.user = l.value;
        else if (l.key == "connection") s.connections = l.value;
        else if (l.key == "enabled") s.enabled = l.value;
        else if (l.key == "nzbcheck") s.nzbCheck = l.value;
    }
    return out;
}

QString onOff(const QString &v)
{
    QString t = lower(v);
    return (t == "true" || t == "on" || t == "1" || t == "yes") ? "true" : "false";
}
} // namespace

QString SettingsTransfer::Server::identity() const
{
    return QString("%1@%2:%3").arg(lower(user), lower(host), port.trimmed());
}

bool SettingsTransfer::isGeneralKey(const QString &key)       { return generalKeys().contains(lower(key)); }
bool SettingsTransfer::isNotificationKey(const QString &key)  { return notificationKeys().contains(lower(key)); }

bool SettingsTransfer::isSecretOrLocalKey(const QString &key)
{
    static const QSet<QString> keys = {
        "pass", "rar_pass", "proxy_socks5", "nzb_upload_url", "nzb_post_cmd", "shutdown_cmd",
        "notify_discord", "notify_telegram_token", "notify_telegram_chat", "notify_webhook",
        "nzbpath", "tmp_dir", "tmp_ram", "tmp_ram_ratio", "rar_path", "par2_path", "mediainfo_path", "post_history", "history",
        "log_in_file", "name_counter", "update_last_check", "resume_queue", "from", "disp_progress"};
    return keys.contains(lower(key));
}

QString SettingsTransfer::exportText(const QString &configText, const ExportOptions &options, const QString &appName,
                                     const QString &appVersion, const QDateTime &when)
{
    QStringList general, notifications, profiles;
    QString favorites;
    for (const QString &raw : configText.split('\n'))
    {
        Line l = splitLine(raw);
        if (!l.isKey)
            continue;
        if (l.key == "profile")
            profiles << "profile = " + l.value;
        else if (l.key == "favorite_groups")
            favorites = l.value;
        else if (notificationKeys().contains(l.key))
            notifications << l.key + " = " + l.value;
        else if (generalKeys().contains(l.key))
            general << l.key + " = " + l.value;
    }

    QStringList out;
    out << HEADER + QString(" (format 1)")
        << QString("# written by %1 %2 on %3").arg(appName, appVersion, when.toString("yyyy-MM-dd hh:mm"))
        << "# No password, no token and no address of a notification, no path of the computer: they stay on the computer."
        << "# Import it in ngPost Deluxe: Settings, History and queue, Import the settings."
        << "";
    if (options.general)
        out << "## Settings" << general << "";
    if (options.notifications)
        out << "## Notifications (which ones: the addresses and tokens are not exported)" << notifications << "";
    if (options.profiles)
    {
        out << "## Profiles";
        out << profiles;
        if (!favorites.isEmpty())
            out << "favorite_groups = " + favorites;
        out << "";
    }
    if (options.servers)
    {
        out << "## Servers (without the password: type it after the import)";
        for (const Server &s : readServers(configText))
        {
            out << "[server]" << "host = " + s.host << "port = " + s.port << "ssl = " + s.ssl << "user = " + s.user
                << "connection = " + s.connections << "enabled = " + s.enabled << "nzbCheck = " + s.nzbCheck << "";
        }
    }
    return out.join("\n") + "\n";
}

SettingsTransfer::Parsed SettingsTransfer::parse(const QString &exportedText)
{
    Parsed p;
    QStringList lines = exportedText.split('\n');
    for (const QString &raw : lines)
    {
        QString t = raw.trimmed();
        if (t.isEmpty())
            continue;
        p.valid = t.startsWith(HEADER);
        break;
    }
    if (!p.valid)
        return p;

    Server *server = nullptr;
    QList<Server> servers;
    for (const QString &raw : lines)
    {
        if (raw.trimmed() == "[server]")
        {
            servers << Server();
            server = &servers.last();
            continue;
        }
        Line l = splitLine(raw);
        if (!l.isKey)
            continue;
        if (server && serverKeys().contains(l.key))
        {
            if (l.key == "host") server->host = l.value;
            else if (l.key == "port") server->port = l.value;
            else if (l.key == "ssl") server->ssl = l.value;
            else if (l.key == "user") server->user = l.value;
            else if (l.key == "connection") server->connections = l.value;
            else if (l.key == "enabled") server->enabled = l.value;
            else if (l.key == "nzbcheck") server->nzbCheck = l.value;
            // "pass" is never read: even in a file edited by hand
            continue;
        }
        if (l.key == "profile")
            p.profiles << l.value;
        else if (l.key == "favorite_groups")
            p.favoriteGroups = l.value;
        else if (notificationKeys().contains(l.key))
            p.notifications << qMakePair(l.key, l.value);
        else if (generalKeys().contains(l.key))
            p.settings << qMakePair(l.key, l.value);
        else
            ++p.ignored;
    }
    for (const Server &s : servers)
        if (!s.host.trimmed().isEmpty() && !s.port.trimmed().isEmpty())
            p.servers << s;
    return p;
}

namespace {
//! index of the line with this key (not commented), -1 if none
int findKey(const QStringList &lines, const QString &key)
{
    for (int i = 0; i < lines.size(); ++i)
    {
        Line l = splitLine(lines.at(i));
        if (l.isKey && l.key == key)
            return i;
    }
    return -1;
}

//! where the new settings are written: before the section of the servers
int insertionPoint(const QStringList &lines)
{
    for (int i = 0; i < lines.size(); ++i)
        if (lines.at(i).trimmed().startsWith("##") && lines.at(i).contains("servers section"))
            return qMax(0, i - 1);
    for (int i = 0; i < lines.size(); ++i)
        if (lines.at(i).trimmed() == "[server]")
            return i;
    return lines.size();
}

QString mergeGroups(const QString &current, const QString &imported)
{
    QStringList out;
    for (const QString &g : current.split(',') + imported.split(','))
    {
        QString t = g.trimmed();
        if (!t.isEmpty() && !out.contains(t, Qt::CaseInsensitive))
            out << t;
    }
    return out.join(",");
}
} // namespace

QString SettingsTransfer::merge(const QString &configText, const Parsed &parsed, const ImportOptions &options, Summary *summary)
{
    Summary sum;
    sum.ignored = parsed.ignored;
    QStringList lines = configText.split('\n');
    while (lines.size() && lines.last().isEmpty())
        lines.removeLast();
    QStringList added;

    auto setKey = [&](const QString &key, const QString &value){
        int i = findKey(lines, key);
        if (i >= 0)
        {
            QString oldKey = lines.at(i).left(lines.at(i).indexOf('=')).trimmed();
            lines[i] = oldKey + " = " + value;
        }
        else
            added << key + " = " + value;
    };
    if (options.general)
        for (const auto &kv : parsed.settings)
        {
            setKey(kv.first, kv.second);
            ++sum.settings;
        }
    if (options.notifications)
        for (const auto &kv : parsed.notifications)
        {
            setKey(kv.first, kv.second);
            ++sum.notifications;
        }

    if (options.profiles)
    {
        QHash<QString, int> existing; // lower case name -> line
        for (int i = 0; i < lines.size(); ++i)
        {
            Line l = splitLine(lines.at(i));
            PostProfile p;
            if (l.isKey && l.key == "profile" && PostProfile::fromLine(l.value, p))
                existing.insert(lower(p.name), i);
        }
        for (const QString &value : parsed.profiles)
        {
            PostProfile p;
            if (!PostProfile::fromLine(value, p))
            {
                ++sum.ignored;
                continue;
            }
            auto it = existing.constFind(lower(p.name));
            if (it == existing.constEnd())
            {
                added << "profile = " + value;
                existing.insert(lower(p.name), -1);
                ++sum.profilesNew;
            }
            else
            {
                ++sum.profilesSame;
                if (options.replaceProfiles && it.value() >= 0)
                    lines[it.value()] = "profile = " + value;
            }
        }
        if (!parsed.favoriteGroups.isEmpty())
        {
            int i = findKey(lines, "favorite_groups");
            QString cur = i >= 0 ? splitLine(lines.at(i)).value : QString();
            setKey("favorite_groups", mergeGroups(cur, parsed.favoriteGroups));
        }
    }

    if (!added.isEmpty())
    {
        int at = insertionPoint(lines);
        for (int k = 0; k < added.size(); ++k)
            lines.insert(at + k, added.at(k));
        lines.insert(at + added.size(), QString());
    }

    if (options.servers)
    {
        QSet<QString> have;
        for (const Server &s : readServers(configText))
            have.insert(s.identity());
        for (const Server &s : parsed.servers)
        {
            if (have.contains(s.identity()))
            {
                ++sum.serversPresent;
                continue;
            }
            have.insert(s.identity());
            ++sum.serversNew;
            bool needsPass = !s.user.trimmed().isEmpty();
            if (needsPass)
                ++sum.serversNeedPassword;
            lines << "" << "[server]" << "host = " + s.host << "port = " + s.port << "ssl  = " + onOff(s.ssl.isEmpty() ? "true" : s.ssl)
                  << "user = " + s.user << "pass = "
                  << "connection = " + (s.connections.isEmpty() ? QString("10") : s.connections)
                  // a server with a user but no password can't connect: it is switched off until the password is typed
                  << "enabled = " + (needsPass ? QString("false") : onOff(s.enabled.isEmpty() ? "true" : s.enabled))
                  << "nzbCheck = " + onOff(s.nzbCheck);
        }
    }
    if (summary)
        *summary = sum;
    return lines.join("\n") + "\n";
}

SettingsTransfer::Summary SettingsTransfer::summarize(const Parsed &parsed, const QString &configText, const ImportOptions &options)
{
    Summary s;
    merge(configText, parsed, options, &s);
    return s;
}
