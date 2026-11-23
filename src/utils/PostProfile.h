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

#ifndef POSTPROFILE_H
#define POSTPROFILE_H

#include <QString>
#include <QStringList>
class QDateTime;

/*!
 * \brief Where a post goes and how it is named: the newsgroups, the servers, the folder of the NZB file and a
 * template for the name. An empty part means "the one of the Settings" (a profile doesn't change it).
 */
struct PostTargets
{
    QStringList groups;        //!< empty: the newsgroups of the Settings
    QStringList servers;       //!< keys of the servers (NgPost::serverKey), empty: the enabled servers
    QString     nzbDir;        //!< empty: the NZB folder of the Settings
    QString     nameTemplate;  //!< empty: the name of the file or folder, else with {name} {date} {time} {n}

    bool isEmpty() const { return groups.isEmpty() && servers.isEmpty() && nzbDir.isEmpty() && nameTemplate.isEmpty(); }
    bool operator==(const PostTargets &o) const
    {
        return groups == o.groups && servers == o.servers && nzbDir == o.nzbDir && nameTemplate == o.nameTemplate;
    }
    bool operator!=(const PostTargets &o) const { return !(*this == o); }

    //! the 4 parts separated by '|' (each part is percent-encoded: no '|', '=', '#' nor new line in the config file)
    QString toLine() const;
    static PostTargets fromLine(const QString &line);
    static PostTargets fromFields(const QStringList &fields); //!< the 4 parts (missing ones are empty)

    //! name of a post from a template: {name} (file or folder), {date} (2026-11-23), {time} (15-04), {n} (counter).
    //! The characters that a file name can't have are removed. An empty template (or result) gives the name.
    static QString applyTemplate(const QString &nameTemplate, const QString &name, const QDateTime &when, uint counter);
    static bool usesCounter(const QString &nameTemplate);
};

/*!
 * \brief A set of options for a post (what the user sets in the Archive card), and optionally its targets
 * (newsgroups, servers, NZB folder, name template). Saved in the config file with the key PROFILE, one line per
 * profile: name|compress|genName|genPass|par2|par2Pct|keepRar, followed by |groups|servers|nzbDir|template
 * when the profile has targets (older versions read the first 7 fields and ignore the others).
 */
struct PostProfile
{
    QString name;
    bool    compress = true;
    bool    genName  = true;   //!< random name of the archive
    bool    genPass  = true;   //!< random password
    bool    par2     = true;
    uint    par2Pct  = 10;
    bool    keepRar  = false;  //!< keep the archives after the post
    PostTargets targets;       //!< the parts that are empty are not changed by the profile

    //! "Recommended": compression, random name and password, par2 with 10 %
    static PostProfile recommended();

    //! the name without the characters used by the config file
    static QString cleanName(const QString &name);

    QString toLine() const;

    //! the options without name (last choices of Quick Post / Auto Posting): compress|genName|genPass|par2|par2Pct|keepRar
    QString optionsLine() const;
    //! sets the options only (the targets are kept)
    static bool fromOptionsLine(const QString &line, PostProfile &profile);
    static bool fromLine(const QString &line, PostProfile &profile);
};

#endif // POSTPROFILE_H
