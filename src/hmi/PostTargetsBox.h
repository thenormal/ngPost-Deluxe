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

#ifndef POSTTARGETSBOX_H
#define POSTTARGETSBOX_H

#include <QObject>
#include "utils/PostProfile.h"

class NgPost;
class GroupsEdit;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QWidget;

/*!
 * \brief Rows of Quick Post and Auto Posting for where a post goes and its name: name template, newsgroups,
 * servers and (Auto Posting) NZB folder. An empty field means "the one of the Settings".
 * The widgets are put in the grid of a card of the page (addRows).
 */
class PostTargetsBox : public QObject
{
    Q_OBJECT
public:
    enum Part {NAME = 0, GROUPS, SERVERS, NZB_DIR};

    PostTargetsBox(NgPost *ngPost, QWidget *parent, bool withNzbDir);

    int  addRows(QGridLayout *grid, int row); //!< returns the next free row
    PostTargets targets() const;              //!< nzbDir is empty without the NZB folder row
    //! onlyFilled: the empty parts of t don't change the fields (a profile without them)
    void setTargets(const PostTargets &t, bool onlyFilled);
    void refreshHints();                      //!< texts shown in the empty fields (the values of the Settings)
    void retranslate();
    GroupsEdit *groupsEdit() const { return _groups; }

    //! the text of a list of servers (the keys that are not in the Servers page are marked)
    static QString serversText(NgPost *ngPost, const QStringList &keys);

signals:
    void changed(int part); //!< a change made by the user (not by setTargets)

private:
    void _fillServersMenu();
    void _updateServersButton();

    NgPost      *_ngPost;
    QWidget     *_parent;
    QLabel      *_nameLbl;
    QLineEdit   *_nameEdit;
    QLabel      *_groupsLbl;
    GroupsEdit  *_groups;
    QLabel      *_serversLbl;
    QPushButton *_serversButton;
    QLabel      *_nzbLbl;
    QLineEdit   *_nzbEdit;
    QPushButton *_nzbButton;
    QStringList  _servers;
    bool         _loading;
};

#endif // POSTTARGETSBOX_H
