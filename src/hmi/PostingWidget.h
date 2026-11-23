//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
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
//
//========================================================================

#ifndef POSTINGWIDGET_H
#define POSTINGWIDGET_H

#include <QWidget>
#include "utils/PostProfile.h"
#include "utils/PostChecker.h"
#include "utils/PostRequest.h"
class NgPost;
class NntpFile;
class MainWindow;
class ProfileBar;
class NfoCard;
class PostingJob;
class QLabel;
class QPushButton;
#include <QFileInfoList>
#include "RepostInfo.h"
#include <QJsonObject>

namespace Ui {
class PostingWidget;
}

class PostingWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::PostingWidget *_ui;
    MainWindow        *_hmi;
    NgPost            *_ngPost;
    const uint         _jobNumber;
    bool               _nameFromFile = false; //!< the archive name was taken from the first file (not typed, not random): a profile with a random name may replace it

    // ngPost Deluxe layout (built in code over the widgets of the .ui)
    QLabel            *_filesTitle;
    QLabel            *_releaseInfo;   //!< shown when the list has several items (they make one release)
    QLabel            *_nzbTitle;
    QLabel            *_archiveTitle;
    QLabel            *_advancedTitle;
    QWidget           *_advancedCard;
    QLabel            *_summary;       //!< "12 items, 8.4 GB -> about ... to upload"
    qint64             _sourceBytes;   //!< size of the list (computed when the list changes)
    ProfileBar        *_profileBar;
    NfoCard           *_nfoCard;
    QPushButton       *_advancedToggle;
    static bool        sShowAdvanced; //!< same state for all the Quick Post tabs
    void _buildLayout();
    void _retranslateLayout();
    void _showAdvanced(bool show);
    bool    _fromAuto = false;
    bool    _afterAct = false;
    QString _afterMoveTo;
    bool _checkBeforePosting(const QFileInfoList &files, bool hasFolder); //!< checks (and fixes) before a post started with the button
    bool _applyFix(PostChecker::Code code); //!< false: the user has to continue elsewhere (the post is cancelled)
    void _updateReleaseInfo();
    void _resetForNextPost(); //!< after a post: empty list, new random name and password (as the profile says)

public:
    explicit PostingWidget(NgPost *ngPost, MainWindow *hmi, uint jobNumber);
    ~PostingWidget();

    void init();
    void genNameAndPassword(bool genName, bool genPass, bool doPar2, bool useRarMax);
    //! what to do with the original files once posted (Auto Posting): nothing, delete them, or move them to \a moveTo
    //! the tab was created by Auto Posting: duplicates were already asked in one window, an existing nzb gets a number
    void setFromAutoPosting() { _fromAuto = true; }
    bool isFromAutoPosting() const { return _fromAuto; }
    void setAfterPost(bool act, const QString &moveTo) { _afterAct = act; _afterMoveTo = moveTo; }
    inline uint jobNumber() const;

    void handleDropEvent(QDropEvent *e);
    void handleKeyEvent(QKeyEvent *keyEvent);

    void addPath(const QString &path, int currentNbFiles, int isDir = false);

    inline MainWindow *hmi() const;

    void udatePostingParams();

    void retranslate();

    void setNzbPassword(const QString &pass);
    void setPackingAuto(bool enabled, const QStringList &keys);

    //! creates the post and puts it in the queue (page Posts); returns its id in NgPost::postList(), -1 if nothing was started
    int postFiles(bool updateMainParams);

    bool isEmptyAndIdle() const; //!< nothing in the list
    void applyRequest(const PostRequest &request); //!< files and options of a Guided Post (one release)
    int  startFromRequest(); //!< posts like the button (checks, items already posted...); id of the post, -1 if nothing started



signals:
    void postQueued(int postId); //!< (Quick Post) a post was started: the page Posts shows it

public:
    PostProfile currentProfile() const;            //!< the options of the Archive card (to save them as a profile)
    QString profileName() const;                   //!< profile chosen in the bar (empty = Custom)

public slots:
    void onPostFiles(); //!< for the post button but also can be used by the AutoPostWidget

private slots: // for the HMI

    void onNzbPassToggled(bool checked);
    void onGenNzbPassword();
    void _updateSummary(bool recomputeSize = true);
    void applyProfile(const PostProfile &profile); //!< a profile was chosen


    void onSelectFilesClicked();
    void onSelectFolderClicked();
    void onClearFilesClicked();
    void onCompressCB(bool checked);
    void onGenCompressName();
    void onCompressPathClicked();
    void onNzbFileClicked();
    void onRarPathClicked();


private:
    void _buildFilesList(QFileInfoList &files, bool &hasFolder);
    bool _fileAlreadyInList(const QString &fileName, int currentNbFiles) const;


};

uint PostingWidget::jobNumber() const { return _jobNumber; }

MainWindow *PostingWidget::hmi() const { return _hmi; }

#endif // POSTINGWIDGET_H
