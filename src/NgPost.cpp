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

#include "NgPost.h"
#include "utils/SpeedLimiter.h"
#include "utils/PostedMover.h"
#include <QSaveFile>
#include "utils/Diagnostics.h"
#include "PostingJob.h"
#include "PostQueue.h"
#include "utils/PostList.h"
#include "FoldersMonitorForNewFiles.h"
#include "nntp/NntpArticle.h"
#include "nntp/NntpServerParams.h"
#include "FileUploader.h"
#include "NzbCheck.h"
#ifdef __USE_HMI__
  #include "hmi/MainWindow.h"
  #include "hmi/PostingWidget.h"
  #include "hmi/AutoPostWidget.h"
#endif

#include <cmath>
#ifdef __USE_HMI__
  #include <QApplication>
  #include <QMessageBox>
  #include <QPalette>
  #include <QFile>
  #include <QSettings>
  #include <QStyle>
#else
  #include <QCoreApplication>
#endif
#include <QThread>
#include <QSysInfo>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  #include <QRandomGenerator>
#endif
#include <QTextStream>
#include <QCommandLineParser>
#include <QDebug>
#include <iostream>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QDir>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QStandardPaths>
#include <QNetworkProxy>
#ifdef __USE_TMP_RAM__
  #include <QStorageInfo>
#endif

const char *NgPost::sAppName          = "ngPost Deluxe";
const QString NgPost::sVersion        = QString(QT_STRINGIFY(APP_VERSION)).remove('"');
const QString NgPost::sStage          = "";
const QString NgPost::sSupportURL     = "https://ko-fi.com/thenormal";

const QString NgPost::sMainThreadName     = "MainThread";
const char *NgPost::sFolderMonitoringName = QT_TRANSLATE_NOOP("NgPost", "Auto Posting");
const char *NgPost::sQuickJobName         = QT_TRANSLATE_NOOP("NgPost", "Quick Post");

std::string NgPost::sArticleIdSignature   = sDefaultMsgIdSignature;
const std::string NgPost::sRandomAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

const QStringList NgPost::sDefaultGroups  = {"alt.binaries.test", "alt.binaries.misc" };

qint64        NgPost::sArticleSize = sDefaultArticleSize;
const QString NgPost::sSpace       = sDefaultSpace;

const QMap<NgPost::Opt, QString> NgPost::sOptionNames =
{
    {Opt::PROXY_SOCKS5,   "proxy_socks5"},
    {Opt::HELP,           "help"},
    {Opt::LANG,           "lang"},
    {Opt::VERSION,        "version"},
    {Opt::CONF,           "conf"},
    {Opt::SHUTDOWN_CMD,   "shutdown_cmd"},
    {Opt::DISP_PROGRESS,  "disp_progress"},
    {Opt::DEBUG,          "debug"},
    {Opt::DEBUG_FULL,     "fulldebug"},
    {Opt::POST_HISTORY,   "post_history"},
    {Opt::FIELD_SEPARATOR,"field_separator"},
    {Opt::NZB_UPLOAD_URL, "nzb_upload_url"},
    {Opt::NZB_POST_CMD,   "nzb_post_cmd"},
    {Opt::NZB_RM_ACCENTS, "nzb_rm_accents"},
    {Opt::AUTO_CLOSE_TABS,"auto_close_tabs"},
    {Opt::RAR_NO_ROOT_FOLDER, "rar_no_root_folder"},
    {Opt::LOG_IN_FILE,     "log_in_file"},

    {Opt::NO_RESUME_AUTO,  "no_resume_auto"},
    {Opt::RESUME_WAIT,     "resume_wait"},
    {Opt::SOCK_TIMEOUT,    "sock_timeout"},
    {Opt::PREPARE_PACKING, "prepare_packing"},
    {Opt::CHECK,           "check"},
    {Opt::QUIET,           "quiet"},


    {Opt::INPUT,        "input"},
    {Opt::AUTO_DIR,     "auto"},
    {Opt::MONITOR_DIR,  "monitor"},
    {Opt::DEL_AUTO,     "rm_posted"},
    {Opt::MOVE_POSTED,  "move_posted"},
    {Opt::EXPORT_SETTINGS, "export_settings"},
    {Opt::IMPORT_SETTINGS, "import_settings"},
    {Opt::OUTPUT,       "output"},
    {Opt::NZB_PATH,     "nzbpath"},
    {Opt::THREAD,       "thread"},

    {Opt::MONITOR_FOLDERS,    "monitor_nzb_folders"},
    {Opt::MONITOR_EXT,        "monitor_extensions"},
    {Opt::MONITOR_IGNORE_DIR, "monitor_ignore_dir"},
    {Opt::MONITOR_SEC_DELAY_SCAN, "monitor_sec_delay_scan"},

    {Opt::MSG_ID,       "msg_id"},
    {Opt::META,         "meta"},
    {Opt::ARTICLE_SIZE, "article_size"},
    {Opt::FROM,         "from"},
    {Opt::GROUPS,       "groups"},
    {Opt::NB_RETRY,     "retry"},
    {Opt::GEN_FROM,     "gen_from"},

    {Opt::OBFUSCATE,    "obfuscate"},
    {Opt::INPUT_DIR,    "inputdir"},
    {Opt::GROUP_POLICY, "group_policy"},

    {Opt::TMP_DIR,      "tmp_dir"},
    {Opt::RAR_PATH,     "rar_path"},
    {Opt::RAR_EXTRA,    "rar_extra"},
    {Opt::RAR_SIZE,     "rar_size"},
    {Opt::RAR_MAX,      "rar_max"},
    {Opt::KEEP_RAR,     "keep_rar"},
  #ifdef __USE_TMP_RAM__
    {Opt::TMP_RAM,      "tmp_ram"},
    {Opt::TMP_RAM_RATIO,"tmp_ram_ratio"},
  #endif

    {Opt::PAR2_PCT,     "par2_pct"},
    {Opt::PAR2_PATH,    "par2_path"},
    {Opt::PAR2_ARGS,    "par2_args"},

    {Opt::AUTO_COMPRESS,"auto_compress"},
    {Opt::PACK,         "pack"},
    {Opt::COMPRESS,     "compress"},
    {Opt::GEN_PAR2,     "gen_par2"},
    {Opt::GEN_NAME,     "gen_name"},
    {Opt::GEN_PASS,     "gen_pass"},
    {Opt::RAR_NAME,     "rar_name"},
    {Opt::RAR_PASS,     "rar_pass"},
    {Opt::LENGTH_NAME,  "length_name"},
    {Opt::LENGTH_PASS,  "length_pass"},

    {Opt::SERVER,       "server"},
    {Opt::HOST,         "host"},
    {Opt::PORT,         "port"},
    {Opt::SSL,          "ssl"},
    {Opt::USER,         "user"},
    {Opt::PASS,         "pass"},
    {Opt::CONNECTION,   "connection"},
    {Opt::ENABLED,      "enabled"},
    {Opt::NZBCHECK,     "nzbcheck"},
};

