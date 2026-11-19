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

#ifndef GUIDEDPOSTWIDGET_H
#define GUIDEDPOSTWIDGET_H

#include <QWidget>
#include <QList>
#include <QFileInfoList>
#include "utils/PostRequest.h"
#include "utils/PostChecker.h"

class NgPost;
class MainWindow;
class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;
class QRadioButton;
class QButtonGroup;
class QListWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QFrame;
class QVBoxLayout;
class QShowEvent;
class QKeyEvent;

/*!
 * \brief "Guided Post" page: a wizard in 5 steps (what to do, what to post, options, where to post, summary).
 * It only collects the choices in a PostRequest and gives them to MainWindow::startPost():
 * the posts are made by Quick Post and Auto Posting, with their checks, duplicates window and queue.
 */
class GuidedPostWidget : public QWidget
{
    Q_OBJECT
public:
    enum Step { STEP_WHAT = 0, STEP_FILES, STEP_OPTIONS, STEP_WHERE, STEP_SUMMARY, NB_STEPS };

    GuidedPostWidget(NgPost *ngPost, MainWindow *hmi, QWidget *parent = nullptr);

    void retranslate();
    void handleDropEvent(class QDropEvent *e);
    int  currentStep() const;

    //! true when "Where to post" can be skipped: a server not known as failing, valid newsgroups, a writable NZB folder
    bool whereIsReady(QString *problem = nullptr) const;

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void _onBack();
    void _onNext();
    void _onCancel();
    void _onAddAnother();
    void _onPost();

private:
    NgPost     *_ngPost;
    MainWindow *_hmi;

    // state
    PostRequest          _req;           //!< the post being prepared
    QList<PostRequest>   _ready;         //!< the posts already prepared ("Add another post")
    int                  _editing;       //!< index in _ready of the post being edited (-1: a new one)
    int                  _step;
    bool                 _loading;       //!< the widgets are filled by the code

    // frame
    QLabel         *_title, *_subtitle;
    QLabel         *_stepLabel, *_stepTitle, *_stepHint;
    QProgressBar   *_progress;
    QStackedWidget *_pages;
    QPushButton    *_backButton, *_nextButton, *_cancelButton;

    // step 1
    QRadioButton   *_kindButtons[4];
    QLabel         *_kindHints[4];
    QButtonGroup   *_kindGroup;
    QLabel         *_reuseHint;

    // step 2
    QStackedWidget *_filesStack;
    QLabel         *_filesLabel;
    QPushButton    *_selectFiles, *_selectFolder, *_removeAll;
    QListWidget    *_fileList;
    QLabel         *_fileSummary;
    QLabel         *_folderLabel;
    QLineEdit      *_folderEdit;
    QPushButton    *_folderBrowse;
    QListWidget    *_folderPreview;
    QLabel         *_folderSummary;

    // step 3
    QLabel         *_profileLabel;
    QComboBox      *_profileCombo;
    QLabel         *_profileDesc;
    QPushButton    *_customize;
    QFrame         *_customCard;
    QCheckBox      *_compressCB, *_genNameCB, *_genPassCB, *_par2CB, *_keepCB;
    QSpinBox       *_par2Pct;
    QFrame         *_nfoCard;
    QCheckBox      *_nfoCB, *_nfoInPostCB, *_nfoPassCB, *_nfoMediaCB;
    QFrame         *_afterCard;
    QLabel         *_afterLabel;
    QComboBox      *_afterCombo;
    QLineEdit      *_moveToEdit;

    // step 4
    QLabel         *_serversTitle, *_groupsTitle, *_nzbTitle;
    QLabel         *_serversText, *_groupsText, *_nzbText;
    QPushButton    *_serversButton, *_groupsButton, *_nzbButton;
    QLabel         *_whereState;
    QCheckBox      *_skipWhereCB;

    // step 5
    QWidget        *_readyBox = nullptr;
    QLabel         *_readyTitle, *_thisTitle, *_thisText, *_checksTitle;
    QListWidget    *_readyList;
    QPushButton    *_editReady, *_removeReady;
    QVBoxLayout    *_checksLayout;
    QWidget        *_checksBox;
    QCheckBox      *_saveProfileCB;
    QLineEdit      *_profileName;
    QPushButton    *_addAnother, *_postButton;

    QList<PostChecker::Issue> _issues;

    void _build();
    QWidget *_buildStepWhat();
    QWidget *_buildStepFiles();
    QWidget *_buildStepOptions();
    QWidget *_buildStepWhere();
    QWidget *_buildStepSummary();

    void _goTo(int step);
    void _applyStepTexts();
    bool _canGoNext(QString *why = nullptr) const;
    void _updateButtons();
    int  _nextStep() const;      //!< the step after the current one (the step 4 can be skipped)
    int  _previousStep() const;

    // between the widgets and _req
    void _fillFromRequest();
    void _readStep(int step);
    void _readOptions();
    void _loadProfiles(const QString &select = QString());
    void _profileChosen(int index);
    void _optionsEdited();
    void _updateProfileDesc();
    void _setCustomVisible(bool visible);
    void _updateOptionsEnabled();
    void _showFilesPage();

    // step 2
    void _addPath(const QString &path);
    void _updateFileSummary();
    void _loadFolderPreview();
    QFileInfoList _requestFiles(const PostRequest &r, bool *hasFolder = nullptr) const;

    // step 4 / 5
    void _refreshWhere();
    void _refreshSummary();
    void _showChecks(const PostRequest &r);
    QList<PostChecker::Issue> _checksFor(const PostRequest &r) const;
    bool _hasBlockingIssue() const;
    void _applyFix(PostChecker::Code code);
    QString _describe(const PostRequest &r) const;
    static QString _describeProfile(const PostProfile &p);
    QString _kindName(PostRequest::Kind kind) const;

    bool _commitCurrent();       //!< current request in the list of the posts ready; false if it is not valid
    void _saveChoices();
    void _reset(bool keepChoices);
    void _remember(const PostRequest &r);
};

#endif // GUIDEDPOSTWIDGET_H
