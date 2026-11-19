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


#include "PostRequest.h"

static const char *sKindKeys[] = {"one_release", "each_item", "scan", "watch"};

QString PostRequest::kindKey(Kind kind)
{
    return QString(sKindKeys[static_cast<int>(kind)]);
}

bool PostRequest::kindFromKey(const QString &key, Kind &kind)
{
    for (int i = 0; i < 4; ++i)
    {
        if (key.trimmed().toLower() == sKindKeys[i])
        {
            kind = static_cast<Kind>(i);
            return true;
        }
    }
    return false;
}

bool PostRequest::isValid(QString *why) const
{
    auto fail = [why](const QString &reason) {
        if (why)
            *why = reason;
        return false;
    };
    if (kind == Kind::OneRelease || kind == Kind::EachItem)
    {
        bool hasPath = false;
        for (const QString &p : paths)
            if (!p.trimmed().isEmpty())
                hasPath = true;
        if (!hasPath)
            return fail("nothing to post");
    }
    else if (folder.trimmed().isEmpty())
        return fail("no folder to scan or to watch");
    if (kind == Kind::OneRelease && after != After::Nothing)
        return fail("moving or deleting the files is only done by Auto Posting");
    if (kind == Kind::WatchFolder && !profile.compress)
        return fail("a folder can only be watched with compression");
    return true;
}

// kind|after|genNfo|nfoInPost|nfoPassword|nfoMedia|<7 fields of the profile>|moveTo (last: it may contain anything)
QString PostRequest::choicesLine() const
{
    return QString("%1|%2|%3|%4|%5|%6|%7|%8").arg(kindKey(kind)).arg(static_cast<int>(after))
            .arg(genNfo).arg(nfoInPost).arg(nfoPassword).arg(nfoMedia)
            .arg(profile.toLine(), QString(moveTo).remove('\n').remove('\r'));
}

bool PostRequest::fromChoicesLine(const QString &line, PostRequest &request)
{
    QStringList f = line.split('|');
    if (f.size() < 14)
        return false;
    Kind kind;
    if (!kindFromKey(f[0], kind))
        return false;
    bool ok = false;
    int after = f[1].toInt(&ok);
    if (!ok || after < 0 || after > 2)
        return false;
    PostProfile profile;
    if (!PostProfile::fromLine(f.mid(6, 7).join('|'), profile))
        return false;
    request.kind        = kind;
    request.after       = static_cast<After>(after);
    request.genNfo      = f[2] == "1";
    request.nfoInPost   = f[3] == "1";
    request.nfoPassword = f[4] == "1";
    request.nfoMedia    = f[5] == "1";
    request.profile     = profile;
    request.moveTo      = f.mid(13).join('|').trimmed();
    return true;
}
