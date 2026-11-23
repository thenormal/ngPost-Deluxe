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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ProgressInfo.h"
#include <QFileInfoList>
#include <QMap>
#include "utils/PostChecker.h"
#include "utils/PostRequest.h"
class QTimer;
class ServersWidget;
class NgPost;
struct NntpServerParams;
class NntpFile;
class PostingWidget;
class AutoPostWidget;
class QStackedWidget;
class QButtonGroup;
class QPushButton;
class QLabel;
class QComboBox;
class QGridLayout;
class QFrame;
class QLineEdit;
class QCheckBox;
class QDoubleSpinBox;
class QPlainTextEdit;
class QSystemTrayIcon;
class HistoryWidget;
class PostsWidget;
class HelpWidget;
class GuidedPostWidget;
class GroupsEdit;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    enum class STATE {IDLE, POSTING, STOPPING};

    Ui::MainWindow *_ui;
    NgPost         *_ngPost;
    STATE           _state;
    PostingWidget  *_quickJobTab;
    AutoPostWidget *_autoPostTab;
    ServersWidget  *_servers; //!< server section (list + details)

    // autosave: the settings are saved automatically shortly after each change made by the user
    QTimer         *_autoSaveTimer;
    bool            _autoSaveReady;     //!< false during the initialisation (the widgets are filled by the code)
    bool            _autoSaveQuickTab;  //!< a setting of the default Quick Post tab changed
    bool            _autoSaveAutoTab;   //!< a setting of the Auto Posting tab changed
    static constexpr int sAutoSaveDelayMs = 1000;

    // new layout of ngPost Deluxe: side bar + pages
    enum Page {PAGE_POSTS = 0, PAGE_SERVERS, PAGE_SETTINGS, PAGE_HISTORY, PAGE_HELP, PAGE_GUIDED, PAGE_LIST}; //!< PAGE_POSTS: Quick Post and Auto Posting, PAGE_LIST: the page Posts
    enum Nav  {NAV_GUIDED = 0, NAV_QUICK, NAV_AUTO, NAV_LIST, NAV_SERVERS, NAV_SETTINGS, NAV_HISTORY, NAV_HELP, NB_NAV};
    QStackedWidget *_pages;
    QButtonGroup   *_nav;
    QPushButton    *_navButtons[NB_NAV];
    HistoryWidget  *_history;
    HelpWidget     *_help;
    GuidedPostWidget *_guided = nullptr; //!< Guided Post wizard
    PostsWidget    *_posts = nullptr;  //!< page Posts: all the posts of the session
    QString         _jobName;     //!< post shown in the bottom bar (empty: none)
    bool            _pauseIcon;   //!< the pause button shows pause (true) or play
    QCheckBox      *_historyCB;
    QCheckBox      *_duplicatesCB;
    QCheckBox      *_limitSpeedCB = nullptr;   //!< Settings > Upload: limit the upload speed
    QDoubleSpinBox *_maxSpeedSB = nullptr;     //!< the limit in MB/s
    QLabel         *_limitSpeedHint = nullptr;
    QLineEdit      *_historyPathEdit;
    QPushButton    *_historyPathButton;
    QLabel         *_historyHint;
    QCheckBox      *_queueCB;
    QCheckBox      *_notifyOkCB = nullptr;
    QCheckBox      *_notifyErrCB = nullptr;
    QCheckBox      *_notifyTrayCB = nullptr;
    QComboBox      *_notifyWhenCB = nullptr;
    QPushButton    *_settingsCats[5] = {nullptr, nullptr, nullptr, nullptr, nullptr}; //!< categories of the Settings page
    QStackedWidget *_settingsStack = nullptr;
    QFrame         *_firstSteps = nullptr;       //!< banner of the posts page: what to do before the first post
    QLabel         *_firstStepsText = nullptr;
    QPushButton    *_firstStepsButton = nullptr;
    QPushButton    *_updateButton = nullptr;     //!< "Version X available", at the bottom of the sidebar
    QCheckBox      *_updateCheckCB = nullptr;    //!< Settings, General: look for new versions at the start
    QString         _updateVersion, _updateUrl;
    int             _trayShown = 0;              //!< system notifications shown this session (diagnostic report)
    int             _firstStepsAction = -1;      //!< 0: open Servers, 1: open the newsgroups of Settings
    QLabel         *_notifyWhenLbl = nullptr;
    QLabel         *_notifyWhatLbl = nullptr, *_notifyWhereLbl = nullptr; //!< the two groups of the card: what is notified, where
    QLabel         *_notifyDiscordLbl = nullptr, *_notifyTgTokenLbl = nullptr, *_notifyTgChatLbl = nullptr, *_notifyWebhookLbl = nullptr;
    QLineEdit      *_notifyDiscordEdit = nullptr, *_notifyTgTokenEdit = nullptr, *_notifyTgChatEdit = nullptr, *_notifyWebhookEdit = nullptr;
    QPushButton    *_notifyTestButton = nullptr;
    QLabel         *_notifyHint = nullptr;
    QSystemTrayIcon *_tray = nullptr;   //!< created when the first system notification is shown
    QLabel         *_queueHint;
    QLabel         *_supportHint;
    QPushButton    *_supportButton;
    QPushButton    *_themeButton;  //!< side bar: switch Auto / Light / Dark
    QPushButton    *_aboutButton;
    QLabel         *_postsTitle;
    QLabel         *_serversTitle;
    QLabel         *_serversSubtitle;
    QLabel         *_settingsTitle;
    QLabel         *_settingsSubtitle;
    QLabel         *_appVersion;
    GroupsEdit     *_groupsEdit   = nullptr; //!< newsgroups as chips (Settings)
    QComboBox      *_groupPolicyCB = nullptr; //!< all the groups / one per post / one per file
    QComboBox      *_themeCB;
    QLabel         *_themeLbl;
    QLabel         *_autoSaveHint;
    QMap<QString, QLabel*> _cardTitles;




