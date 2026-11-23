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

#include "PostProfile.h"
#include <QDateTime>
#include <QRegularExpression>
#include <QUrl>

namespace
{
// characters kept as they are (readable config file); '|', '=', '#', '%', ',' and the new lines are encoded
const QByteArray sKeep(" /\\:{}[]()@!&'+~");

QString enc(const QString &str)
{
    return QString::fromUtf8(QUrl::toPercentEncoding(str, sKeep));
}

QString dec(const QString &str)
{
    return QUrl::fromPercentEncoding(str.trimmed().toUtf8());
}

QString encList(const QStringList &list)
{
    QStringList parts;
    for (const QString &item : list)
        if (!item.trimmed().isEmpty())
            parts << enc(item.trimmed());
    return parts.join(',');
}

QStringList decList(const QString &str)
{
    QStringList list;
    for (const QString &item : str.split(',', Qt::SkipEmptyParts))
    {
        QString v = dec(item);
        if (!v.isEmpty() && !list.contains(v))
            list << v;
    }
    return list;
}
} // namespace

QString PostTargets::toLine() const
{
    return QString("%1|%2|%3|%4").arg(encList(groups), encList(servers), enc(nzbDir.trimmed()), enc(nameTemplate.trimmed()));
}

PostTargets PostTargets::fromFields(const QStringList &f)
{
    PostTargets t;
    t.groups       = decList(f.value(0));
    for (QString &g : t.groups)
        g = g.toLower();
    t.servers      = decList(f.value(1));
    t.nzbDir       = dec(f.value(2));
    t.nameTemplate = dec(f.value(3));
    return t;
}

PostTargets PostTargets::fromLine(const QString &line)
{
    return fromFields(line.split('|'));
}

QString PostTargets::applyTemplate(const QString &nameTemplate, const QString &name, const QDateTime &when, uint counter)
{
    QString tpl = nameTemplate.trimmed();
    if (tpl.isEmpty())
        return name;
    QString res = tpl;
    res.replace(QRegularExpression("\\{name\\}", QRegularExpression::CaseInsensitiveOption), name);
    res.replace(QRegularExpression("\\{date\\}", QRegularExpression::CaseInsensitiveOption), when.toString("yyyy-MM-dd"));
    res.replace(QRegularExpression("\\{time\\}", QRegularExpression::CaseInsensitiveOption), when.toString("HH-mm"));
    res.replace(QRegularExpression("\\{n\\}",    QRegularExpression::CaseInsensitiveOption), QString::number(counter));
    res.remove(QRegularExpression("[\\\\/:*?\"<>|\\r\\n\\t]"));
    res = res.trimmed();
    while (res.endsWith('.'))
        res.chop(1); // Windows doesn't keep a dot at the end of a name
    return res.isEmpty() ? name : res;
}

bool PostTargets::usesCounter(const QString &nameTemplate)
{
    return nameTemplate.contains(QRegularExpression("\\{n\\}", QRegularExpression::CaseInsensitiveOption));
}

PostProfile PostProfile::recommended()
{
    return PostProfile(); // the defaults of the struct
}

QString PostProfile::cleanName(const QString &name)
{
    QString n = name;
    n.remove('|').remove('=').remove('#').remove('\n').remove('\r');
    return n.trimmed();
}

QString PostProfile::toLine() const
{
    QString line = QString("%1|%2|%3|%4|%5|%6|%7").arg(cleanName(name)).arg(compress).arg(genName).arg(genPass).arg(par2).arg(par2Pct).arg(keepRar);
    if (!targets.isEmpty())
        line += "|" + targets.toLine();
    return line;
}

QString PostProfile::optionsLine() const
{
    return QString("%1|%2|%3|%4|%5|%6").arg(compress).arg(genName).arg(genPass).arg(par2).arg(par2Pct).arg(keepRar);
}

bool PostProfile::fromOptionsLine(const QString &line, PostProfile &profile)
{
    PostProfile p;
    if (!fromLine(QStringLiteral("x|") + line.trimmed(), p))
        return false;
    profile.compress = p.compress;
    profile.genName  = p.genName;
    profile.genPass  = p.genPass;
    profile.par2     = p.par2;
    profile.par2Pct  = p.par2Pct;
    profile.keepRar  = p.keepRar;
    return true;
}

bool PostProfile::fromLine(const QString &line, PostProfile &profile)
{
    QStringList f = line.split('|');
    if (f.size() < 7 || cleanName(f[0]).isEmpty())
        return false;
    bool ok = true;
    uint pct = f[5].toUInt(&ok);
    if (!ok || pct > 100)
        return false;
    profile.name     = cleanName(f[0]);
    profile.compress = f[1] == "1";
    profile.genName  = f[2] == "1";
    profile.genPass  = f[3] == "1";
    profile.par2     = f[4] == "1";
    profile.par2Pct  = pct;
    profile.keepRar  = f[6] == "1";
    profile.targets  = PostTargets::fromFields(f.mid(7));
    return true;
}
