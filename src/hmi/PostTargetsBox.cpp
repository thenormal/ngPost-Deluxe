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

#include "PostTargetsBox.h"
#include "GroupsEdit.h"
#include "NgPost.h"
#include "nntp/NntpServerParams.h"

#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>

PostTargetsBox::PostTargetsBox(NgPost *ngPost, QWidget *parent, bool withNzbDir) :
    QObject(parent), _ngPost(ngPost), _parent(parent),
    _nameLbl(new QLabel(parent)), _nameEdit(new QLineEdit(parent)),
    _groupsLbl(new QLabel(parent)), _groups(new GroupsEdit(parent)),
    _serversLbl(new QLabel(parent)), _serversButton(new QPushButton(parent)),
    _nzbLbl(nullptr), _nzbEdit(nullptr), _nzbButton(nullptr), _servers(), _loading(false)
{
    _nameEdit->setClearButtonEnabled(true);
    connect(_nameEdit, &QLineEdit::textEdited, this, [this](){ if (!_loading) emit changed(NAME); });
    connect(_groups, &GroupsEdit::changed, this, [this](){ if (!_loading) emit changed(GROUPS); });

    _serversButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _serversButton->setStyleSheet("text-align: left; padding-left: 9px;");
    _serversButton->setObjectName("serversChoice");
    QMenu *menu = new QMenu(_serversButton);
    _serversButton->setMenu(menu);
    connect(menu, &QMenu::aboutToShow, this, &PostTargetsBox::_fillServersMenu);

    if (withNzbDir)
    {
        _nzbLbl    = new QLabel(parent);
        _nzbEdit   = new QLineEdit(parent);
        _nzbButton = new QPushButton("...", parent);
        _nzbButton->setFixedWidth(40);
        _nzbEdit->setClearButtonEnabled(true);
        connect(_nzbEdit, &QLineEdit::textEdited, this, [this](){ if (!_loading) emit changed(NZB_DIR); });
        connect(_nzbButton, &QAbstractButton::clicked, this, [this]() {
            QString start = _nzbEdit->text().isEmpty() ? _ngPost->_nzbPath : _nzbEdit->text();
            QString path = QFileDialog::getExistingDirectory(_parent, tr("Select a Folder"), start, QFileDialog::ShowDirsOnly);
            if (path.isEmpty())
                return;
            _nzbEdit->setText(QDir::toNativeSeparators(path));
            emit changed(NZB_DIR);
        });
    }
    _updateServersButton();
    retranslate();
}

int PostTargetsBox::addRows(QGridLayout *grid, int row)
{
    grid->addWidget(_nameLbl, row, 0);
    grid->addWidget(_nameEdit, row++, 1);
    grid->addWidget(_groupsLbl, row, 0, Qt::AlignLeft | Qt::AlignTop);
    grid->addWidget(_groups, row++, 1);
    grid->addWidget(_serversLbl, row, 0);
    grid->addWidget(_serversButton, row++, 1);
    if (_nzbEdit)
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->setSpacing(8);
        h->addWidget(_nzbEdit, 1);
        h->addWidget(_nzbButton);
        grid->addWidget(_nzbLbl, row, 0);
        grid->addLayout(h, row++, 1);
    }
    _groupsLbl->setContentsMargins(0, 5, 0, 0); // in front of the first line of the groups
    return row;
}

PostTargets PostTargetsBox::targets() const
{
    PostTargets t;
    t.nameTemplate = _nameEdit->text().trimmed();
    t.groups       = _groups->groups();
    t.servers      = _servers;
    if (_nzbEdit)
        t.nzbDir = _nzbEdit->text().trimmed();
    return t;
}

void PostTargetsBox::setTargets(const PostTargets &t, bool onlyFilled)
{
    _loading = true;
    if (!onlyFilled || !t.nameTemplate.isEmpty())
        _nameEdit->setText(t.nameTemplate);
    if (!onlyFilled || !t.groups.isEmpty())
        _groups->setGroups(t.groups);
    if (!onlyFilled || !t.servers.isEmpty())
    {
        _servers = t.servers;
        _updateServersButton();
    }
    if (_nzbEdit && (!onlyFilled || !t.nzbDir.isEmpty()))
        _nzbEdit->setText(QDir::toNativeSeparators(t.nzbDir));
    _loading = false;
}