const QList<QCommandLineOption> NgPost::sCmdOptions = {
    { sOptionNames[Opt::HELP],                tr("Help: display syntax")},
    {{"v", sOptionNames[Opt::VERSION]},       tr( "app version")},
    {{"c", sOptionNames[Opt::CONF]},          tr( "use configuration file (if not provided, we try to load $HOME/.ngPost)"), sOptionNames[Opt::CONF]},
    { sOptionNames[Opt::DISP_PROGRESS],       tr( "display cmd progressbar: NONE (default), BAR or FILES"), sOptionNames[Opt::DISP_PROGRESS]},
    {{"d", sOptionNames[Opt::DEBUG]},         tr( "display extra information")},
    { sOptionNames[Opt::DEBUG_FULL],          tr( "display full debug information")},
    {{"l", sOptionNames[Opt::LANG]},          tr( "application language"), sOptionNames[Opt::LANG]},

    { sOptionNames[Opt::CHECK],               tr( "check nzb file (if articles are available on Usenet) cf https://github.com/mbruel/nzbCheck"), sOptionNames[Opt::CHECK]},
    { {"q", sOptionNames[Opt::QUIET]},        tr( "quiet mode (no output on stdout)")},

// automated posting (scanning and/or monitoring)
    { sOptionNames[Opt::AUTO_DIR],            tr("parse directory and post every file/folder separately. You must use --compress, should add --gen_par2, --gen_name and --gen_pass"), sOptionNames[Opt::AUTO_DIR]},
    { sOptionNames[Opt::MONITOR_DIR],         tr("monitor directory and post every new file/folder. You must use --compress, should add --gen_par2, --gen_name and --gen_pass"), sOptionNames[Opt::MONITOR_DIR]},
    { sOptionNames[Opt::DEL_AUTO],            tr("delete file/folder once posted. You must use --auto or --monitor with this option.")},
    { sOptionNames[Opt::MOVE_POSTED],         tr("move the file/folder to this folder once posted (a name like Posted is created next to each file/folder, a full path is used as it is). You must use --auto or --monitor with this option."), sOptionNames[Opt::MOVE_POSTED]},
    { sOptionNames[Opt::EXPORT_SETTINGS],     tr("write the settings, the profiles, the notifications and the servers (without passwords) in this file, to use them on another computer, then exit"), "file"},
    { sOptionNames[Opt::IMPORT_SETTINGS],     tr("read a file written by --export_settings and add it to the config (the old config is kept as .before-import), then exit. Passwords are never in the file: new servers are switched off until you type their password"), "file"},
    { "resume_queue",                         tr("post again the posts that were not finished (queue saved next to the config file)")},
    { "notify_discord",                       tr("send a notification to this Discord webhook address when the posts are done"), "notify_discord"},
    { "notify_telegram_token",                tr("token of the Telegram bot that sends the notifications (with --notify_telegram_chat)"), "notify_telegram_token"},
    { "notify_telegram_chat",                 tr("Telegram chat id that receives the notifications"), "notify_telegram_chat"},
    { "notify_webhook",                       tr("send a JSON POST {\"title\", \"text\"} to this address when the posts are done"), "notify_webhook"},
    { "notify_slack", tr("send a notification to this Slack incoming webhook address when the posts are done"), "notify_slack"},
    { "notify_ntfy", tr("send a notification to this ntfy address (for example https://ntfy.sh/mytopic) when the posts are done"), "notify_ntfy"},
    { "notify_ntfy_token", tr("access token of ntfy (only if the topic needs it)"), "notify_ntfy_token"},
    { "notify_gotify_url", tr("address of the Gotify server that sends the notifications (with --notify_gotify_token)"), "notify_gotify_url"},
    { "notify_gotify_token", tr("token of the Gotify application"), "notify_gotify_token"},
    { "notify_pushover_token", tr("token of the Pushover application (with --notify_pushover_user)"), "notify_pushover_token"},
    { "notify_pushover_user", tr("user or group key of Pushover"), "notify_pushover_user"},
    { "notify_email_host", tr("SMTP server that sends the notifications by email (with --notify_email_from or user, and --notify_email_to)"), "notify_email_host"},
    { "notify_email_port", tr("port of the SMTP server (587 by default; 465 for ssl)"), "notify_email_port"},
    { "notify_email_security", tr("starttls (default), ssl or none"), "notify_email_security"},
    { "notify_email_user", tr("user of the SMTP server (empty: no login)"), "notify_email_user"},
    { "notify_email_pass", tr("password of the SMTP server"), "notify_email_pass"},
    { "notify_email_from", tr("address of the sender (default: the user, if it is an address)"), "notify_email_from"},
    { "notify_email_to", tr("address that receives the notifications (several: separated by a comma)"), "notify_email_to"},
    { "notify_when",                          tr("notify \"each\" (after each post) or \"all_done\" (once, when all the posts are done, default)"), "notify_when"},
    { "gen_nfo",                              tr("write an NFO file (name, date, files, archive, par2) next to the nzb")},
    { "nfo_in_post",                          tr("also post the NFO file with the files (implies --gen_nfo)")},
    { "nfo_password",                         tr("write the password of the archive in the NFO file")},
    { "no_nfo_media",                         tr("don't write the media information (MediaInfo) in the NFO file")},
    { "nfo_style",                            tr("look of the NFO file: classic, clean, box, scene (default), elegant or retro"), "nfo_style"},
    { "nfo_ascii",                            tr("only ASCII characters in the NFO file (for old systems and viewers)")},
    { "mediainfo_path",                       tr("path of the MediaInfo program (default: automatic)"), "mediainfo_path"},
    { "nfo_text",                             tr("text file whose content is added at the end of the NFO file"), "nfo_text"},
    { "allow_repost",                         tr("post again the files that are already in the history (by default they are skipped)")},
    { "max_speed",                            tr("limit the upload speed to this number of MB/s for all the connections (0: no limit)"), "MB/s"},

// quick posting (several files/folders)
    {{"i", sOptionNames[Opt::INPUT]},         tr("input file to upload (single file or directory), you can use it multiple times"), sOptionNames[Opt::INPUT]},
    {{"o", sOptionNames[Opt::OUTPUT]},        tr("output file path (nzb)"), sOptionNames[Opt::OUTPUT]},   

// general options
    {{"x", sOptionNames[Opt::OBFUSCATE]},     tr("obfuscate the subjects of the articles (CAREFUL you won't find your post if you lose the nzb file)")},
    {{"g", sOptionNames[Opt::GROUPS]},        tr("newsgroups where to post the files (coma separated without space)"), sOptionNames[Opt::GROUPS]},
    {{"m", sOptionNames[Opt::META]},          tr("extra meta data in header (typically \"password=qwerty42\")"), sOptionNames[Opt::META]},
    {{"f", sOptionNames[Opt::FROM]},          tr("poster email (random one if not provided)"), sOptionNames[Opt::FROM]},
    {{"a", sOptionNames[Opt::ARTICLE_SIZE]},  tr("article size (default one: %1)").arg(sDefaultArticleSize), sOptionNames[Opt::ARTICLE_SIZE]},
    {{"z", sOptionNames[Opt::MSG_ID]},        tr("msg id signature, after the @ (default one: %1)").arg(sDefaultMsgIdSignature), sOptionNames[Opt::MSG_ID]},
    {{"r", sOptionNames[Opt::NB_RETRY]},      tr("number of time we retry to an Article that failed (default: %1)").arg(NntpArticle::nbMaxTrySending()), sOptionNames[Opt::NB_RETRY]},
    {{"t", sOptionNames[Opt::THREAD]},        tr("number of Threads (the connections will be distributed amongs them)"), sOptionNames[Opt::THREAD]},
    { sOptionNames[Opt::GEN_FROM],            tr("generate a new random email for each Post (--auto or --monitor)")},


// for compression and par2 support
    { sOptionNames[Opt::TMP_DIR],             tr( "temporary folder where the compressed files and par2 will be stored"), sOptionNames[Opt::TMP_DIR]},
    { sOptionNames[Opt::RAR_PATH],            tr( "RAR absolute file path (external application)"), sOptionNames[Opt::RAR_PATH]},
    { sOptionNames[Opt::RAR_SIZE],            tr( "size in MB of the RAR volumes (0 by default meaning NO split)"), sOptionNames[Opt::RAR_SIZE]},
    { sOptionNames[Opt::RAR_MAX],             tr( "maximum number of archive volumes"), sOptionNames[Opt::RAR_MAX]},
    { sOptionNames[Opt::PAR2_PCT],            tr( "par2 redundancy percentage (0 by default meaning NO par2 generation)"), sOptionNames[Opt::PAR2_PCT]},
    { sOptionNames[Opt::PAR2_PATH],           tr( "par2 absolute file path (in case of self compilation of ngPost)"), sOptionNames[Opt::PAR2_PCT]},

    { sOptionNames[Opt::PACK],                tr( "Pack posts using config PACK definition with a subset of (COMPRESS, GEN_NAME, GEN_PASS, GEN_PAR2)")},
    { sOptionNames[Opt::COMPRESS],            tr( "compress inputs using RAR or 7z")},
    { sOptionNames[Opt::GEN_PAR2],            tr( "generate par2 (to be used with --compress)")},
    { sOptionNames[Opt::RAR_NAME],            tr( "provide the RAR file name (to be used with --compress)"), sOptionNames[Opt::RAR_NAME]},
    { sOptionNames[Opt::RAR_PASS],            tr( "provide the RAR password (to be used with --compress)"), sOptionNames[Opt::RAR_PASS]},
    { sOptionNames[Opt::GEN_NAME],            tr( "generate random RAR name (to be used with --compress)")},
    { sOptionNames[Opt::GEN_PASS],            tr( "generate random RAR password (to be used with --compress)")},
    { sOptionNames[Opt::LENGTH_NAME],         tr( "length of the random RAR name (to be used with --gen_name), default: %1").arg(sDefaultLengthName), sOptionNames[Opt::LENGTH_NAME]},
    { sOptionNames[Opt::LENGTH_PASS],         tr( "length of the random RAR password (to be used with --gen_pass), default: %1").arg(sDefaultLengthPass), sOptionNames[Opt::LENGTH_PASS]},
    { sOptionNames[Opt::RAR_NO_ROOT_FOLDER],  tr( "Remove root (parent) folder when compressing Folders using RAR")},


    {{"S", sOptionNames[Opt::SERVER]},        tr("NNTP server following the format (<user>:<pass>@@@)?<host>:<port>:<nbCons>:(no)?ssl"), sOptionNames[Opt::SERVER]},
// without config file, you can provide all the parameters to connect to ONE SINGLE server
    {{"h", sOptionNames[Opt::HOST]},          tr("NNTP server hostname (or IP)"), sOptionNames[Opt::HOST]},
    {{"P", sOptionNames[Opt::PORT]},          tr("NNTP server port"), sOptionNames[Opt::PORT]},
    {{"s", sOptionNames[Opt::SSL]},           tr("use SSL")},
    {{"u", sOptionNames[Opt::USER]},          tr("NNTP server username"), sOptionNames[Opt::USER]},
    {{"p", sOptionNames[Opt::PASS]},          tr("NNTP server password"), sOptionNames[Opt::PASS]},
    {{"n", sOptionNames[Opt::CONNECTION]},    tr("number of NNTP connections"), sOptionNames[Opt::CONNECTION]},
};

