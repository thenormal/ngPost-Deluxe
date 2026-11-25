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

#ifndef AUTOPOSTWIDGET_H
#define AUTOPOSTWIDGET_H

#include <QWidget>
#include <QHash>
#include <QFileInfoList>
#include <QStringList>
#include "utils/PostProfile.h"
#include "utils/PostChecker.h"
#include "utils/PostRequest.h"
class NgPost;
class MainWindow;
class ProfileBar;
class PostTargetsBox;
class NfoCard;
class QFileInfo;


namespace Ui {
class AutoPostWidget;
}
class QLabel;
class QPushButton;
class QFrame;
class QComboBox;
class QLineEdit;
class QTimer;

class AutoPostWidget : public QWidget
{
    Q_OBJECT
private:
    QHash<int, QString> _itemOfPost; //!< id of a post (NgPost::postList) => item of the list that it posts
    Ui::AutoPostWidget *_ui;
    MainWindow         *_hmi;
    NgPost             *_ngPost;
    bool                _isMonitoring;

    // ngPost Deluxe layout (built in code over the widgets of the .ui)
    QLabel             *_filesTitle;
    QLabel             *_sourceTitle;
    QLabel             *_archiveTitle;
    QLabel             *_advancedTitle;
    QLabel             *_monitorTitle;
    QWidget            *_advancedCard;
    ProfileBar         *_profileBar;
    PostTargetsBox     *_targets = nullptr; //!< name template, newsgroups, servers and NZB folder of the posts
    QLabel             *_targetsTitle = nullptr;
    NfoCard            *_nfoCard;
    QPushButton        *_advancedToggle;
    bool                _showAdvanced;
    // the three ways of posting
    enum class Mode { Choose, Scan, Watch };
    Mode                _mode;
    QPushButton        *_modeButtons[3];
    QFrame             *_sourceCard;
    QFrame             *_monitorCard;
    QLabel             *_summary;
    // after posting: nothing / move to "Posted" / delete
    QLabel             *_afterTitle;
    QComboBox          *_afterCombo;
    QLineEdit          *_postedEdit;
    QPushButton        *_postedButton;
    QWidget            *_afterCard;
    QTimer             *_sizeTimer;
    QFrame             *_watchBar = nullptr;     //!< state of the watching, visible while watching
    QLabel             *_watchLabel = nullptr;
    QPushButton        *_watchPauseButton = nullptr;
    QTimer             *_watchTimer = nullptr;
    QStringList         _watchedFolders;
    static constexpr int sSizeRole = Qt::UserRole + 2; //!< size in bytes of a list item (-1: not computed yet)
    void _setMode(Mode mode);
    void _applyMode();
    void _decorateItems();           //!< check box + size on the new items of the list
    void _computeNextSize();
    void _updateSummary();
    void _updateWatchBar();
    void _rememberWatchedFolders(); //!< keeps the list in the config (the next start offers to watch them again)
    void _onWatchPause();
    QFileInfoList _checkedFiles(bool *hasFolder = nullptr) const;
    bool _checkBeforeStart(const QFileInfoList &files, bool hasFolder);
    //! posts each file/folder in its own Quick Post tab (checks, items already posted...); false if nothing was created
    bool _postFiles(QFileInfoList files, bool hasFolder, bool forceStart);
    void _applyRequestOptions(const PostRequest &request); //!< profile and what happens after the post
    void _afterPostChanged(bool warnDelete);
    bool _applyFix(PostChecker::Code code);
    void _buildLayout();
    void _retranslateLayout();
    int                 _currentPostIdx;
protected:
    void showEvent(QShowEvent *event) override;
private:

    static constexpr int sCompletedRole = Qt::UserRole + 1; //!< set on the items uploaded without error

public:
    explicit AutoPostWidget(NgPost *ngPost, MainWindow *hmi);
    ~AutoPostWidget();

    void init();

    void handleKeyEvent(QKeyEvent *keyEvent);
    void handleDropEvent(QDropEvent *e);


    void udatePostingParams();
    PostProfile currentProfile() const;            //!< the options of the Archive card (to save them as a profile)
    QString profileName() const;                   //!< profile chosen in the bar (empty = Custom)
    void applyProfile(const PostProfile &profile); //!< a profile was chosen

    //! Guided Post: one post for each item, scan of a folder or watch of a folder; false if nothing started
    bool postRequest(const PostRequest &request);

    void newFileToProcess(const QFileInfo &fileInfo);
    void updateFinishedJob(const QString &path, uint nbArticles, uint nbUploaded, uint nbFailed);

    bool deleteFilesOncePosted() const;
    QString movePostedTo() const;     //!< folder "Posted" (empty: the files are not moved)

    void retranslate();

    void offerResumeWatching(); //!< at the start: the folders that were watched when the program was closed (Watch again / Forget / Decide later)

    void setPackingAuto(bool enabled, const QStringList &keys);

public slots:
    void onMonitorJobStart();

private slots:
    void onPostChanged(int postId); //!< a post of the list is finished: its item is colored
    void onRemoveCompleted();
    void onRemoveAll();
    void onSelectFilesClicked();
    void onGenQuickPosts();
    void onCompressPathClicked();
    void onRarPathClicked();
    void onSelectAutoDirClicked();
    void onScanAutoDirClicked();
    void onMonitoringClicked();
    void onAfterPostChanged();
    void onPostedFolderClicked();
    void onAddMonitoringFolder();
    void onCompressToggled(bool checked);
    void onGenPar2Toggled(bool checked);
};

#endif // AUTOPOSTWIDGET_H
