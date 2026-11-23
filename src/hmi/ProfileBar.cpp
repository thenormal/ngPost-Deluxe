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

#include "ProfileBar.h"
#include "NgPost.h"
#include "PostTargetsBox.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>

ProfileBar::ProfileBar(NgPost *ngPost, QWidget *parent) :
    QWidget(parent), _ngPost(ngPost), _busy(false)
{
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(2, 0, 2, 0);
    h->setSpacing(8);
    _label = new QLabel(this);
    _combo = new QComboBox(this);
    _combo->setMinimumWidth(200);
    _saveButton   = new QPushButton(this);
    _deleteButton = new QPushButton(this);
    h->addWidget(_label);
    h->addWidget(_combo);
    h->addWidget(_saveButton);
    h->addWidget(_deleteButton);
    h->addStretch();

    connect(_combo, QOverload<int>::of(&QComboBox::activated), this, &ProfileBar::_onActivated);
    connect(_saveButton,   &QAbstractButton::clicked, this, &ProfileBar::_onSave);
    connect(_deleteButton, &QAbstractButton::clicked, this, &ProfileBar::_onDelete);
    connect(_ngPost, &NgPost::profilesChanged, this, &ProfileBar::_reload);
    _fill(QString());
    retranslate();
}

void ProfileBar::setHandlers(std::function<PostProfile()> current, std::function<void(const PostProfile &)> apply)
{
    _current = current;
    _apply   = apply;
}

void ProfileBar::retranslate()
{
    _label->setText(tr("Profile:"));
    _saveButton->setText(tr("Save as profile..."));
    _saveButton->setToolTip(tr("Save the current options (compression, random name and password, par2, keep the archives) under a name, "
                               "and if you want the name template, the newsgroups, the servers and the NZB folder"));
    _deleteButton->setText(tr("Delete"));
    _deleteButton->setToolTip(tr("Delete the chosen profile"));
    QString sel = _combo->currentIndex() >= 2 ? _combo->currentText() : QString();
    int idx = _combo->currentIndex();
    _fill(sel);
    if (sel.isEmpty() && idx >= 0 && idx < 2)
        _combo->setCurrentIndex(idx);
}

void ProfileBar::_fill(const QString &select)
{
    QSignalBlocker block(_combo);
    int keep = _combo->currentIndex();
    _combo->clear();
    _combo->addItem(tr("Custom"));
    _combo->addItem(tr("Recommended (compression, random name and password, par2 10 %)"));
    for (const PostProfile &p : _ngPost->_profiles)
    {
        _combo->addItem(p.name);
        _combo->setItemData(_combo->count() - 1, _describe(p), Qt::ToolTipRole);
    }
    int idx = select.isEmpty() ? qMax(0, keep) : _combo->findText(select);
    _combo->setCurrentIndex(idx < 0 || idx >= _combo->count() ? 0 : idx);
    _deleteButton->setEnabled(_combo->currentIndex() >= 2);
}

void ProfileBar::_reload()
{
    // another bar added or deleted a profile: keep my choice if it still exists
    QString sel = _combo->currentIndex() >= 2 ? _combo->currentText() : QString();
    int idx = _combo->currentIndex();
    _fill(sel);
    if (sel.isEmpty() && idx < 2)
        _combo->setCurrentIndex(idx);
}

void ProfileBar::markCustom()
{
    if (_busy)
        return;
    QSignalBlocker block(_combo);
    _combo->setCurrentIndex(0);
    _deleteButton->setEnabled(false);
}

void ProfileBar::applyRecommendedIfNew()
{
    if (!_ngPost->_freshInstall)
        return;
    _combo->setCurrentIndex(1);
    _applyProfile(PostProfile::recommended());
}

static bool sameOptions(const PostProfile &a, const PostProfile &b)
{
    return a.compress == b.compress && a.genName == b.genName && a.genPass == b.genPass
            && a.par2 == b.par2 && a.par2Pct == b.par2Pct && a.keepRar == b.keepRar;
}