const QMap<NgPost::PostCmdPlaceHolders, QString> NgPost::sPostCmdPlaceHolders = {
    {PostCmdPlaceHolders::nzbPath,          "__nzbPath__"},
    {PostCmdPlaceHolders::nzbName,          "__nzbName__"},
    {PostCmdPlaceHolders::rarName,          "__rarName__"},
    {PostCmdPlaceHolders::rarPass,          "__rarPass__"},
    {PostCmdPlaceHolders::nbArticles,       "__nbArticles__"},
    {PostCmdPlaceHolders::nbArticlesFailed, "__nbArticlesFailed__"},
    {PostCmdPlaceHolders::sizeInByte,       "__sizeInByte__"},
    {PostCmdPlaceHolders::nbFiles,          "__nbFiles__"},
    {PostCmdPlaceHolders::groups,           "__groups__"}
};

const QMap<NgPost::GROUP_POLICY, QString> NgPost::sGroupPolicies = {
    {GROUP_POLICY::ALL,       "all"},
    {GROUP_POLICY::EACH_POST, "each_post"},
    {GROUP_POLICY::EACH_FILE, "each_file"}
};

#if __DEBUG__ && QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include <QNetworkConfigurationManager>
#endif
NgPost::NgPost(int &argc, char *argv[]):
    QObject (), CmdOrGuiApp(argc, argv),
    _cout(stdout),
    _cerr(stderr),
    _err(ERROR_CODE::NONE),
#ifdef __DEBUG__
    _debug(2),
#else
    _debug(0),
#endif
    _dispProgressBar(false),
    _dispFilesPosting(false),
    _nzbName(),
    _nbFiles(0),
    _nntpServers(),
    _obfuscateArticles(false), _obfuscateFileName(false),
    _genFrom(false), _saveFrom(false), _from(),
    _meta(), _grpList(sDefaultGroups), _nbGroups(sDefaultGroups.size()),
    _nbThreads(QThread::idealThreadCount()),
    _socketTimeOut(sDefaultSocketTimeOut), _nzbPath(sDefaultNzbPath), _nzbPathConf(sDefaultNzbPath),
    _progressbarTimer(), _refreshRate(sDefaultRefreshRate),
  #ifdef __USE_TMP_RAM__
    _storage(nullptr), _ramPath(), _ramRatio(sRamRatioMin),
  #endif
    _tmpPath(), _rarPath(), _rarArgs(), _rarSize(0), _rarMax(sDefaultRarMax), _useRarMax(false),
    _par2Pct(10), _par2Path(), _par2Args(), _par2PathConfig(),
    _doCompress(false), _doPar2(false), _genName(), _genPass(),
    _lengthName(sDefaultLengthName), _lengthPass(sDefaultLengthPass),
    _rarName(), _rarPass(), _rarPassFixed(),
    _inputDir(),
    _activeJob(nullptr), _pendingJobs(), _packingJob(nullptr),
    _postList(new PostList(this)), _postListTimer(new QTimer(this)),
    _historyFieldSeparator(sDefaultFieldSeparator),
    _postHistoryFile(), _historyEnabled(true), _historyKeyInConfig(false), _configPath(),
    _autoDirs(),
    _folderMonitor(nullptr), _monitorThread(nullptr),
    _delAuto(false),
    _monitor_nzb_folders(false), _monitorExtensions(), _monitorIgnoreDir(false), _monitorSecDelayScan(1),
    _keepRar(false), _packAuto(false), _packAutoKeywords(),
    _lang("en"), _theme("auto"), _darkTheme(false), _translators(),
    _netMgr(), _urlNzbUpload(nullptr), _urlNzbUploadStr(),
    _doShutdownWhenDone(false), _shutdownProc(nullptr),
#if defined(WIN32) || defined(__MINGW64__)
    _shutdownCmd(sDefaultShutdownCmdWindows),
#elif defined(__APPLE__)|| defined(__MACH__)
    _shutdownCmd(sDefaultShutdownCmdMacOS),
#else
    _shutdownCmd(sDefaultShutdownCmdLinux),
#endif
    _removeAccentsOnNzbFileName(false),
    _autoCloseTabs(false),
    _rarNoRootFolder(false),
    _tryResumePostWhenConnectionLost(true),
    _waitDurationBeforeAutoResume(sDefaultResumeWaitInSec),
    _nzbPostCmd(), _preparePacking(false),
    _groupPolicy(GROUP_POLICY::ALL),
    _nzbCheck(nullptr), _quiet(false),
    _proxySocks5(QNetworkProxy::NoProxy), _proxyUrl(),
    _logFile(nullptr), _logStream(nullptr)
{
    QThread::currentThread()->setObjectName(sMainThreadName);

#ifdef __USE_HMI__
    if (_hmi)
        _hmi->setWindowTitle(QString("%1 v%2").arg(sAppName, versionLabel()));
#endif

    // in case we want to generate random uploader (_from not provided)
//    std::srand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
    std::srand(QUuid::createUuid().data1); // use more random seed

    // check if the embedded par2 generator is available (windows or appImage)
    // ParPar (much faster) is preferred if it is shipped next to ngPost: parpar(.exe) or parpar/parpar(.exe)
    // PAR2_PATH (config file) or --par2_path still take precedence (cf below)
    const QString appDir = qApp->applicationDirPath();
    QStringList par2Candidates;
#if defined(WIN32) || defined(__MINGW64__)
    // the shipped ParPar is a 64bit build: use it only on a 64bit Windows (even for a 32bit ngPost)
    if (QSysInfo::currentCpuArchitecture().contains("64"))
        par2Candidates << QString("%1/parpar.exe").arg(appDir)
                       << QString("%1/parpar/parpar.exe").arg(appDir);
    par2Candidates << QString("%1/par2.exe").arg(appDir);
#else
    par2Candidates << QString("%1/parpar").arg(appDir)
                   << QString("%1/parpar/parpar").arg(appDir)
                   << QString("%1/par2").arg(appDir);
#endif
    for (const QString &par2Embedded : par2Candidates)
    {
        QFileInfo fi(par2Embedded);
        if (fi.exists() && fi.isFile() && fi.isExecutable())
        {
            _par2Path = par2Embedded;
            break;
        }
    }

    connect(this, &NgPost::log,   this, &NgPost::onLog,   Qt::QueuedConnection);
    connect(this, &NgPost::logLevel, this, &NgPost::onLogLevel, Qt::QueuedConnection);
    connect(this, &NgPost::error, this, &NgPost::onError, Qt::QueuedConnection);

    _loadTanslators();

#if defined(__DEBUG__) && QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QNetworkConfigurationManager netConfMgr;
    for (const QNetworkConfiguration &conf : netConfMgr.allConfigurations())
    {
        qDebug() << "net conf: " << conf.name()
                 << ", bearerType: " << conf.bearerTypeName()
            #if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
                 << ", timeout: " << conf.connectTimeout()
            #endif
                 << ", isValid: " << conf.isValid()
                 << ", state: " << conf.state();
    }

    QNetworkConfiguration conf = netConfMgr.defaultConfiguration();
    qDebug() << "DEFAULT conf: " << conf.name()
             << ", bearerType: " << conf.bearerTypeName()
          #if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
             << ", timeout: " << conf.connectTimeout()
          #endif
             << ", isValid: " << conf.isValid()
             << ", state: " << conf.state();
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(&_netMgr, &QNetworkAccessManager::networkAccessibleChanged, this, &NgPost::onNetworkAccessibleChanged);
    _notifier.setAppName(sAppName);
    connect(&_notifier, &Notifier::log, this, [this](const QString &line){ _log(line); });
#endif
}


NgPost::~NgPost()
{
#ifdef __DEBUG__
    _log("Destuction NgPost...");
#endif

    if (_logStream){
        delete _logStream;
        delete _logFile;
    }

    if (_nzbCheck)
        delete _nzbCheck;

    _stopMonitoring();

//    _finishPosting();

    _progressbarTimer.stop();

    closeAllPostingJobs();

    if (_activeJob)
        delete _activeJob;
    qDeleteAll(_pendingJobs);
    _pendingJobs.clear();

    delete _postQueue; // releases its lock
    _postQueue = nullptr;

    qDeleteAll(_nntpServers);
    qDeleteAll(_removedServers);

    if (_urlNzbUpload)
        delete _urlNzbUpload;

#ifdef __USE_TMP_RAM__
    if (_storage)
        delete _storage;
#endif
}

int NgPost::nbMissingArticles() const
{
    return _nzbCheck->nbMissingArticles();
}

void NgPost::_finishPosting()
{
#ifdef __USE_HMI__
    if (_hmi || _dispProgressBar)
#else
    if (_dispProgressBar)
#endif
        disconnect(&_progressbarTimer, &QTimer::timeout, this, &NgPost::onRefreshprogressbarBar);

    if (_activeJob && _activeJob->hasUploaded())
    {
#ifdef __USE_HMI__
        if (_hmi || _dispProgressBar)
#else
        if (_dispProgressBar)
#endif
        {
            onRefreshprogressbarBar();
#ifdef __USE_HMI__
            if (!_hmi)
#endif
                std::cout << std::endl;
        }
    }
}