void PostTargetsBox::refreshHints()
{
    const QStringList settingsGroups = _ngPost->_grpList;
    _groups->setEmptyHint(settingsGroups.isEmpty() ? tr("Add a newsgroup, for example alt.binaries.test")
                                                   : tr("Settings: %1").arg(settingsGroups.join(", ")));
    if (_nzbEdit)
        _nzbEdit->setPlaceholderText(tr("Settings: %1").arg(QDir::toNativeSeparators(_ngPost->_nzbPath)));
    _updateServersButton();
}

void PostTargetsBox::retranslate()
{
    _nameLbl->setText(tr("Name"));
    _nameEdit->setPlaceholderText(tr("{name} (the name of the file or folder)"));
    _nameEdit->setToolTip(tr("Template of the name of the post (NZB file):\n"
                             "{name} the name of the file or folder\n"
                             "{date} the date (2026-11-23)\n"
                             "{time} the time (15-04)\n"
                             "{n} a number that grows by one at each post\n"
                             "Example: {name} [{date}]. Empty: the name of the file or folder."));
    _groupsLbl->setText(tr("Newsgroups"));
    _groups->setToolTip(tr("Newsgroups of this post. Empty: the newsgroups of the Settings."));
    _serversLbl->setText(tr("Servers"));
    _serversButton->setToolTip(tr("Servers used for this post. By default, all the servers enabled in the page Servers."));
    if (_nzbEdit)
    {
        _nzbLbl->setText(tr("NZB folder"));
        _nzbEdit->setToolTip(tr("Folder of the NZB files of these posts. Empty: the folder of the Settings."));
    }
    _groups->retranslate();
    refreshHints();
}

QString PostTargetsBox::serversText(NgPost *ngPost, const QStringList &keys)
{
    QStringList known;
    for (NntpServerParams *srv : ngPost->_nntpServers)
        known << NgPost::serverKey(*srv);
    QStringList parts;
    for (const QString &key : keys)
        parts << (known.contains(key) ? key : tr("%1 (not in the list)").arg(key));
    return parts.join(", ");
}

void PostTargetsBox::_updateServersButton()
{
    if (_servers.isEmpty())
    {
        int nb = 0;
        for (NntpServerParams *srv : _ngPost->_nntpServers)
            if (srv->enabled)
                ++nb;
        _serversButton->setText(tr("All the enabled servers (%1)").arg(nb));
    }
    else
        _serversButton->setText(serversText(_ngPost, _servers));
}

void PostTargetsBox::_fillServersMenu()
{
    QMenu *menu = _serversButton->menu();
    menu->clear();
    QAction *all = menu->addAction(tr("All the enabled servers (page Servers)"));
    all->setCheckable(true);
    all->setChecked(_servers.isEmpty());
    connect(all, &QAction::triggered, this, [this]() {
        _servers.clear();
        _updateServersButton();
        emit changed(SERVERS);
    });
    menu->addSeparator();
    QStringList keys;
    for (NntpServerParams *srv : _ngPost->_nntpServers)
    {
        const QString key = NgPost::serverKey(*srv);
        if (keys.contains(key))
            continue;
        keys << key;
        QAction *a = menu->addAction(srv->enabled ? key : tr("%1 (disabled in the page Servers)").arg(key));
        a->setCheckable(true);
        a->setChecked(_servers.contains(key));
        connect(a, &QAction::triggered, this, [this, key](bool checked) {
            if (checked && !_servers.contains(key))
                _servers << key;
            else if (!checked)
                _servers.removeAll(key);
            _updateServersButton();
            emit changed(SERVERS);
        });
    }
    for (const QString &key : QStringList(_servers))
        if (!keys.contains(key))
        {   // a server of a profile that was removed: it can be unchecked
            QAction *a = menu->addAction(tr("%1 (not in the list)").arg(key));
            a->setCheckable(true);
            a->setChecked(true);
            connect(a, &QAction::triggered, this, [this, key]() {
                _servers.removeAll(key);
                _updateServersButton();
                emit changed(SERVERS);
            });
        }
}
