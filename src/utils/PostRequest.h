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


#ifndef POSTREQUEST_H
#define POSTREQUEST_H

#include "PostProfile.h"
#include <QStringList>

/*!
 * \brief What the user wants to post, independent of the page that posts it.
 * Built by Guided Post and given to MainWindow::startPost(), which hands it to Quick Post
 * (one release) or to Auto Posting (one post for each item, scan or watch of a folder).
 * The choices (not the files) can be remembered in the config file on one line (key guided_post).
 */
struct PostRequest
{
    enum class Kind  { OneRelease, EachItem, ScanFolder, WatchFolder };
    enum class After { Nothing, MoveToPosted, Delete }; //!< what happens to the files once posted (Auto Posting only)

    Kind        kind = Kind::OneRelease;
    QStringList paths;        //!< files and folders (OneRelease, EachItem)
    QString     folder;       //!< folder to scan or to watch (ScanFolder, WatchFolder)
    PostProfile profile = PostProfile::recommended();
    bool        genNfo      = false;
    bool        nfoInPost   = false;
    bool        nfoPassword = false;
    bool        nfoMedia    = true;
    After       after  = After::Nothing;
    QString     moveTo;       //!< MoveToPosted: a name (folder created next to each item) or a full path; empty: "Posted"

    //! one release in Quick Post, everything else in Auto Posting
    bool usesAutoPosting() const { return kind != Kind::OneRelease; }

    //! false with the reason (in English, for the log) when the request can't be posted as it is
    bool isValid(QString *why = nullptr) const;

    //! the choices on one line (kind, after the post, NFO, profile), without the files
    QString choicesLine() const;
    //! false when the line can't be read: the request is not changed
    static bool fromChoicesLine(const QString &line, PostRequest &request);

    static QString kindKey(Kind kind);   //!< one_release, each_item, scan, watch
    static bool    kindFromKey(const QString &key, Kind &kind);
};

#endif // POSTREQUEST_H