void NgPost::updateGroups(const QString &groups)
{
    _grpList.clear();
    for (const QString &grp : groups.split(","))
    {
        QString name = grp.trimmed(); // "a.b.c, d.e.f" would otherwise keep a space in front of the second group
        if (!name.isEmpty() && !_grpList.contains(name))
            _grpList << name;
    }
    _nbGroups = _grpList.size();
}


#ifdef __USE_HMI__
bool NgPost::systemUsesDarkTheme()
{
#if defined(WIN32) || defined(__MINGW64__)
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                  QSettings::NativeFormat);
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
#else
    // other OS: dark if the system palette has a dark window
    return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
#endif
}

void NgPost::setTheme(const QString &theme)
{
    _theme = theme.toLower();
    applyTheme();
}

void NgPost::applyTheme()
{
    // set the environment variable NGPOST_NO_THEME to get the native look of the system back
    if (qEnvironmentVariableIsSet("NGPOST_NO_THEME"))
        return;

    // Fusion gives the same, predictable base on every OS (the native styles ignore most of the style sheet)
    static QPalette sSystemPalette = QApplication::palette(); // before we change it (for the auto mode on Linux)
    if (QApplication::style()->objectName().compare("fusion", Qt::CaseInsensitive) != 0)
        QApplication::setStyle("Fusion");

    if (_theme == "dark")
        _darkTheme = true;
    else if (_theme == "light")
        _darkTheme = false;
    else
    {
#if defined(WIN32) || defined(__MINGW64__)
        _darkTheme = systemUsesDarkTheme();
#else
        _darkTheme = sSystemPalette.color(QPalette::Window).lightness() < 128;
#endif
    }

    // colours of the two themes (Windows 11 inspired), used by the palette and by :/theme/theme.qss
    static const QMap<QString, QString> sLight = {
        {"window", "#f3f3f3"}, {"sidebar", "#ebebeb"}, {"card", "#ffffff"}, {"border", "#e5e5e5"},
        {"inputBorder", "#d1d1d1"}, {"input", "#ffffff"}, {"text", "#1b1b1b"}, {"subtext", "#5f5f5f"},
        {"disabled", "#a0a0a0"}, {"button", "#fdfdfd"}, {"buttonHover", "#f5f5f5"}, {"buttonPressed", "#ebebeb"},
        {"alt", "#f7f7f7"}, {"accent", "#0067c0"}, {"accentHover", "#1975c5"}, {"accentPressed", "#3a87ca"},
        {"accentText", "#ffffff"}, {"selection", "#cce4f7"}, {"selectionText", "#1b1b1b"},
        {"scroll", "#c3c3c3"}, {"scrollHover", "#9e9e9e"}, {"chunk", "#0067c0"}, {"progress", "#e3e3e3"},
        {"navHover", "#e0e0e0"}, {"navChecked", "#dcdcdc"}, {"tooltip", "#ffffff"}
    };
    static const QMap<QString, QString> sDark = {
        {"window", "#202020"}, {"sidebar", "#1b1b1b"}, {"card", "#2b2b2b"}, {"border", "#383838"},
        {"inputBorder", "#474747"}, {"input", "#1f1f1f"}, {"text", "#f3f3f3"}, {"subtext", "#a9a9a9"},
        {"disabled", "#6d6d6d"}, {"button", "#333333"}, {"buttonHover", "#3b3b3b"}, {"buttonPressed", "#2a2a2a"},
        {"alt", "#262626"}, {"accent", "#4cc2ff"}, {"accentHover", "#48b2ea"}, {"accentPressed", "#42a1d2"},
        {"accentText", "#000000"}, {"selection", "#264f78"}, {"selectionText", "#ffffff"},
        {"scroll", "#5c5c5c"}, {"scrollHover", "#7c7c7c"}, {"chunk", "#4cc2ff"}, {"progress", "#3a3a3a"},
        {"navHover", "#2a2a2a"}, {"navChecked", "#2f2f2f"}, {"tooltip", "#2b2b2b"}
    };
    const QMap<QString, QString> &c = _darkTheme ? sDark : sLight;

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor(c["window"]));
    pal.setColor(QPalette::WindowText,      QColor(c["text"]));
    pal.setColor(QPalette::Base,            QColor(c["input"]));
    pal.setColor(QPalette::AlternateBase,   QColor(c["alt"]));
    pal.setColor(QPalette::Text,            QColor(c["text"]));
    pal.setColor(QPalette::Button,          QColor(c["button"]));
    pal.setColor(QPalette::ButtonText,      QColor(c["text"]));
    pal.setColor(QPalette::BrightText,      QColor(c["text"]));
    pal.setColor(QPalette::Highlight,       QColor(c["accent"]));
    pal.setColor(QPalette::HighlightedText, QColor(c["accentText"]));
    pal.setColor(QPalette::ToolTipBase,     QColor(c["tooltip"]));
    pal.setColor(QPalette::ToolTipText,     QColor(c["text"]));
    pal.setColor(QPalette::Link,            QColor(c["accent"]));
    pal.setColor(QPalette::PlaceholderText, QColor(c["disabled"]));
    pal.setColor(QPalette::Light,           QColor(c["border"]));
    pal.setColor(QPalette::Midlight,        QColor(c["border"]));
    pal.setColor(QPalette::Mid,             QColor(c["inputBorder"]));
    pal.setColor(QPalette::Dark,            QColor(c["inputBorder"]));
    pal.setColor(QPalette::Shadow,          QColor(_darkTheme ? "#000000" : "#a0a0a0"));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(c["disabled"]));
    pal.setColor(QPalette::Disabled, QPalette::Text,       QColor(c["disabled"]));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(c["disabled"]));
    QApplication::setPalette(pal);

    QFile qss(":/theme/theme.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString style = QString::fromUtf8(qss.readAll());
        for (auto it = c.cbegin(); it != c.cend(); ++it)
            style.replace(QString("@%1@").arg(it.key()), it.value());
        style.replace("@arrows@", _darkTheme ? "_dark" : "");
        qApp->setStyleSheet(style);
    }

    MainWindow::setThemeColors(_darkTheme);
    if (_hmi)
        _hmi->onThemeApplied();
}

int NgPost::startHMI()
{
    QString err = parseDefaultConfig();
    applyTheme(); // after the config (theme)
    if (!err.isEmpty())
        _error(err);

    if (_from.empty())
            _from = randomStdFrom();
#ifdef __DEBUG__
    _dumpParams();
#endif
    _hmi->init(this);
    _hmi->show();
    changeLanguage(_lang); // reforce lang set up...

    return _app->exec();
}
#endif


void NgPost::onLog(QString msg, bool newline)
{
    _log(msg, newline);
}

void NgPost::onLogLevel(QString msg, int level)
{
    _logLevel(msg, level);
}

void NgPost::onError(QString msg)
{
    _error(msg, ERROR_CODE::COMPLETED_WITH_ERRORS);
}


void NgPost::onErrorConnecting(QString err)
{
    _error(err, ERROR_CODE::COMPLETED_WITH_ERRORS);
}


ProgressInfo NgPost::progressInfo() const
{
    ProgressInfo info;
    PostingJob *job = _activeJob;
    if (!job)
        return info;

    switch (job->phase())
    {
    case PostingJob::Phase::WAITING:     info.phase = ProgressInfo::IDLE;        break;
    case PostingJob::Phase::COMPRESSING: info.phase = ProgressInfo::COMPRESSING; break;
    case PostingJob::Phase::PAR2:        info.phase = ProgressInfo::PAR2;        break;
    case PostingJob::Phase::UPLOADING:   info.phase = ProgressInfo::UPLOADING;   break;
    case PostingJob::Phase::DONE:
        info.phase = job->uploadFinished() ? ProgressInfo::DONE : ProgressInfo::ABORTED;
        break;
    }
    info.paused     = job->isPaused() && info.phase == ProgressInfo::UPLOADING;
    info.nbTotal    = job->nbArticlesTotal();
    info.nbUploaded = job->nbArticlesUploaded();
    info.nbFailed   = job->nbArticlesFailed();
    info.sizeTotal  = job->totalSizeBytes();
    info.sizeDone   = job->uploadedSizeBytes();
    info.avgSpeed   = job->avgSpeed();
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    info.immediateSpeed = job->immediateSpeed();
#endif
    info.etaSec     = info.phase == ProgressInfo::UPLOADING ? job->etaSeconds() : -1;
    return info;
}