QString ProfileBar::_describe(const PostProfile &p) const
{
    auto yesNo = [](bool on) { return on ? tr("yes") : tr("no"); };
    QStringList lines;
    lines << tr("Compress: %1, random name: %2, random password: %3").arg(yesNo(p.compress), yesNo(p.genName), yesNo(p.genPass));
    lines << (p.par2 ? tr("par2: %1 %").arg(p.par2Pct) : tr("par2: no"));
    if (p.keepRar)
        lines << tr("Keep the archives");
    if (!p.targets.nameTemplate.isEmpty())
        lines << tr("Name: %1").arg(p.targets.nameTemplate);
    if (!p.targets.groups.isEmpty())
        lines << tr("Newsgroups: %1").arg(p.targets.groups.join(", "));
    if (!p.targets.servers.isEmpty())
        lines << tr("Servers: %1").arg(PostTargetsBox::serversText(_ngPost, p.targets.servers));
    if (!p.targets.nzbDir.isEmpty())
        lines << tr("NZB folder: %1").arg(QDir::toNativeSeparators(p.targets.nzbDir));
    return lines.join("\n");
}

const PostProfile *ProfileBar::selectedProfile() const
{
    int idx = _combo->currentIndex();
    if (idx >= 2 && idx - 2 < _ngPost->_profiles.size())
        return &_ngPost->_profiles.at(idx - 2);
    return nullptr;
}

QString ProfileBar::selectedName() const
{
    int idx = _combo->currentIndex();
    if (idx == 1)
        return QStringLiteral("Recommended");
    if (idx >= 2 && idx - 2 < _ngPost->_profiles.size())
        return _ngPost->_profiles.at(idx - 2).name;
    return QString();
}

void ProfileBar::showMatching(const QString &profileName)
{
    if (!_current)
        return;
    const PostProfile now = _current();
    int idx = 0; // Custom
    // the profile of the last session first, then any profile with the same options
    for (int pass = 0; pass < 2 && idx == 0; ++pass)
        for (int i = 0; i < _ngPost->_profiles.size() && idx == 0; ++i)
            if (sameOptions(_ngPost->_profiles.at(i), now) && (pass == 1 || _ngPost->_profiles.at(i).name == profileName))
                idx = i + 2;
    if (idx == 0 && sameOptions(now, PostProfile::recommended()) && (profileName.isEmpty() || profileName == QLatin1String("Recommended")))
        idx = 1;
    QSignalBlocker block(_combo);
    _combo->setCurrentIndex(idx);
    _deleteButton->setEnabled(idx >= 2);
}

void ProfileBar::restore(const PostProfile &options, const QString &profileName)
{
    // the profile that was chosen when the program was closed (if it still exists): its options are applied
    if (profileName == QLatin1String("Recommended"))
    {
        PostProfile r = PostProfile::recommended();
        select(r);
        return;
    }
    for (const PostProfile &saved : _ngPost->_profiles)
        if (!profileName.isEmpty() && saved.name == profileName)
        {
            select(saved);
            return;
        }
    PostProfile p = options;
    for (const PostProfile &saved : _ngPost->_profiles)
        if (sameOptions(saved, options))
        {
            p.name = saved.name;
            break;
        }
    select(p);
}

void ProfileBar::select(const PostProfile &profile)
{
    int idx = 0; // Custom
    for (int i = 0; i < _ngPost->_profiles.size(); ++i)
        if (_ngPost->_profiles.at(i).name == profile.name && sameOptions(_ngPost->_profiles.at(i), profile))
            idx = i + 2;
    if (idx == 0 && sameOptions(profile, PostProfile::recommended()))
        idx = 1;
    {
        QSignalBlocker block(_combo);
        _combo->setCurrentIndex(idx);
    }
    _deleteButton->setEnabled(idx >= 2);
    _applyProfile(profile);
}

void ProfileBar::_applyProfile(const PostProfile &profile)
{
    if (!_apply)
        return;
    _busy = true;
    _apply(profile);
    _busy = false;
}

void ProfileBar::_onActivated(int index)
{
    _deleteButton->setEnabled(index >= 2);
    if (index == 1)
        _applyProfile(PostProfile::recommended());
    else if (index >= 2 && index - 2 < _ngPost->_profiles.size())
        _applyProfile(_ngPost->_profiles.at(index - 2));
}