public:
    void saveDiagnosticReport() { onDiagnosticReport(); }  //!< same as the button under the log (used by the Feedback window)
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void init(NgPost *ngPost);

    void updateProgress(const ProgressInfo &info); //!< progress bar, text under it and title of the window

    void updateServers();
    QStringList invalidGroups() const;  //!< the newsgroups that are not valid
    bool hasGroups() const;
    void showServersPage();
    void showGuidedPage();
    ServersWidget *serversWidget() const { return _servers; }
    QStringList groupsList() const;  //!< the newsgroups of the settings
    enum SettingsCategory { CAT_GENERAL, CAT_POST, CAT_NZB, CAT_NOTIFY, CAT_HISTORY, NB_CATS };
    void showSettingsPage(int category = -1); //!< -1: the category that was open
    void updateFirstSteps();                  //!< shows what is missing before a first post (server, newsgroup), hides it when done
    bool checkGroups(); //!< false (and a message in the log) if the newsgroups can't be used to post
    void updateParams();
    void updateAutoPostingParams();

    QString fixedArchivePassword() const;

    //! a Quick Post form that is not shown: Auto Posting and Guided Post use it to prepare and start one post
    PostingWidget *newHiddenQuickPost(const QFileInfoList &files = QFileInfoList());
    void showPostsPage(int postId = -1); //!< page Posts, with \a postId selected (after a post was started)

    /*!
     * \brief starts what the user wants to post (Guided Post): one release in a new Quick Post tab,
     * everything else in Auto Posting. The pages do their usual checks (and fixes), the window of the items
     * already posted and the queue: nothing is posted in another way.
     * \return false when nothing was started (invalid request, checks cancelled, a folder already watched...)
     */
    bool startPost(const PostRequest &request);

    /*!
     * \brief what the checks before posting need (Quick Post, Auto Posting, Guided Post), from the current settings
     * \param nzbFile the nzb file that will be written (empty: none known yet)
     * \param watching a folder is watched: what will arrive is not known yet
     */
    PostChecker::Input checkInput(const QFileInfoList &files, bool hasFolder, const QString &nzbFile,
                                  bool passwordChecked, const QString &password, bool watching = false) const;

    void setJobLabel(const QString &postName); //!< name of the post in the bottom bar (empty: "Ready")

    void log(const QString &aMsg, bool newline = true, int level = 0) const; //!< log function for QString (level: NgPost::LOG_LEVEL)
    void logError(const QString &error) const; //!< log function for QString

    bool useFixedPassword() const;
    bool hasAutoCompress() const;


    inline AutoPostWidget *autoWidget() const;

    void setPauseIcon(bool pause);

    static QColor        sPostingColor;
    static const QString sPostingIcon;
    static QColor        sPendingColor;
    static const QString sPendingIcon;
    static QColor        sDoneOKColor;
    static const QString sDoneOKIcon;
    static QColor        sDoneKOColor;
    static const QString sDoneKOIcon;
    static QColor        sArticlesFailedColor;

    static void setThemeColors(bool dark); //!< status colours readable on the light or the dark theme
    void onThemeApplied(); //!< called by NgPost::applyTheme (icons of the side bar...)
    void onHistoryUpdated(); //!< a post has been added in the history file

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent* event) override;