QStringList NgPost::progressParts(const ProgressInfo &info)
{
    QStringList parts;
    switch (info.phase)
    {
    case ProgressInfo::IDLE:
        return parts;
    case ProgressInfo::COMPRESSING: parts << tr("Compression"); break;
    case ProgressInfo::PAR2:        parts << tr("par2"); break;
    case ProgressInfo::UPLOADING:   parts << (info.paused ? tr("Paused") : tr("Upload")); break;
    case ProgressInfo::DONE:
        parts << (info.nbFailed ? tr("Completed, %1 Article(s) failed").arg(info.nbFailed) : tr("Completed"));
        break;
    case ProgressInfo::ABORTED:     parts << tr("Stopped"); break;
    }

    bool uploading = info.phase == ProgressInfo::UPLOADING && !info.paused;
    if (uploading && info.etaSec >= 0)
        parts << tr("ETA %1").arg(PostingJob::humanDuration(info.etaSec * 1000));
    if (uploading && !info.immediateSpeed.trimmed().isEmpty())
    {
        QString speed = info.immediateSpeed.simplified();
        if (SpeedLimiter::limit() > 0)
            speed += " " + tr("(max %1 MB/s)").arg(QLocale().toString(SpeedLimiter::limit() / (1024. * 1024.), 'g', 4));
        parts << speed;
    }
    else if (info.phase == ProgressInfo::DONE)
        parts << info.avgSpeed.simplified();
    if (info.nbTotal && (info.phase == ProgressInfo::UPLOADING || info.phase == ProgressInfo::DONE))
    {
        parts << QString("%1 / %2").arg(PostingJob::humanSize(static_cast<double>(info.sizeDone))).arg(
                     PostingJob::humanSize(static_cast<double>(info.sizeTotal)));
        parts << QString("%1 / %2").arg(info.nbUploaded).arg(info.nbTotal);
    }
    return parts;
}

void NgPost::onRefreshprogressbarBar()
{
    ProgressInfo info = progressInfo();
#ifdef __USE_HMI__
    if (_hmi)
        _hmi->updateProgress(info);
    else
#endif
    {
        if (info.phase == ProgressInfo::COMPRESSING || info.phase == ProgressInfo::PAR2)
        {
            std::cout << "\r" << progressParts(info).first().toStdString() << "...                                        ";
        }
        else if (info.phase != ProgressInfo::IDLE)
        {
            float progressbar = info.nbTotal ? static_cast<float>(info.nbUploaded) / info.nbTotal : 0.f;

            std::cout << "\r[";
            int pos = static_cast<int>(std::floor(progressbar * sprogressbarBarWidth));
            for (int i = 0; i < sprogressbarBarWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progressbar * 100) << " %"
                      << " (" << info.nbUploaded << " / " << info.nbTotal << ")"
                      << " avg. speed: " << info.avgSpeed.toStdString();
            if (info.etaSec >= 0)
                std::cout << "  " << tr("ETA %1").arg(PostingJob::humanDuration(info.etaSec * 1000)).toStdString();
            std::cout << "   ";
        }
        std::cout.flush();
    }
    // the timer goes on during the whole post (compression and par2 included), not when it is paused:
    // resume() starts it again
    if (_activeJob && !info.paused && info.phase != ProgressInfo::DONE && info.phase != ProgressInfo::ABORTED)
        _progressbarTimer.start(_refreshRate);
}


#include <QTranslator>
void NgPost::_loadTanslators()
{
    QDir dir(sTranslationPath);
    for (const QFileInfo &qmFile : dir.entryInfoList(QStringList("ngPost_*.qm")))
    {
        QTranslator *translator = new QTranslator();
        QString lang = qmFile.completeBaseName(); // ngPost_en
        lang.remove(0, lang.indexOf('_') + 1); // en

        if (translator->load(qmFile.absoluteFilePath()))
            _translators.insert(lang, translator);
        else
        {
            if (lang == "en")
                _translators.insert(lang, nullptr);
            else
                _cerr << tr("error loading translator %1").arg(qmFile.absoluteFilePath()) << "\n" << MB_FLUSH;
            delete translator;
        }
    }
#ifdef __DEBUG__
    qDebug() << "available translators: " << _translators.keys().join(", ");
#endif
    if (_lang != "en"){
        if (_translators.contains(_lang))
            _app->installTranslator(_translators[_lang]);
        else
        {
            _cerr << tr("ERROR: couldn't find translator for lang %1").arg(_lang) << "\n" << MB_FLUSH;
            _lang = "en";
        }
    }
}

void NgPost::changeLanguage(const QString &lang)
{
    _app->removeTranslator(_translators[_lang]);
    _lang = lang;

    if (_translators[_lang])
        _app->installTranslator(_translators[_lang]);
}

bool NgPost::checkSupportSSL()
{
    if (!PostingJob::supportsSsl())
    {
        _log(tr("SSL issue on your system..."));
        _log(PostingJob::sslSupportInfo());
        return false;
    }
    return true;
}

QStringList NgPost::splitArgs(const QString &args)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return parseCombinedArgString(args);
#else
    return QProcess::splitCommand(args);
#endif
}

void NgPost::doNzbPostCMD(PostingJob *job)
{
    // first NZB_UPLOAD_URL
    if (_urlNzbUpload)
    {
        FileUploader *testUpload = new FileUploader(_netMgr, job->nzbFilePath());
        connect(testUpload, &FileUploader::error,      this,       &NgPost::onError,      Qt::DirectConnection);
        connect(testUpload, &FileUploader::log,        this,       &NgPost::onLog,        Qt::DirectConnection);
        connect(testUpload, &FileUploader::readyToDie, testUpload, &QObject::deleteLater, Qt::QueuedConnection);
        testUpload->startUpload(*_urlNzbUpload);
    }

    // second NZB_POST_CMD
    if (!_nzbPostCmd.isEmpty())
    {
        for (const QString &nzbPostCmd : _nzbPostCmd)
        {
            // Split the command template first and substitute the placeholders afterwards in each argument:
            // a value with spaces or quotes (nzb path, rar name/pass...) must stay ONE argument
            QStringList args = splitArgs(nzbPostCmd);
            if (args.isEmpty())
                continue;

            for (QString &arg : args)
            {
                arg.replace("%1",                   job->nzbFilePath()); // for backwards compatibility
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::nzbPath],
        #if defined( Q_OS_WIN )
                        QString(job->nzbFilePath()).replace("/", "\\")
        #else
                        job->nzbFilePath()
        #endif
                        );
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::nzbName],          QFileInfo(job->nzbFilePath()).completeBaseName());
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::rarName],          job->rarName());
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::rarPass],          job->rarPass());
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::nbArticles],       QString::number(job->nbArticlesTotal()));
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::nbArticlesFailed], QString::number(job->nbArticlesFailed()));
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::sizeInByte],       QString::number(job->_totalSize));
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::nbFiles],          QString::number(job->_nbFiles));
                arg.replace(sPostCmdPlaceHolders[PostCmdPlaceHolders::groups],           job->groups());
            }
            const QString fullCmd = args.join(" ");
            QString     cmd  = args.takeFirst();
            qDebug() << "cmd: " << cmd << ", args: " << args;
            // start it in background: QProcess::execute would freeze ngPost until the command is finished
            bool started = QProcess::startDetached(cmd, args);
            if (debugMode())
                _log(tr("NZB Post cmd: %1 started: %2").arg(fullCmd).arg(started ? "yes" : "no"));
            else
                _log(fullCmd);
            if (!started)
                _error(tr("Couldn't start the NZB Post cmd: %1").arg(fullCmd));
        }
    }

}

bool NgPost::isPaused() const
{
    return _activeJob && (_activeJob->isPaused() || _activeJob->isPauseRequested());
}

void NgPost::pause(bool byUser)
{
    if (!_activeJob || _activeJob->isPaused())
        return;
    _activeJob->setPausedByUser(byUser);
    if (!_activeJob->hasPostStarted())
        _activeJob->setPauseRequested(true); // compression or par2: it pauses when the upload starts
    else
        _activeJob->pause();
#ifdef __USE_HMI__
    if (_hmi)
        _hmi->setPauseIcon(false);
#endif
    _refreshPost(_activeJob);
}

void NgPost::resume()
{
    if (!_activeJob)
        return;
    _activeJob->setPausedByUser(false);
    if (_activeJob->isPauseRequested())
        _activeJob->setPauseRequested(false);
    else if (_activeJob->isPaused())
    {
        _activeJob->resume();
        _progressbarTimer.start(_refreshRate);
    }
    else
        return;
#ifdef __USE_HMI__
    if (_hmi)
        _hmi->setPauseIcon(true);
#endif
    _refreshPost(_activeJob);
}


#ifdef __USE_TMP_RAM__
qint64 NgPost::ramAvailable() const { return _storage->bytesAvailable(); }

qint64 NgPost::recursiveSize(const QFileInfo &fi)
{
    if (fi.isDir())
    {
        qint64 size = 4096; // size of a dir on Unix
        QDir dir(fi.absoluteFilePath());
        for (const QFileInfo &subFi : dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries, QDir::Name|QDir::DirsLast))
        {
            if (subFi.isDir())
                size += recursiveSize(subFi);
            else
                size += subFi.size();
        }
        return size;
    }
    else if (fi.isFile())
        return fi.size();
    else
        return 0;
}
#endif


QStringList NgPost::getPostingGroups(const QStringList &groups) const
{
    if (groups.isEmpty())
        return getPostingGroups();
    if (_groupPolicy == GROUP_POLICY::EACH_POST && groups.size() > 1)
        return QStringList(groups.at(std::rand() % groups.size()));
    return groups;
}

QString NgPost::serverKey(const NntpServerParams &server)
{
    QString key = QString("%1:%2").arg(server.host).arg(server.port);
    if (server.auth && !server.user.empty())
        key.prepend(QString::fromStdString(server.user) + "@");
    return key;
}