void ProfileBar::_onSave()
{
    if (!_current)
        return;
    const PostProfile now = _current();
    const PostTargets &t = now.targets;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Save as profile"));
    QVBoxLayout *v = new QVBoxLayout(&dlg);
    QFormLayout *form = new QFormLayout();
    QLineEdit *nameEdit = new QLineEdit(_combo->currentIndex() >= 2 ? _combo->currentText() : QString(), &dlg);
    nameEdit->setMinimumWidth(320);
    form->addRow(tr("Name of the profile:"), nameEdit);
    v->addLayout(form);
    QLabel *info = new QLabel(tr("The options of the archive are always saved. Also save in the profile:"), &dlg);
    info->setWordWrap(true);
    v->addWidget(info);
    auto addPart = [&dlg, v](const QString &label, const QString &value) {
        QCheckBox *cb = new QCheckBox(value.isEmpty() ? label : QString("%1: %2").arg(label, value), &dlg);
        cb->setEnabled(!value.isEmpty());
        if (value.isEmpty())
            cb->setToolTip(tr("Empty: the profile doesn't change it"));
        v->addWidget(cb);
        return cb;
    };
    QCheckBox *nameCB    = addPart(tr("Name template"), t.nameTemplate);
    QCheckBox *groupsCB  = addPart(tr("Newsgroups"), t.groups.join(", "));
    QCheckBox *serversCB = addPart(tr("Servers"), t.servers.isEmpty() ? QString() : PostTargetsBox::serversText(_ngPost, t.servers));
    QCheckBox *nzbCB     = addPart(tr("NZB folder"), QDir::toNativeSeparators(t.nzbDir));
    QLabel *hint = new QLabel(tr("A part that is not saved is not changed when the profile is chosen."), &dlg);
    hint->setObjectName("hint");
    hint->setWordWrap(true);
    v->addWidget(hint);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    v->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // what is checked: the parts of the profile that is replaced, else what is set (not the NZB folder)
    auto setDefaults = [&]() {
        const QString name = PostProfile::cleanName(nameEdit->text());
        const PostProfile *old = nullptr;
        for (const PostProfile &p : _ngPost->_profiles)
            if (p.name.compare(name, Qt::CaseInsensitive) == 0)
                old = &p;
        nameCB->setChecked(nameCB->isEnabled() && (old ? !old->targets.nameTemplate.isEmpty() : true));
        groupsCB->setChecked(groupsCB->isEnabled() && (old ? !old->targets.groups.isEmpty() : true));
        serversCB->setChecked(serversCB->isEnabled() && (old ? !old->targets.servers.isEmpty() : true));
        nzbCB->setChecked(nzbCB->isEnabled() && old && !old->targets.nzbDir.isEmpty());
    };
    setDefaults();
    connect(nameEdit, &QLineEdit::textChanged, &dlg, setDefaults);

    while (true)
    {
        if (dlg.exec() != QDialog::Accepted)
            return;
        QString name = PostProfile::cleanName(nameEdit->text());
        if (name.isEmpty())
            continue;
        if (name.compare(tr("Custom"), Qt::CaseInsensitive) == 0 || name.compare("Custom", Qt::CaseInsensitive) == 0
                || name.compare("Recommended", Qt::CaseInsensitive) == 0 || name.startsWith("Recommended ", Qt::CaseInsensitive))
        {
            QMessageBox::information(this, tr("Save as profile"), tr("This name is reserved: choose another one."));
            continue;
        }
        PostProfile p = now;
        p.name = name;
        p.targets = PostTargets();
        if (nameCB->isChecked())    p.targets.nameTemplate = t.nameTemplate;
        if (groupsCB->isChecked())  p.targets.groups       = t.groups;
        if (serversCB->isChecked()) p.targets.servers      = t.servers;
        if (nzbCB->isChecked())     p.targets.nzbDir       = t.nzbDir;
        bool replaced = false;
        for (PostProfile &old : _ngPost->_profiles)
            if (old.name.compare(name, Qt::CaseInsensitive) == 0)
            {
                old = p;
                replaced = true;
            }
        if (!replaced)
            _ngPost->_profiles.append(p);
        _fill(name);
        emit _ngPost->profilesChanged();
        return;
    }
}

void ProfileBar::_onDelete()
{
    int idx = _combo->currentIndex();
    if (idx < 2 || idx - 2 >= _ngPost->_profiles.size())
        return;
    QString name = _ngPost->_profiles.at(idx - 2).name;
    if (QMessageBox::question(this, tr("Delete"), tr("Delete the profile \"%1\"?").arg(name)) != QMessageBox::Yes)
        return;
    _ngPost->_profiles.removeAt(idx - 2);
    _combo->setCurrentIndex(0);
    emit _ngPost->profilesChanged();
}