public slots:
    void onSetProgressBarRange(int nbArticles);


public slots:
    void scheduleAutoSave(); //!< to be called after any change of a setting (the save is delayed and grouped)
    void autoSaveForm(int tab); //!< same for a change of the form of Quick Post (0) or Auto Posting (1)

private slots:
    void onNavClicked(int id);
    void onPostTabChanged(int index);
    void onThemeSelected(int index);
    void onThemeButtonClicked();
    void onHistoryToggled(bool checked);
    void onQueueToggled(bool checked);
    void _startUpdateCheck();   //!< once a day, if the user did not turn it off: is there a newer release?
    void _offerQueue();   //!< at the start: the posts that were not finished (Resume / Discard / Decide later)
    void onHistoryPathClicked();
    void onAutoSave();
    void onDiagnosticReport();
    void _showTrayMessage(const QString &title, const QString &text);

    void onObfucateToggled(bool checked);




    void onGenPoster();
    void onUniqueFromToggled(bool checked);
    void onRarPassToggled(bool checked);
    void onRarPassUpdated(const QString &fixedPass);
    void onArchivePass();
    void onAutoCompressToggled(bool checked);

    void onDebugToggled(bool checked);
    void onDebugValue(int value);

    void onSaveConfig();


    void toBeImplemented();

    void onNzbPathClicked();

    void onLangChanged(const QString &lang);

    void onShutdownToggled(bool checked);

    void onPauseClicked();

private:
    void _watchForAutoSave(QWidget *root, int tab); //!< tab: -1 global settings, 0 Quick Post, 1 Auto Posting
    void _saveConfig(bool quickTab, bool autoTab);
    QString _diagnosticReport() const;

    void _buildShell();          //!< side bar + pages (replaces the splitters of the original window)
    QWidget *_buildSettingsPage();
    QFrame *_card(const QString &key, QGridLayout *&grid);
    void _retranslateShell();
    void _fixIconButtons();
    void _adaptIcons();
    void _adaptTabIcons();
    QColor _tabNormalColor() const;
    QIcon  _tabIcon(const QString &path) const;
    void _retranslateGroups();
    void _updateThemeButton();
    void _updateHistorySettings();
    static QIcon _navIcon(const QString &path, const QColor &normal, const QColor &checked);

    void _initServerBox();
    void _initPostingBox();



};

AutoPostWidget *MainWindow::autoWidget() const { return _autoPostTab; }

#endif // MAINWINDOW_H