void NgPost::_post(const QFileInfo &fileInfo, const QString &monitorFolder)
{
    setNzbName(fileInfo);
    const PostTargets &targets = _watchTargets; // (empty on the command line)
    if (!targets.nameTemplate.isEmpty())
    {
        if (PostTargets::usesCounter(targets.nameTemplate))
        {
            ++_nameCounter;
#ifdef __USE_HMI__
            if (_hmi)
                _hmi->scheduleAutoSave(); // the counter is kept
#endif
        }
        _nzbName = PostTargets::applyTemplate(targets.nameTemplate, _nzbName, QDateTime::currentDateTime(), _nameCounter);
    }
    const QString globalNzbPath = _nzbPath;
    if (!targets.nzbDir.isEmpty())
        _nzbPath = targets.nzbDir;
    QString nzbFilePath = nzbPath(monitorFolder);
    _nzbPath = globalNzbPath;
    if (!nzbFilePath.endsWith(".nzb"))
        nzbFilePath += ".nzb";

    _rarName = _nzbName;
    _rarPass = "";
    if (_doCompress)
    {
        if (_genName)
            _rarName = randomPass(_lengthName);

        if (_genPass) // shall we gen password?
            _rarPass = randomPass(_lengthPass);
        if (!_rarPassFixed.isEmpty()) // rar pass fixed would take other
            _rarPass = _rarPassFixed;
        if (!_rarPass.isEmpty())
            _meta.remove("password");
    }

    qDebug() << "Start posting job for " << _nzbName
             << " with rar_name: " << _rarName << " and pass: " << _rarPass
             << " (auto delete: " << _delAuto << ")";

    PostingJob *job = new PostingJob(this, nzbFilePath, {fileInfo}, nullptr,
                                     getPostingGroups(targets.groups), from(),
                                     _obfuscateArticles, _obfuscateFileName,
                                     _tmpPath, _rarPath, _rarArgs,
                                     _rarSize, _useRarMax, _par2Pct,
                                     _doCompress, _doPar2, _rarName, _rarPass,
                                     _keepRar, _delAuto || !_movePosted.isEmpty(), false);
    job->setMovePostedTo(_movePosted);
    job->setServers(targets.servers);
    startPostingJob(job);
}


#ifdef __USE_HMI__
#include <QDesktopServices>

#include "hmi/AboutNgPost.h"
void NgPost::onAboutClicked()
{
    AboutNgPost about(this);
    if (_hmi)
        connect(&about, &AboutNgPost::saveReportRequested, _hmi, &MainWindow::saveDiagnosticReport);
    about.exec();
}
#endif

void NgPost::onPostingJobStarted()
{
#ifdef __USE_HMI__
    if (_hmi || _dispProgressBar)
#else
    if (_dispProgressBar)
#endif
    {
        connect(&_progressbarTimer, &QTimer::timeout, this, &NgPost::onRefreshprogressbarBar,
                Qt::ConnectionType(Qt::DirectConnection | Qt::UniqueConnection));
        _progressbarTimer.start(_refreshRate);
    }
#ifdef __USE_HMI__
    if (_hmi && _activeJob)
        _hmi->setJobLabel(_activeJob->nzbName()); // not "Ready" while it is compressing
#endif
}

void NgPost::onPackingDone()
{
    if (_preparePacking)
    {
        PostingJob *job = static_cast<PostingJob*>(sender());
#ifdef __DEBUG__
qDebug() << "[MB_TRACE][Issue#82][NgPost::onPackingDone] job: " << job
         << ", file: " << job->nzbName();
#endif
        if (job == _activeJob)
        {
#ifdef __DEBUG__
            _log("[NgPost::onPackingDone] Active Job packed :)");
#endif
            // The previous active Job finished posting before the packing of the next one
            if (_packingJob == nullptr)
            {
                if (!job->hasPostStarted())
                {
#ifdef __DEBUG__
            _log("[NgPost::onPackingDone] Active Job didn't start posting...");
#endif
                    job->_postFiles();
                }
                _prepareNextPacking();
            }
        }
        else if (job != _packingJob)
            _error("unexpected job finished packing..."); // should never happen...
#ifdef __DEBUG__
        else
            _log("[NgPost::onPackingDone] Packing Job done :)");
#endif
    }
}

void NgPost::_prepareNextPacking()
{
    if (_packingJob)
        return; // already prepared (or being prepared)
    int idx = -1;
    for (int i = 0; i < _pendingJobs.size(); ++i)
        if (!_pendingJobs.at(i)->isHeld())
        {
            idx = i;
            break;
        }
    if (idx < 0)
        return; // nothing waits (or only paused posts)
    PostingJob *job = _pendingJobs.at(idx);
    if (idx > 0)
    {   // the paused posts before it: it goes first (it is the next one to start)
        _pendingJobs.removeAt(idx);
        _pendingJobs.prepend(job);
    }
    _packingJob = job;
    if (job->hasPacking())
        emit job->startPosting(false);
    else if (debugFull())
        _log(tr("no packing needed for next pending job %1").arg(job->nzbName()));
}

QVector<QueueSlot> NgPost::_queueSlots() const
{
    QVector<QueueSlot> queue;
    queue.reserve(_pendingJobs.size());
    for (PostingJob *job : _pendingJobs)
    {
        QueueSlot slot;
        slot.priority = job->priority();
        slot.paused   = job->isHeld();
        slot.locked   = job == _packingJob && job->hasPacking();
        queue << slot;
    }
    return queue;
}

bool NgPost::_startNextJob()
{
    if (_activeJob)
        return false;
    int idx = QueueOrder::nextIndex(_queueSlots());
    if (idx < 0)
        return false;

    _activeJob = _pendingJobs.takeAt(idx);
    onPostingJobStarted();
    if (_preparePacking)
    {
        if (_packingJob == _activeJob)
        {
            _packingJob = nullptr;
            if (_activeJob->isPacked())
            {
                _activeJob->_postFiles();
                _prepareNextPacking();
            }
            else if (!_activeJob->hasPacking())
            {
                if (debugFull())
                    _log(tr("start non packing job..."));
                emit _activeJob->startPosting(true);
                _prepareNextPacking();
            }
            // otherwise it will be triggered automatically when the packing is finished
            // as it is now the active job ;)
        }
        else
        {
            if (_packingJob && _packingJob->hasPacking())
                _error("next active job different to the packing one..."); // should never happen (it stays first)
            _packingJob = nullptr;
            if (_activeJob->isPacked())
            {
                _activeJob->_postFiles();
                _prepareNextPacking();
            }
            else
            {
                emit _activeJob->startPosting(true);
                if (!_activeJob->hasPacking())
                    _prepareNextPacking(); // otherwise when its packing is done (onPackingDone)
            }
        }
    }
    else
        emit _activeJob->startPosting(true);
    _queueChanged();
    return true;
}

void NgPost::_queueChanged()
{
    // a next post without compression nor par2 is not started yet: it follows the order of the queue
    if (_packingJob && !_packingJob->hasPacking())
    {
        int next = QueueOrder::nextIndex(_queueSlots());
        if (next < 0 || _pendingJobs.at(next) != _packingJob)
            _packingJob = nullptr;
    }
    if (_preparePacking && _activeJob && _activeJob->isPacked() && !_packingJob)
        _prepareNextPacking();

    QVector<int> ids;
    QStringList  queueIds;
    if (_activeJob && !_activeJob->queueId().isEmpty())
        queueIds << _activeJob->queueId();
    for (PostingJob *job : _pendingJobs)
    {
        if (job->postId() >= 0)
            ids << job->postId();
        if (!job->queueId().isEmpty())
            queueIds << job->queueId();
    }
    _postList->setQueueOrder(ids);
    if (_postQueue)
        _postQueue->reorder(queueIds); // a restart posts them in the same order

    if ((_activeJob || _packingJob) && !_postListTimer->isActive())
    {   // (it stops when nothing runs, for example when all the posts are paused)
        connect(_postListTimer, &QTimer::timeout, this, &NgPost::_onPostListTimer, Qt::UniqueConnection);
        _postListTimer->start(1000);
    }
}

void NgPost::_refreshPost(PostingJob *job)
{
    if (!job)
        return;
    _postListRefresh(job);
    queueSaveJob(job);
}

void NgPost::onPostingJobFinished()
{
    PostingJob *job = static_cast<PostingJob*>(sender());
#ifdef __DEBUG__
qDebug() << "[MB_TRACE][Issue#82][NgPost::onPostingJobFinished] job: " << job
         << ", file: " << job->nzbName();
#endif
    queueJobEnded(job); // no effect if done at the end of the post (PostingJob::_finishPosting)
    if (job == _activeJob)
    {
        _postListFinish(job);
#ifdef __USE_HMI__
        if (_hmi)
        {
            job->setDone(); // a post stopped before the upload shows "Stopped"
            _hmi->updateProgress(progressInfo());
        }
#endif
#ifdef __USE_HMI__
        if (_hmi && !job->fromGuiForm())
        {
            int ok = 0, errors = 0;
            watchStatsToday(ok, errors); // (also starts a new day)
            if (job->nbArticlesUploaded() > 0 && job->nbArticlesFailed() == 0)
                ++_watchOk;
            else
                ++_watchErrors;
        }
        if (_hmi && !job->fromGuiForm())
            _hmi->autoWidget()->updateFinishedJob(job->getFirstOriginalFile(),
                                                  job->nbArticlesTotal(),
                                                  job->nbArticlesUploaded(),
                                                  job->nbArticlesFailed());
#endif

        QString historyPath = historyFile();
        if (_activeJob->hasPostFinished() && !_activeJob->isRepost() && !historyPath.isEmpty())
        {
            QFile hist(historyPath);
            bool newFile = !hist.exists() || hist.size() == 0;
            if (hist.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
            {
                QTextStream stream(&hist);
                if (newFile)
                    stream << historyHeader().join(_historyFieldSeparator) << "\n";
                qint64 sec = _activeJob->jobDurationMs() / 1000;
                stream << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")
                       << _historyFieldSeparator << _activeJob->nzbName()
                       << _historyFieldSeparator << _activeJob->postSize()
                       << _historyFieldSeparator << _activeJob->avgSpeed().trimmed();
                if (_activeJob->hasCompressed())
                    stream << _historyFieldSeparator << _activeJob->rarName()
                           << _historyFieldSeparator << _activeJob->rarPass();
                else
                    stream << _historyFieldSeparator << _historyFieldSeparator;

                stream << _historyFieldSeparator << _activeJob->groups()
                       << _historyFieldSeparator << _activeJob->from()
                       // columns added by ngPost Deluxe (at the end so the old lines stay valid)
                       << _historyFieldSeparator << (_activeJob->nbArticlesFailed() == 0 ?
                                                         QString("OK") :
                                                         QString("ERRORS (%1)").arg(_activeJob->nbArticlesFailed()))
                       << _historyFieldSeparator << _activeJob->nbOriginalFiles()
                       << _historyFieldSeparator << QString("%1:%2:%3").arg(sec / 3600, 2, 10, QChar('0'))
                                                                     .arg((sec / 60) % 60, 2, 10, QChar('0'))
                                                                     .arg(sec % 60, 2, 10, QChar('0'))
                       << _historyFieldSeparator << QDir::toNativeSeparators(_activeJob->nzbFilePath())
                       << _historyFieldSeparator << inputSize(_activeJob->originalFiles()) // to recognize the same post later
                       // columns added in 1.4 for the statistics: servers used, options, how it was posted
                       << _historyFieldSeparator << _activeJob->serverHosts()
                       << _historyFieldSeparator << historyProfile(_activeJob)
                       << _historyFieldSeparator << historyKind(_activeJob)
                       << _historyFieldSeparator << _activeJob->_totalSize // exact size (the size column is rounded)
                       << "\n" << MB_FLUSH;
                hist.close();
            }
#ifdef __USE_HMI__
            if (_hmi)
                _hmi->onHistoryUpdated();
#endif
        }

#ifdef __USE_HMI__
        if (_hmi && _autoCloseTabs && _activeJob->hasPostFinishedSuccessfully() && _activeJob->postId() >= 0)
            _postList->remove(_activeJob->postId()); // "remove the finished posts from the list"
#endif

        // command line only: post again the Articles that failed (used by the tests, the GUI has a button)
        // result for the notifications (sent below, once we know if other posts are waiting)
        Notifier::Result notifyResult;
        notifyResult.name     = _activeJob->nzbName();
        notifyResult.state    = !_activeJob->hasPostFinished() ? Notifier::State::FAILED :
                                (_activeJob->nbArticlesFailed() ? Notifier::State::ERRORS : Notifier::State::OK);
        notifyResult.size     = _activeJob->postSize();
        notifyResult.speed    = _activeJob->avgSpeed().trimmed();
        notifyResult.nbFailed = _activeJob->nbArticlesFailed();
        notifyResult.nbTotal  = _activeJob->nbArticlesTotal();
        {
            qint64 sec = _activeJob->jobDurationMs() / 1000;
            notifyResult.duration = QString("%1:%2:%3").arg(sec / 3600, 2, 10, QChar('0'))
                                                       .arg((sec / 60) % 60, 2, 10, QChar('0'))
                                                       .arg(sec % 60, 2, 10, QChar('0'));
        }

        PostingJob *repostJob = nullptr;
        if (!_activeJob->fromGuiForm() && !_activeJob->isRepost() && !_activeJob->repostInfo().isEmpty()
                && !qgetenv("NGPOST_REPOST_MISSING").isEmpty())
            repostJob = PostingJob::createRepostJob(this, _activeJob->repostInfo(), nullptr);

        _activeJob->deleteLater();
        _activeJob = nullptr;

        if (repostJob)
        {
            _postListAdd(repostJob);
            _pendingJobs.insert(QueueOrder::nbLocked(_queueSlots()), repostJob);
        }

        _notifier.jobEnded(notifyResult, QueueOrder::nextIndex(_queueSlots()) < 0);

        if (_startNextJob())
            return;
        _queueChanged(); // only paused posts (or nothing) wait
        if (_doShutdownWhenDone && !_shutdownCmd.isEmpty())
        {   // the paused posts don't keep the computer on: they stay in the queue for the next start
            _whenNotificationsSent([this](){ _runShutdown(); });
        }
#ifdef __USE_HMI__
        else if (!_folderMonitor && !_hmi)
#else
        else if (!_folderMonitor)
#endif
        {
            if (debugFull())
                _error(tr(" => closing application"));
            _whenNotificationsSent([](){ qApp->quit(); }); // the notifications of the last post are sent first
        }
    }
    else if (_preparePacking && job ==_packingJob)
    {
        _packingJob = nullptr;
        _pendingJobs.removeOne(job);
        if (job->postId() >= 0) // cancelled while its files were packed: never posted
        {
            _postList->appendLog(job->postId(), tr("Cancelled before it started"), PostEntry::LOG_WARN);
            _postList->finish(job->postId(), PostEntry::State::CANCELLED);
            _dumpPostList();
        }
        job->deleteLater();
        _queueChanged(); // the following post is prepared
        if (!_activeJob)
            _startNextJob();
    }
    else
    {
        // it was a Pending job that has been cancelled
        if (debugFull())
            _log(tr("Cancelled pending job?"));
        if (job->postId() >= 0)
        {
            _postList->appendLog(job->postId(), tr("Cancelled before it started"), PostEntry::LOG_WARN);
            _postList->finish(job->postId(), PostEntry::State::CANCELLED);
            _dumpPostList();
        }
        _pendingJobs.removeOne(job);
        job->deleteLater();
        _queueChanged();
    }
}

void NgPost::_whenNotificationsSent(const std::function<void()> &action)
{
    if (_notifier.pending() <= 0)
    {
        action();
        return;
    }
    auto *conn = new QMetaObject::Connection;
    *conn = connect(&_notifier, &Notifier::allSent, this, [action, conn, this](){
        disconnect(*conn);
        delete conn;
        action();
    });
}

void NgPost::_runShutdown()
{
            //cf https://forum.qt.io/topic/111602/qprocess-signals-not-received-in-slots-except-in-debug-with-breakpoints/
//            int exitCode = QProcess::execute("echo \\\"toto\\\" | /usr/bin/sudo -S /bin/ls -al");
//            qDebug() << QString("Shutdown exit code: %1").arg(exitCode);
            _shutdownProc = new QProcess();
            connect(_shutdownProc, &QProcess::readyReadStandardOutput, this, &NgPost::onShutdownProcReadyReadStandardOutput, Qt::DirectConnection);
            connect(_shutdownProc, &QProcess::readyReadStandardError,  this, &NgPost::onShutdownProcReadyReadStandardError,  Qt::DirectConnection);
            connect(_shutdownProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &NgPost::onShutdownProcFinished, Qt::QueuedConnection);
//            connect(_shutdownProc, &QProcess::started, this, &NgPost::onShutdownProcStarted, Qt::DirectConnection);
//            connect(_shutdownProc, &QProcess::stateChanged,  this, &NgPost::onShutdownProcStateChanged,  Qt::DirectConnection);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
            connect(_shutdownProc, &QProcess::errorOccurred,  this, &NgPost::onShutdownProcError, Qt::DirectConnection);
#endif
//            _shutdownProc->start("/bin/ls", QStringList() << "-al");
//            _shutdownProc->start("/usr/bin/sudo", QStringList() << "-n" << "/sbin/poweroff");

//            qDebug() << " cmd: " << _shutdownCmd;

//            QStringList args = _shutdownCmd.split(QRegularExpression("\\s+"));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            QStringList args = parseCombinedArgString(_shutdownCmd);
#else
            QStringList args = QProcess::splitCommand(_shutdownCmd);
#endif
            QString     cmd  = args.takeFirst();
            qDebug() << "cmd: " << cmd << ", args: " << args;
            _shutdownProc->start(cmd, args);
}

void NgPost::onShutdownProcReadyReadStandardOutput()
{
    QString line(_shutdownProc->readAllStandardOutput());
//    _cout << "Shutdown out: " << line << "\n" << MB_FLUSH;
    _log(QString("Shutdown out: %1").arg(QString(line)));
}

void NgPost::onShutdownProcReadyReadStandardError()
{
    QString line(_shutdownProc->readAllStandardError());
//    _cout << "Shutdown ERROR: " << line << "\n" << MB_FLUSH;
    _error(QString("Shutdown ERROR: %1").arg(line));
}

void NgPost::onShutdownProcFinished(int exitCode)
{
    if (_debug)
        _log(QString("Shutdown proc exitCode: ").arg(exitCode));
    _shutdownProc->deleteLater();
    _shutdownProc = nullptr;
}

//void NgPost::onShutdownProcStarted()
//{
//    qDebug() << "Shutdown proc Started";
//}

//void NgPost::onShutdownProcStateChanged(QProcess::ProcessState newState)
//{
//    qDebug() << "Shutdown proc new State: " << newState;
//}

void NgPost::onShutdownProcError(QProcess::ProcessError error)
{
    _error(QString("Shutdown process Error: %1").arg(error));
}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
void NgPost::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    qDebug() << "[NgPost::onNetworkAccessibleChanged] accessible: " << accessible;
    QString msg(tr("Network access changed: %1"));
    if (accessible == QNetworkAccessManager::NetworkAccessibility::Accessible)
    {
        _log(msg.arg("ON"));
#ifndef __USE_CONNECTION_TIMEOUT__
        if (_activeJob && _activeJob->isPaused())
            _activeJob->resume();
#endif
    }
    else
    {
        _error(msg.arg("OFF"));
#ifndef __USE_CONNECTION_TIMEOUT__
        if (_activeJob)
            _activeJob->pause();
#endif
    }
}
#endif

void NgPost::_log(const QString &aMsg, bool newline) const
{
#ifdef __USE_HMI__
    if (_hmi)
    {
        _hmi->log(aMsg, newline);
        if (_logStream && newline)
            *_logStream << aMsg << "\n" << MB_FLUSH; // force flush in case of crash
    }
    else
#endif
    {
        _cout << aMsg;
        if (newline)
            _cout << "\n";
        _cout << MB_FLUSH;
    }
}

void NgPost::_logLevel(const QString &aMsg, int level) const
{
    Q_UNUSED(level)
#ifdef __USE_HMI__
    if (_hmi)
    {
        _hmi->log(aMsg, true, level);
        if (_logStream)
            *_logStream << aMsg << "\n" << MB_FLUSH; // force flush in case of crash
    }
    else
#endif
        _cout << aMsg << "\n" << MB_FLUSH;
}

void NgPost::_error(const QString &error) const
{
    Diagnostics::instance().event("ERROR", error);
#ifdef __USE_HMI__
    if (_hmi)
    {
        _hmi->logError(error);
        if (_logStream)
            *_logStream << "ERR: " << error << "\n" << MB_FLUSH; // force flush in case of crash
    }
    else
#endif
        _cerr << error << "\n" << MB_FLUSH;
}

void NgPost::_error(const QString &error, NgPost::ERROR_CODE code)
{
    _err = code;
    _error(error);
}


QString NgPost::randomPass(uint length) const
{
    QString pass, alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    int nbLetters = alphabet.length();
    for (uint i = 0 ; i < length ; ++i)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        // OS cryptographic source (unbiased, no modulo): passwords protect the archives
        pass.append(alphabet.at(static_cast<int>(QRandomGenerator::system()->bounded(nbLetters))));
#else
        pass.append(alphabet.at(std::rand()%nbLetters));
#endif
    }

    return pass;
}


void NgPost::closeAllPostingJobs()
{
    _closingApp = true; // the posts that are stopped now stay in the queue to be posted again at the next start
    qDeleteAll(_pendingJobs);
    _pendingJobs.clear();
    _packingJob = nullptr;
    if (_activeJob)
        _activeJob->onStopPosting();
}


bool NgPost::startPostingJob(PostingJob *job, bool atEnd)
{
    // a new post of the auto posting / command line that is already in the history is skipped
    // (Quick Post asks the user before creating its job, the queue and the reposts are never checked)
    if (!job->isRepost() && job->queueId().isEmpty() && !job->fromGuiForm())
    {
        QString posted = duplicateInPost(job->nzbName(), job->originalFiles());
        if (!posted.isEmpty())
        {
            _log(tr("%1 is already in the history (posted on %2): skipped. Use --allow_repost (or warn_duplicates = off in the config) to post it again.").arg(
                     job->nzbName(), posted));
            delete job;
            QTimer::singleShot(0, this, [this](){ // nothing else to do: the command line ends (after the loops that start the posts)
#ifdef __USE_HMI__
                if (!_activeJob && _pendingJobs.isEmpty() && !_folderMonitor && !_hmi)
#else
                if (!_activeJob && _pendingJobs.isEmpty() && !_folderMonitor)
#endif
                    qApp->quit();
            });
            return false;
        }
    }

    if (!job->isRepost() && job->queueId().isEmpty())
    {
        if (PostQueue *queue = postQueue())
        {
            job->setQueueId(PostQueue::newId());
            queue->put(job->toQueueEntry());
        }
    }
    _postListAdd(job);

#ifdef __DEBUG__
qDebug() << "[MB_TRACE][Issue#82][NgPost::startPostingJob] job: " << job
         << ", file: " << job->nzbName();
#endif
#ifdef __USE_HMI__
    if (_hmi)
    {
        connect(job, &PostingJob::articlesNumber, _hmi, &MainWindow::onSetProgressBarRange,  Qt::QueuedConnection);
        if (!job->fromGuiForm())
            connect(job, &PostingJob::postingStarted,  _hmi->autoWidget(), &AutoPostWidget::onMonitorJobStart);
    }
#endif

    if (_activeJob || job->isHeld())
    {   // it waits: after the posts with the same or a higher priority (resumed posts keep their order)
        int idx = atEnd ? _pendingJobs.size() : QueueOrder::insertIndex(_queueSlots(), job->priority());
        _pendingJobs.insert(idx, job);
        if (job->isHeld() && job->postId() >= 0)
            _postList->appendLog(job->postId(), tr("Added paused: it starts when you resume it"), PostEntry::LOG_INFO);
        _queueChanged(); // (prepares it if it is the next one)
        return false;
    }
    else
    {
        _activeJob = job;
        onPostingJobStarted(); // the progress shows the compression and the par2 too
        emit job->startPosting(true);
        _queueChanged();
        return true;
    }
}

#ifdef __DEBUG__
void NgPost::_dumpParams() const
{
    QString servers;
    for (NntpServerParams *srv : _nntpServers)
        servers += srv->str() + " ";
    qDebug() << "[NgPost::_dumpParams]>>>>>>>>>>>>>>>>>>\n"
             << "nb Servers: " << _nntpServers.size() << ": " << servers

             << "\n\nnbThreads: " << _nbThreads << " nb Inputs: " << _nbFiles
             << ", nzbPath: " << _nzbPath << ", nzbName" << _nzbName

             << "\ninputDir: " << _inputDir << ", autoDelete: " << _delAuto
             << ", packAutoKeywords:" << _packAutoKeywords << ", autoClose: " << _autoCloseTabs
             << "\n, monitor delay: " << _monitorSecDelayScan << " ignore dir: " << _monitorIgnoreDir
             << " ext: " << _monitorExtensions
             << "\n\nfrom: " << _from.c_str() << ", genFrom: " << _genFrom << ", saveFrom: " << _saveFrom
             << ", groups: " << _grpList.join(",")
             << " policy: " << sGroupPolicies[_groupPolicy].toUpper()
             << "\narticleSize: " << sArticleSize
             << ", obfuscate articles: " << _obfuscateArticles
             << ", disp progress bar: " << _dispProgressBar
             << ", disp posting files: " << _dispFilesPosting
             << ", logInFile (GUI only): " << (_logFile == nullptr ? "NO" : "YES")
             << "\n\ncompression settings: <tmp_path: " << _tmpPath << ">"
#ifdef __USE_TMP_RAM__
             << " <ram_path: " << _ramPath << " ratio: " << _ramRatio << ">"
#endif
             << ", <rar_path: " << _rarPath << ">"
             << ", <rar_size: " << _rarSize << ">"
             << "\n<par2_pct: " << _par2Pct << ">"
             << ", <par2_path: " << _par2Path << ">"
             << ", <par2_pathCfg: " << _par2PathConfig << ">"
             << ", <par2_args: " << _par2Args << ">"
             << "\n\ncompress: " << _doCompress << ", doPar2: " << _doPar2
             << ", gen_name: " << _genName << ", genPass: " << _genPass
             << "\nrarName: " << _rarName << ", rarPass: " << _rarPass
             << ", lengthName: " << _lengthName << ", lengthPass: " << _lengthPass
             << "\n[NgPost::_dumpParams]<<<<<<<<<<<<<<<<<<\n";
}
#endif


const QString NgPost::sNgPostASCII = QString("\
                   __________               __\n\
       ____    ____\\______   \\____  _______/  |_\n\
      /    \\  / ___\\|     ___/  _ \\/  ___/\\   __\\\n\
     |   |  \\/ /_/  >    |  (  <_> )___ \\  |  |\n\
     |___|  /\\___  /|____|   \\____/____  > |__|\n\
          \\//_____/                    \\/\n\
");


