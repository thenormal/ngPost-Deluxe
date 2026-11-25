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

// Configuration file: read, save, export and import of the settings (member functions of NgPost, split from NgPost.cpp)

QString NgPost::_parseConfig(const QString &configPath)
{
    QString err;
    QFileInfo fileInfo(configPath);
    _configPath = fileInfo.absoluteFilePath();
    if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable())
        err = tr("The config file '%1' is not readable...").arg(configPath);

    QFile file(fileInfo.absoluteFilePath());
    if (file.open(QIODevice::ReadOnly))
    {
        NntpServerParams *serverParams = nullptr;
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#') || line.startsWith('/'))
                continue;
            else if (line == "[server]")
            {
                serverParams = new NntpServerParams();
                _nntpServers << serverParams;
            }
            else
            {
                QStringList args = line.split('=');
                if (args.size() >= 2)
                {
                    QString opt = args.takeFirst().trimmed().toLower(),
                            val = args.at(0).trimmed();
                    bool ok = false;
                    if (opt == sOptionNames[Opt::THREAD])
                    {
                        int nb = val.toInt(&ok);
                        if (ok)
                        {
                            if (nb < 1)
                                _nbThreads = 1;
                            else
                                _nbThreads = nb;
                        }
                    }
                    else if (opt == sOptionNames[Opt::NZB_PATH])
                    {
                        QFileInfo nzbFI(val);
                        if (nzbFI.exists() && nzbFI.isDir() && nzbFI.isWritable())
                        {
                            _nzbPath     = val;
                            _nzbPathConf = val;
                        }
                        else
                            err += tr("the nzbPath '%1' is not writable...\n").arg(val);
                    }
                    else if (opt == sOptionNames[Opt::NZB_UPLOAD_URL])
                    {
                        _urlNzbUploadStr = val;
                        for (int i = 2; i < args.size(); ++i)
                            _urlNzbUploadStr += QString("=%1").arg(args.at(i).trimmed());
                        _urlNzbUpload    = new QUrl(_urlNzbUploadStr);
                        QStringList allowedProtocols = {"ftp", "http", "https"};
                        if (!allowedProtocols.contains(_urlNzbUpload->scheme()))
                        {
                            err += tr("Unsupported protocol for NZB_UPLOAD_URL (%1). You can only use: %2\n").arg(
                                        _urlNzbUpload->toString(QUrl::RemoveUserInfo)).arg(allowedProtocols.join(", "));
                            delete _urlNzbUpload;
                            _urlNzbUpload = nullptr;
                        }
                    }
                    else if (opt == sOptionNames[Opt::RESUME_WAIT])
                    {
                        ushort nb = val.toUShort(&ok);
                        if (ok && nb > sDefaultResumeWaitInSec)
                            _waitDurationBeforeAutoResume = nb;
                    }
                    else if (opt == sOptionNames[Opt::NO_RESUME_AUTO])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _tryResumePostWhenConnectionLost = false;
                    }
                    else if (opt == sOptionNames[Opt::PREPARE_PACKING])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _preparePacking = true;
                    }
                    else if (opt == sOptionNames[Opt::SOCK_TIMEOUT])
                    {
                        int nb = val.toInt(&ok);
                        if (ok)
                        {
                            int timeout = nb *1000;
                            if (timeout > sMinSocketTimeOut)
                                _socketTimeOut = timeout;
                        }
                    }
                    else if (opt == sOptionNames[Opt::MONITOR_FOLDERS])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _monitor_nzb_folders = true;
                    }
                    else if (opt == sOptionNames[Opt::MONITOR_IGNORE_DIR])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _monitorIgnoreDir = true;
                    }
                    else if (opt == sOptionNames[Opt::MONITOR_SEC_DELAY_SCAN])
                    {
                        int nb = val.toInt(&ok);
                        if (ok && nb > 1 && nb <= 120) {
                            _monitorSecDelayScan = static_cast<ushort>(nb);
                            FoldersMonitorForNewFiles::sMSleep = _monitorSecDelayScan * 1000;
                        }
                    }
                    else if (opt == sOptionNames[Opt::NZB_RM_ACCENTS])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _removeAccentsOnNzbFileName = true;
                    }
                    else if (opt == sOptionNames[Opt::AUTO_CLOSE_TABS])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _autoCloseTabs = true;
                    }
                    else if (opt == sOptionNames[Opt::LOG_IN_FILE] && useHMI())
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                        {
#if defined(WIN32) || defined(__MINGW64__)
                            QString logFilePath = sDefaultLogFile;
#else
                            QString logFilePath = QString("%1/%2").arg(getenv("HOME")).arg(sDefaultLogFile);
#endif

                            _logFile = new QFile(logFilePath);
                            if (_logFile->open(QIODevice::WriteOnly|QIODevice::Text))
                            {
                                _logStream = new QTextStream(_logFile);
                                _log(tr("ngPost starts logging: %1").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")));
                            }
                            else
                            {
                                delete _logFile;
                                _logFile = nullptr;
                                _error(tr("Error opening log file: '%1'").arg(logFilePath));
                            }
                        }
                    }
                    else if (opt == sOptionNames[Opt::MONITOR_EXT])
                    {
                        for (const QString &extension : val.split(","))
                            _monitorExtensions << extension.trimmed();
                    }
                    else if (opt == sOptionNames[Opt::OBFUSCATE])
                    {
                        if (val.toLower().startsWith("article"))
                        {
                            _obfuscateArticles = true;
                            qDebug() << "Do article obfuscation (the subject of each Article will be a UUID)\n";
                        }
                    }
                    else if (opt == sOptionNames[Opt::GROUP_POLICY])
                    {
                        val = val.toLower();
                        if (val == sGroupPolicies[GROUP_POLICY::EACH_POST])
                        {
                            _groupPolicy = GROUP_POLICY::EACH_POST;
                            if (_debug)
                                _log(tr("Group Policy: one group per Post"));
                        }
                        else if (val == sGroupPolicies[GROUP_POLICY::EACH_FILE])
                        {
                            _groupPolicy = GROUP_POLICY::EACH_FILE;
                            if (_debug)
                                _log(tr("Group Policy: one group per File"));
                        }
                    }
                    else if (opt == sOptionNames[Opt::DISP_PROGRESS])
                    {
                        val = val.toLower();
                        if (val == "bar")
                        {
                            _dispProgressBar = true;
                            qDebug() << "Display progressbar bar\n";
                        }
                        else if (val == "files")
                        {
                            _dispFilesPosting = true;
                            qDebug() << "Display Files when start posting\n";
                        }
                    }
                    else if (opt == sOptionNames[Opt::MSG_ID])
                    {
                        sArticleIdSignature = val.toStdString();
                    }
                    else if (opt == sOptionNames[Opt::ARTICLE_SIZE])
                    {
                        int nb = val.toInt(&ok);
                        if (ok)
                            sArticleSize = nb;
                    }
                    else if (opt == sOptionNames[Opt::NB_RETRY])
                    {
                        ushort nb = val.toUShort(&ok);
                        if (ok)
                            NntpArticle::setNbMaxRetry(nb);
                    }
                    else if (opt == sOptionNames[Opt::FROM])
                    {
                        QRegularExpression email("\\w+@\\w+\\.\\w+");
                        if (!email.match(val).hasMatch())
                            val += "@ngPost.com";
                        val = escapeXML(val);
                        _from = val.toStdString();
                        _saveFrom = true;
                    }
                    else if (opt == sOptionNames[Opt::GEN_FROM])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                        {
                            _genFrom = true;
                            if (_debug)
                                _cout << tr("Generate new random poster for each post") << "\n" << MB_FLUSH;
                        }
                    }
                    else if (opt == sOptionNames[Opt::GROUPS])
                        updateGroups(val);

                    else if (opt == sOptionNames[Opt::LANG])
                        changeLanguage(val.toLower());

                    else if (opt == "queue")
                    {
                        val = val.toLower();
                        _queueEnabled = !(val == "off" || val == "false" || val == "0" || val == "no");
                    }
                    else if (opt == "notify_ok" || opt == "notify_errors" || opt == "notify_tray")
                    {
                        val = val.toLower();
                        bool on = (val == "on" || val == "true" || val == "1" || val == "yes");
                        (opt == "notify_ok" ? _notifier.settings().onSuccess : opt == "notify_errors" ? _notifier.settings().onErrors : _notifier.settings().tray) = on;
                    }
                    else if (opt == "guided_post")
                    {
                        _guidedHasChoices = PostRequest::fromChoicesLine(val.trimmed(), _guidedChoices);
                    }
                    else if (opt == "guided_skip_where")
                    {
                        val = val.toLower();
                        _guidedSkipWhere = (val == "on" || val == "true" || val == "1" || val == "yes");
                    }
                    else if (opt == "check_updates")
                    {
                        val = val.toLower();
                        _checkUpdates = !(val == "off" || val == "false" || val == "0" || val == "no");
                    }
                    else if (opt == "update_last_check")
                        _updateLastCheck = val.trimmed();
                    else if (opt == "notify_when")
                        _notifier.settings().allDone = (val.trimmed().toLower() != "each");
                    else if (opt == "notify_discord")
                        _notifier.settings().discord = val.trimmed();
                    else if (opt == "notify_telegram_token")
                        _notifier.settings().telegramToken = val.trimmed();
                    else if (opt == "notify_telegram_chat")
                        _notifier.settings().telegramChat = val.trimmed();
                    else if (opt == "notify_webhook")
                        _notifier.settings().webhook = val.trimmed();
                    else if (opt == "notify_slack")
                        _notifier.settings().slack = val.trimmed();
                    else if (opt == "notify_ntfy")
                        _notifier.settings().ntfy = val.trimmed();
                    else if (opt == "notify_ntfy_token")
                        _notifier.settings().ntfyToken = val.trimmed();
                    else if (opt == "notify_gotify_url")
                        _notifier.settings().gotifyUrl = val.trimmed();
                    else if (opt == "notify_gotify_token")
                        _notifier.settings().gotifyToken = val.trimmed();
                    else if (opt == "notify_pushover_token")
                        _notifier.settings().pushoverToken = val.trimmed();
                    else if (opt == "notify_pushover_user")
                        _notifier.settings().pushoverUser = val.trimmed();
                    else if (opt == "notify_email_host")
                        _notifier.settings().emailHost = val.trimmed();
                    else if (opt == "notify_email_port")
                        _notifier.settings().emailPort = static_cast<quint16>(val.trimmed().toUInt());
                    else if (opt == "notify_email_security")
                        _notifier.settings().emailSecurity = val.trimmed();
                    else if (opt == "notify_email_user")
                        _notifier.settings().emailUser = val.trimmed();
                    else if (opt == "notify_email_pass")
                        _notifier.settings().emailPass = val.trimmed();
                    else if (opt == "notify_email_from")
                        _notifier.settings().emailFrom = val.trimmed();
                    else if (opt == "notify_email_to")
                        _notifier.settings().emailTo = val.trimmed();
                    else if (opt == "move_posted")
                        _movePosted = val.trimmed();
                    else if (opt == "quick_options")
                        _quickOptionsSet = PostProfile::fromOptionsLine(val.trimmed(), _quickOptions);
                    else if (opt == "quick_profile")
                        _quickProfile = PostProfile::cleanName(val);
                    else if (opt == "auto_profile")
                        _autoProfile = PostProfile::cleanName(val);
                    else if (opt == "auto_options")
                        _autoOptionsSet = PostProfile::fromOptionsLine(val.trimmed(), _autoOptions);
                    else if (opt == "quick_targets")
                        _quickOptions.targets = PostTargets::fromLine(val.trimmed());
                    else if (opt == "auto_targets")
                        _autoOptions.targets = PostTargets::fromLine(val.trimmed());
                    else if (opt == "watch_folder")
                    {
                        if (!val.trimmed().isEmpty() && !_watchFolders.contains(val.trimmed()))
                            _watchFolders << val.trimmed();
                    }
                    else if (opt == "favorite_groups")
                    {
                        _favoriteGroups.clear();
                        for (const QString &g : val.split(',', Qt::SkipEmptyParts))
                            if (!g.trimmed().isEmpty() && !_favoriteGroups.contains(g.trimmed().toLower()))
                                _favoriteGroups << g.trimmed().toLower();
                    }
                    else if (opt == "name_counter")
                        _nameCounter = val.trimmed().toUInt();
                    else if (opt == "auto_mode")
                    {
                        QString v = val.trimmed().toLower();
                        if (v == "choose" || v == "scan" || v == "watch")
                            _autoMode = v;
                    }
                    else if (opt == "nfo_style")
                    {
                        bool known = false;
                        NfoWriter::Style style = NfoWriter::styleFromKey(val, &known);
                        if (known)
                            _nfoStyle = NfoWriter::styleKey(style);
                    }
                    else if (opt == "gen_nfo" || opt == "nfo_in_post" || opt == "nfo_password" || opt == "nfo_media" || opt == "nfo_ascii")
                    {
                        val = val.toLower();
                        bool on = (val == "on" || val == "true" || val == "1" || val == "yes");
                        (opt == "gen_nfo" ? _genNfo : opt == "nfo_in_post" ? _nfoInPost : opt == "nfo_media" ? _nfoMedia : opt == "nfo_ascii" ? _nfoAscii : _nfoPassword) = on;
                    }
                    else if (opt == "profile")
                    {
                        PostProfile p;
                        if (PostProfile::fromLine(val, p))
                            _profiles.append(p);
                    }
                    else if (opt == "mediainfo_path")
                    {
                        _mediaInfoPath = val.trimmed();
                    }
                    else if (opt == "nfo_text")
                    {
                        _nfoText = val.replace("\\n", "\n"); // saved on one line
                    }
                    else if (opt == "max_speed")
                    {
                        double mbps = val.trimmed().replace(',', '.').toDouble(&ok);
                        if (ok && mbps >= 0)
                        {
                            if (mbps > 0)
                                _maxSpeed = mbps;
                            if (!_limitSpeedKey)
                                _limitSpeed = mbps > 0; // max_speed alone (Docker): the limit is on
                        }
                    }
                    else if (opt == "limit_speed")
                    {
                        val = val.toLower();
                        _limitSpeed = (val == "on" || val == "true" || val == "1" || val == "yes");
                        _limitSpeedKey = true;
                    }
                    else if (opt == "warn_duplicates")
                    {
                        val = val.toLower();
                        _checkDuplicates = !(val == "off" || val == "false" || val == "0" || val == "no");
                    }
                    else if (opt == "resume_queue")
                    {
                        val = val.toLower();
                        _resumeQueue = (val == "on" || val == "true" || val == "1" || val == "yes");
                    }

                    else if (opt == "history")
                    {
                        val = val.toLower();
                        _historyEnabled = !(val == "off" || val == "false" || val == "0" || val == "no");
                        _historyKeyInConfig = true;
                    }

                    else if (opt == "theme")
                    {
                        val = val.toLower();
                        if (val == "light" || val == "dark" || val == "auto")
                            _theme = val;
                    }

                    else if (opt == sOptionNames[Opt::SHUTDOWN_CMD])
                        _shutdownCmd = val;

                    else if (opt == sOptionNames[Opt::PROXY_SOCKS5])
                    {
                        QRegularExpression regExp(sProxyStrRegExp,  QRegularExpression::CaseInsensitiveOption);
                        QRegularExpressionMatch match = regExp.match(val);
                        if (match.hasMatch())
                        {
                            _proxyUrl = val;
                            // "^(([^:]+):([^@]+)@)?([\\w\\.\\-_]+):(\\d+)$";
                            QString user  = match.captured(2);
                            QString pass  = match.captured(3);
                            QString host  = match.captured(4);
                            ushort  port  = match.captured(5).toUShort();
                            _proxySocks5 = QNetworkProxy(QNetworkProxy::Socks5Proxy, host, port, user, pass);
                            QNetworkProxy::setApplicationProxy(_proxySocks5);
                        }
                        else
                            err += tr("Error parsing Proxy Socks5 parameters. The syntax should be: %1").arg(sProxyStrRegExp);

                    }

                    else if (opt == sOptionNames[Opt::NZB_POST_CMD])
                        _nzbPostCmd << args.join("=").trimmed();

                    else if (opt == sOptionNames[Opt::INPUT_DIR])
                        _inputDir = val;

                    else if (opt == sOptionNames[Opt::POST_HISTORY])
                    {
                        _postHistoryFile = val;
                        QFileInfo fi(val);
                        if (fi.isDir())
                            err += tr("the post history '%1' can't be a directory...\n").arg(val);
                        else
                        {
                            if (fi.exists())
                            {
                                if (!fi.isWritable())
                                    err += tr("the post history file '%1' is not writable...\n").arg(val);
                            }
                            else if (!QFileInfo(fi.absolutePath()).isWritable())
                                err += tr("the post history file '%1' is not writable...\n").arg(val);
                        }
                    }

                    else if (opt == sOptionNames[Opt::FIELD_SEPARATOR])
                        _historyFieldSeparator = val;

                    // compression section

#ifdef __USE_TMP_RAM__
                    else if (opt == sOptionNames[Opt::TMP_RAM])
                    {
                        _ramPath = val;
                        QFileInfo fi(_ramPath);
                        if (!fi.isDir())
                            err += QString("%1 %2\n").arg(sOptionNames[Opt::TMP_RAM].toUpper()).arg(tr("should be a directory!..."));
                        else if (!fi.isWritable())
                            err += QString("%1 %2\n").arg(sOptionNames[Opt::TMP_RAM].toUpper()).arg(tr("should be writable!..."));
                        else
                        {
                            _storage = new QStorageInfo(_ramPath);

                            _log(tr("Using RAM Storage %1, root: %2, type: %3, size: %4, available: %5").arg(
                                     _ramPath).arg(
                                     _storage->rootPath()).arg(
                                     QString(_storage->fileSystemType())).arg(
                                     PostingJob::humanSize(static_cast<double>(_storage->bytesTotal()))).arg(
                                     PostingJob::humanSize(static_cast<double>(_storage->bytesAvailable()))));
                        }
                    }
                    else if (opt == sOptionNames[Opt::TMP_RAM_RATIO])
                    {
                        double ratio = val.toDouble(&ok);
                        if (!ok || ratio < sRamRatioMin || ratio > sRamRatioMax)
                            err += QString("%1 %2\n").arg(
                                        sOptionNames[Opt::TMP_RAM_RATIO].toUpper()).arg(
                                        tr("should be a ratio between %1 and %2").arg(sRamRatioMin).arg(sRamRatioMax));
                        else
                            _ramRatio = ratio;
                    }
#endif
                    else if (opt == sOptionNames[Opt::TMP_DIR])
                        _tmpPath = val;
                    else if (opt == sOptionNames[Opt::RAR_PATH])
                        _rarPath = val;
                    else if (opt == sOptionNames[Opt::RAR_PASS])
                    {
                        _rarPassFixed = val;
                        _rarPass      = val;
                    }
                    else if (opt == sOptionNames[Opt::RAR_EXTRA])
                        _rarArgs = val;
                    else if (opt == sOptionNames[Opt::RAR_SIZE])
                    {
                        uint nb = val.toUInt(&ok);
                        if (ok)
                            _rarSize = nb;
                    }
                    else if (opt == sOptionNames[Opt::RAR_MAX])
                    {
                        _useRarMax = true;
                        uint nb = val.toUInt(&ok);
                        if (ok)
                            _rarMax = nb;
                    }
                    else if (opt == sOptionNames[Opt::KEEP_RAR])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _keepRar = true;
                    }
                    else if (opt == sOptionNames[Opt::AUTO_COMPRESS])
                    {
                        if (useHMI())
                            _log(tr("obsolete keyword AUTO_COMPRESS, you should use PACK instead, please click SAVE to update your conf and then go check it."));
                        else
                            _log(tr("obsolete keyword AUTO_COMPRESS, you should use PACK instead, please refer to the conf example: %1").arg(
                                     "https://github.com/mbruel/ngPost/blob/master/ngPost.conf#L140"));
                    }
                    else if (opt == sOptionNames[Opt::PACK])
                    {
                        val = val.toLower();
                        QStringList packKeywords = val.split(","), wrongKeywords,
                                allowedKeywords = {sOptionNames[Opt::COMPRESS], sOptionNames[Opt::GEN_NAME],
                                                    sOptionNames[Opt::GEN_PASS], sOptionNames[Opt::GEN_PAR2]};
                        for (auto it = packKeywords.cbegin(), itEnd = packKeywords.cend(); it != itEnd; ++it)
                        {
                            QString keyWord = (*it).trimmed();
                            if (allowedKeywords.contains(keyWord))
                                _packAutoKeywords << keyWord;
                            else
                                wrongKeywords << keyWord.toUpper();
                        }

                        if (wrongKeywords.size())
                            err += tr("Wrong keywords for PACK: %1. It should be a subset of (%2)").arg(
                                        wrongKeywords.join(", "), allowedKeywords.join(", ").toUpper());
                        else if (useHMI())
                            enableAutoPacking();
                    }
                    else if (opt == sOptionNames[Opt::RAR_NO_ROOT_FOLDER])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            _rarNoRootFolder = true;
                    }

                    else if (opt == sOptionNames[Opt::PAR2_PCT])
                    {
                        uint nb = val.toUInt(&ok);
                        if (ok)
                            _par2Pct = nb;
                    }
                    else if (opt == sOptionNames[Opt::PAR2_PATH])
                    {
                        if (!val.isEmpty())
                        {
                            QFileInfo fi(val);
                            if (fi.exists() && fi.isFile() && fi.isExecutable())
                            {
                                _par2Path       = val;
                                _par2PathConfig = val;
                            }
                        }
                    }
                    else if (opt == sOptionNames[Opt::PAR2_ARGS])
                        _par2Args = val;
                    else if (opt == sOptionNames[Opt::LENGTH_NAME])
                    {
                        uint nb = val.toUInt(&ok);
                        if (ok)
                            _lengthName = nb;
                    }
                    else if (opt == sOptionNames[Opt::LENGTH_PASS])
                    {
                        uint nb = val.toUInt(&ok);
                        if (ok)
                            _lengthPass = nb;
                    }





                    // Server Section under
                    else if (opt == sOptionNames[Opt::HOST])
                    {
                        serverParams->host = val;
                    }
                    else if (opt == sOptionNames[Opt::PORT])
                    {
                        ushort nb = val.toUShort(&ok);
                        if (ok)
                            serverParams->port = nb;

                    }
                    else if (opt == sOptionNames[Opt::SSL])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                        {
                            serverParams->useSSL = true;
                            if (serverParams->port == NntpServerParams::sDefaultPort)
                                serverParams->port = NntpServerParams::sDefaultSslPort;
                        }
                    }
                    else if (opt == sOptionNames[Opt::ENABLED])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            serverParams->enabled = true;
                        else
                            serverParams->enabled = false;
                    }
                    else if (opt == sOptionNames[Opt::NZBCHECK])
                    {
                        val = val.toLower();
                        if (val == "true" || val == "on" || val == "1")
                            serverParams->nzbCheck = true;
                        else
                            serverParams->nzbCheck = false;
                    }
                    else if (opt == sOptionNames[Opt::USER])
                    {
                        serverParams->user = val.toStdString();
                        serverParams->auth = true;
                    }
                    else if (opt == sOptionNames[Opt::PASS])
                    {
                        serverParams->pass = val.toStdString();
                        serverParams->auth = true;
                    }
                    else if (opt == sOptionNames[Opt::CONNECTION])
                    {
                        int nb = val.toInt(&ok);
                        if (ok)
                            serverParams->nbCons = nb;
                    }               
                }
            }
        }
        file.close();
    }
    applySpeedLimit();

    return err;
}

void NgPost::applySpeedLimit()
{
    SpeedLimiter::setLimit(_limitSpeed && _maxSpeed > 0 ? static_cast<qint64>(_maxSpeed * 1024. * 1024.) : 0);
}

QString NgPost::profilesText() const
{
    QString text;
    for (const PostProfile &p : _profiles)
        text += "profile = " + p.toLine() + "\n";
    return text;
}

QString NgPost::defaultConfigPath()
{
#if defined(WIN32) || defined(__MINGW64__)
    // 1. ngPost.conf in the working folder: the portable package (ZIP) and the previous versions
    if (QFileInfo(sDefaultConfig).isFile())
        return sDefaultConfig;
    // 2. next to the program
    QString nextToExe = QString("%1/%2").arg(QCoreApplication::applicationDirPath(), sDefaultConfig);
    if (QFileInfo(nextToExe).isFile())
        return nextToExe;
    // 3. the installed program (C:\Program Files is read only): the user's folder %APPDATA%\ngPost Deluxe
    QString appData = qEnvironmentVariable("APPDATA");
    if (appData.isEmpty())
        appData = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString dir = QString("%1/ngPost Deluxe").arg(appData);
    QDir().mkpath(dir);
    return QString("%1/%2").arg(dir, sDefaultConfig);
#else
    return QString("%1/%2").arg(getenv("HOME")).arg(sDefaultConfig);
#endif
}

QString NgPost::parseDefaultConfig()
{
    QString conf = defaultConfigPath();
    QString err;
    QFileInfo defaultConf(conf);
    if (defaultConf.exists() && defaultConf.isFile())
    {
        if (!_quiet)
            _cout << tr("Using default config file: %1").arg(conf) << "\n" << MB_FLUSH;
        err = _parseConfig(conf);
    }
    else
    {
        _freshInstall = true;
        qCritical() << "The default config file doesn't exist: " << conf;
    }

#if defined(WIN32) || defined(__MINGW64__)
    // no nzb folder defined and the working folder is not writable (program installed in C:\Program Files)
    if (_nzbPath.isEmpty() && !QFileInfo(QDir::currentPath()).isWritable())
    {
        _nzbPath = _nzbPathConf = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir().mkpath(_nzbPath);
    }

    // 7-Zip shipped with the program (folder "7-Zip" next to ngPostDeluxe.exe): used when no valid path is defined
    QString bundled7z = QString("%1/7-Zip/7z.exe").arg(QCoreApplication::applicationDirPath());
    if (QFileInfo(bundled7z).isFile() && (_rarPath.isEmpty() || !QFileInfo(_rarPath).isFile()))
        _rarPath = bundled7z;
#endif

    return err;
}

QString NgPost::configFilePath() const
{
    return _configPath.isEmpty() ? QFileInfo(defaultConfigPath()).absoluteFilePath() : _configPath;
}

bool NgPost::exportSettings(const QString &path, const SettingsTransfer::ExportOptions &options, QString &err) const
{
    QFile conf(configFilePath());
    QString text;
    if (conf.open(QIODevice::ReadOnly | QIODevice::Text))
        text = QString::fromUtf8(conf.readAll());
    else if (!_freshInstall)
    {
        err = tr("Can't read the config file %1").arg(conf.fileName());
        return false;
    }
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        err = tr("Can't write the file %1").arg(path);
        return false;
    }
    out.write(SettingsTransfer::exportText(text, options, sAppName, versionLabel(), QDateTime::currentDateTime()).toUtf8());
    if (!out.commit())
    {
        err = tr("Can't write the file %1").arg(path);
        return false;
    }
    return true;
}

bool NgPost::importSettings(const QString &path, const SettingsTransfer::ImportOptions &options, bool apply,
                            SettingsTransfer::Summary &summary, QString &err)
{
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        err = tr("Can't read the file %1").arg(path);
        return false;
    }
    SettingsTransfer::Parsed parsed = SettingsTransfer::parse(QString::fromUtf8(in.readAll()));
    if (!parsed.valid)
    {
        err = tr("%1 is not a file of settings exported by ngPost Deluxe").arg(path);
        return false;
    }
    QString confPath = configFilePath();
    QString text;
    QFile conf(confPath);
    if (conf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        text = QString::fromUtf8(conf.readAll());
        conf.close();
    }
    QString merged = SettingsTransfer::merge(text, parsed, options, &summary);
    if (!apply)
        return true;

    if (QFileInfo::exists(confPath))
    {
        QString backup = confPath + ".before-import";
        QFile::remove(backup);
        if (!QFile::copy(confPath, backup))
        {
            err = tr("Can't make a copy of the config file: nothing was changed");
            return false;
        }
    }
    QDir().mkpath(QFileInfo(confPath).absolutePath());
    QSaveFile out(confPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        err = tr("Can't write the config file %1").arg(confPath);
        return false;
    }
    out.write(merged.toUtf8());
    if (!out.commit())
    {
        err = tr("Can't write the config file %1").arg(confPath);
        return false;
    }
    return true;
}

QString NgPost::watchFoldersText() const
{
    QString out;
    for (const QString &f : _watchFolders)
        out += "watch_folder = " + f + "\n";
    if (out.isEmpty())
        out = "#watch_folder = \n";
    return out;
}

void NgPost::saveConfig(bool quiet)
{
    if (_settingsImported)
        return;
    QString conf = defaultConfigPath();
    if (_configPath.isEmpty())
        _configPath = QFileInfo(conf).absoluteFilePath();
    _historyKeyInConfig = true; // the key "history" is written below

    // the whole file is rewritten: keep a copy of the previous one, once per session (ngPost.conf.bak)
    static bool backupDone = false;
    if (!backupDone && QFileInfo::exists(conf))
    {
        QString backup = conf + ".bak";
        QFile::remove(backup);
        if (QFile::copy(conf, backup))
            backupDone = true;
    }

    // QSaveFile writes in a temporary file and replaces the config only when everything is written
    // (the config can't be left half written in case of crash or full disk)
    QSaveFile file(conf);
    if (file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << tr("# ngPost configuration file") << "\n"
               << "#\n"
               << "#\n"
               << "\n"
               << tr("## Lang for the app. Currently supported: EN, FR, ES, DE, NL, PT, ZH, IT") << "\n"
               << "lang = " << _lang.toUpper() << "\n"
               << "\n"
               << tr("## interface theme: AUTO (same as Windows), LIGHT or DARK") << "\n"
               << "theme = " << _theme.toUpper() << "\n"
               << "\n"
               << tr("## use Proxy (only Socks5 type!)") << "\n"
               << (_proxyUrl.isEmpty()  ? "#PROXY_SOCKS5 = user:pass@192.168.1.1:5555" : _proxyUrl)  << "\n"
               << "\n"
               << tr("## destination folder for all your nzb") << "\n"
               << tr("## if you don't put anything, the nzb will be generated in the folder of ngPost on Windows and in /tmp on Linux") << "\n"
               << tr("## this will be overwritten if you use the option -o with the full path of the nzb") << "\n"
               << "nzbPath  = " << (_nzbPath.isEmpty() ? _nzbPathConf : _nzbPath) << "\n"
               << "\n"
               << tr("## Shutdown command to switch off the computer when ngPost is done with all its queued posting") << "\n"
               << tr("## this should mainly used with the auto posting") << "\n"
               << tr("## you could use whatever script instead (like to send a mail...)") << "\n"
               << tr("#SHUTDOWN_CMD = shutdown /s /f /t 0  (Windows)") << "\n"
               << tr("#SHUTDOWN_CMD = sudo -n /sbin/poweroff  (Linux, make sure poweroff has sudo rights without any password or change the command)") << "\n"
               << tr("#SHUTDOWN_CMD = sudo -n shutdown -h now (MacOS, same make sure you've sudo rights)") << "\n"
               << "SHUTDOWN_CMD = " << _shutdownCmd << "\n"
               << "\n"
               << tr("## upload the nzb to a specific URL") << "\n"
               << tr("## only http, https or ftp (neither ftps or sftp are supported)") << "\n"
               << tr("#NZB_UPLOAD_URL = ftp://user:pass@url_or_ip:21") << "\n"
               << (_urlNzbUploadStr.isEmpty() ? QString() : QString("NZB_UPLOAD_URL = %1\n").arg(_urlNzbUpload->url()))
               << "\n"
               << tr("## execute a command or script at the end of each post (see examples)") << "\n"
               << tr("## you can use several post commands by defining several NZB_POST_CMD") << "\n"
               << tr("## here is the list of the available placehoders") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::nzbPath] <<"          : " << tr("full path of the written nzb file") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::nzbName] <<"          : " << tr("name of the nzb without the extension (original source name)") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::rarName] <<"          : " << tr("name of the archive files (in case of obfuscation)") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::rarPass] <<"          : " << tr("archive password") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::sizeInByte] <<"       : " << tr("size of the post (before yEnc encoding)") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::groups] <<"           : " << tr("list of groups (comma separated)") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::nbFiles] <<"          : " << tr("number of files in the post") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::nbArticles] <<"       : " << tr("number of Articles") << "\n"
               << "##   "<< sPostCmdPlaceHolders[PostCmdPlaceHolders::nbArticlesFailed] <<" : " << tr("number of Articles that failed to be posted") << "\n"
               << "#\n"
               << "#NZB_POST_CMD = scp \"__nzbPath__\" myBox.com:~/nzbs/\n"
               << "#NZB_POST_CMD = zip \"__nzbPath__.zip\" \"__nzbPath__\"\n"
               << "#NZB_POST_CMD = ~/scripts/postNZB.sh \"__nzbPath__\" \"__groups__\" __rarName__ __rarPass__ __sizeInByte__ __nbFiles__ __nbArticles__ __nbArticlesFailed__\n"
               << "#NZB_POST_CMD = mysql -h localhost -D myDB -u myUser -pmyPass-e \"INSERT INTO POST (release, rarName, rarPass, size) VALUES('__nzbName__', '__rarName__', '__rarPass__', '__sizeInByte__')\"\n"
               << "#NZB_POST_CMD = cmd.exe /C move \"__nzbPath__\" \"C:\\ngPost\\nzb\\__nzbName__{{__rarPass__}}.nzb\"\n"
               << "#NZB_POST_CMD = curl -X POST -F \"file=@__nzbPath__\" -F \"api=12345\" -F \"cat=45\" -F \"private=no\" https://usenet.com/post-api\n"
               << "" ;
        for (const QString &nzbPostCmd : _nzbPostCmd)
            stream << "NZB_POST_CMD = " << nzbPostCmd << "\n";
        stream << "\n"
               << tr("## nzb files are normally all created in nzbPath") << "\n"
               << tr("## but using this option, the nzb of each monitoring folder will be stored in their own folder (created in nzbPath)") << "\n"
               << (_monitor_nzb_folders  ? "" : "#") << "MONITOR_NZB_FOLDERS = true\n"
               << "\n"
               << tr("## for monitoring, extension file filter for new incoming files (coma separated, no dot)") << "\n"
               << (_monitorExtensions.isEmpty()  ? "#" : "") << "MONITOR_EXTENSIONS = "
               << (_monitorExtensions.isEmpty() ? "mkv,mp4,avi,zip,tar,gz,iso" : _monitorExtensions.join(",")) << "\n"
               << "\n"
               << tr("## for monitoring, ignore new incoming folders") << "\n"
               << (_monitorIgnoreDir  ? "" : "#") << "MONITOR_IGNORE_DIR = true\n"
               << tr("## for monitoring, delay to check the size of an incoming file/folder to make sure it is fully arrived before posting it") << "\n"
               << tr("## must be between 1sec and 120sec (otherwise default: 1sec)") << "\n"
               << "MONITOR_SEC_DELAY_SCAN = "  << _monitorSecDelayScan << "\n"
               << "\n\n"
               << tr("## Default folder to open to select files from the HMI") << "\n"
               << "inputDir = " << _inputDir << "\n"
               << "\n"
               << tr("## History of the uploads (shown in the History page): ON or OFF") << "\n"
               << tr("## the file contains the archive passwords in clear text: keep it private") << "\n"
               << "history = " << (_historyEnabled ? "ON" : "OFF") << "\n"
               << tr("## by default the history is saved in ngPost_history.csv next to this file") << "\n"
               << "\n"
               << "## Profiles of options (GUI), one line each: profile = name|compress|random name|random password|par2|par2 %|keep the archives (1 or 0)" << "\n"
               << "## optionally followed by |newsgroups|servers|NZB folder|name template (empty = not in the profile)" << "\n"
               << profilesText()
               << "\n"
               << tr("## Notifications when the posts end: events (notify_ok, notify_errors), system notification (notify_tray, GUI only),") << "\n"
               << tr("## notify_when = all_done (one message when all the posts are done) or each (after each post)") << "\n"
               << tr("## Discord / Telegram / Slack / ntfy / Gotify / Pushover / email / webhook addresses, tokens and passwords are secrets, like the passwords: keep this file private") << "\n"
               << "notify_ok = " << (_notifier.settings().onSuccess ? "ON" : "OFF") << "\n"
               << "notify_errors = " << (_notifier.settings().onErrors ? "ON" : "OFF") << "\n"
               << "notify_tray = " << (_notifier.settings().tray ? "ON" : "OFF") << "\n"
               << "notify_when = " << (_notifier.settings().allDone ? "all_done" : "each") << "\n"
               << (_notifier.settings().discord.isEmpty() ? "#" : "") << "notify_discord = " << _notifier.settings().discord << "\n"
               << (_notifier.settings().telegramToken.isEmpty() ? "#" : "") << "notify_telegram_token = " << _notifier.settings().telegramToken << "\n"
               << (_notifier.settings().telegramChat.isEmpty() ? "#" : "") << "notify_telegram_chat = " << _notifier.settings().telegramChat << "\n"
               << (_notifier.settings().webhook.isEmpty() ? "#" : "") << "notify_webhook = " << _notifier.settings().webhook << "\n"
               << (_notifier.settings().slack.isEmpty() ? "#" : "") << "notify_slack = " << _notifier.settings().slack << "\n"
               << (_notifier.settings().ntfy.isEmpty() ? "#" : "") << "notify_ntfy = " << _notifier.settings().ntfy << "\n"
               << (_notifier.settings().ntfyToken.isEmpty() ? "#" : "") << "notify_ntfy_token = " << _notifier.settings().ntfyToken << "\n"
               << (_notifier.settings().gotifyUrl.isEmpty() ? "#" : "") << "notify_gotify_url = " << _notifier.settings().gotifyUrl << "\n"
               << (_notifier.settings().gotifyToken.isEmpty() ? "#" : "") << "notify_gotify_token = " << _notifier.settings().gotifyToken << "\n"
               << (_notifier.settings().pushoverToken.isEmpty() ? "#" : "") << "notify_pushover_token = " << _notifier.settings().pushoverToken << "\n"
               << (_notifier.settings().pushoverUser.isEmpty() ? "#" : "") << "notify_pushover_user = " << _notifier.settings().pushoverUser << "\n"
               << (_notifier.settings().emailHost.isEmpty() ? "#" : "") << "notify_email_host = " << _notifier.settings().emailHost << "\n"
               << (_notifier.settings().emailHost.isEmpty() ? "#" : "") << "notify_email_port = " << _notifier.settings().emailPort << "\n"
               << (_notifier.settings().emailHost.isEmpty() ? "#" : "") << "notify_email_security = " << _notifier.settings().emailSecurity << "\n"
               << (_notifier.settings().emailUser.isEmpty() ? "#" : "") << "notify_email_user = " << _notifier.settings().emailUser << "\n"
               << (_notifier.settings().emailPass.isEmpty() ? "#" : "") << "notify_email_pass = " << _notifier.settings().emailPass << "\n"
               << (_notifier.settings().emailFrom.isEmpty() ? "#" : "") << "notify_email_from = " << _notifier.settings().emailFrom << "\n"
               << (_notifier.settings().emailTo.isEmpty() ? "#" : "") << "notify_email_to = " << _notifier.settings().emailTo << "\n"
               << "\n"
               << tr("## New versions: the GUI looks once a day for a newer release on GitHub and only tells you (nothing is downloaded)") << "\n"
               << "check_updates = " << (_checkUpdates ? "ON" : "OFF") << "\n"
               << (_updateLastCheck.isEmpty() ? "#" : "") << "update_last_check = " << _updateLastCheck << "\n"
               << "\n"
               << tr("## Guided Post: the last choices (kind of post, options, what happens after the post) and whether to skip the step \"Where to post\" when everything is set") << "\n"
               << (_guidedHasChoices ? "" : "#") << "guided_post = " << (_guidedHasChoices ? _guidedChoices.choicesLine() : QString()) << "\n"
               << "guided_skip_where = " << (_guidedSkipWhere ? "ON" : "OFF") << "\n"
               << "\n"
               << tr("## Quick Post and Auto Posting: the options of the last session (compress|random name|random password|par2|par2 %|keep the archives, 1 or 0)") << "\n"
               << (_quickOptionsSet ? "" : "#") << "quick_options = " << _quickOptions.optionsLine() << "\n"
               << (_autoOptionsSet ? "" : "#") << "auto_options = " << _autoOptions.optionsLine() << "\n"
               << "## " << tr("Profile chosen when the program was closed (a name of the list of profiles, Recommended, or empty = Custom)") << "\n"
               << (_quickProfile.isEmpty() ? "#" : "") << "quick_profile = " << _quickProfile << "\n"
               << (_autoProfile.isEmpty() ? "#" : "") << "auto_profile = " << _autoProfile << "\n"
               << "## " << tr("Newsgroups, servers, NZB folder and name template of the last session (groups|servers|NZB folder|template, empty = the ones of the Settings)") << "\n"
               << (_quickOptions.targets.isEmpty() ? "#" : "") << "quick_targets = " << _quickOptions.targets.toLine() << "\n"
               << (_autoOptions.targets.isEmpty() ? "#" : "") << "auto_targets = " << _autoOptions.targets.toLine() << "\n"
               << "## " << tr("Newsgroups marked as favorites (shown first in the suggestions)") << "\n"
               << (_favoriteGroups.isEmpty() ? "#" : "") << "favorite_groups = " << _favoriteGroups.join(",") << "\n"
               << "## " << tr("Folders watched by Auto Posting when the program was closed (offered again at the next start), one line each") << "\n"
               << watchFoldersText()
               << "## " << tr("Last number used by {n} in a name template") << "\n"
               << "name_counter = " << _nameCounter << "\n"
               << "\n"
               << tr("## Auto Posting: last mode used (choose, scan or watch) and the folder where the posted files are moved") << "\n"
               << tr("## (a name like Posted is created next to each file/folder, a full path is used as it is; without it nothing is moved)") << "\n"
               << "auto_mode = " << _autoMode << "\n"
               << (_movePosted.isEmpty() ? "#" : "") << "move_posted = " << (_movePosted.isEmpty() ? QString("Posted") : _movePosted) << "\n"
               << "\n"
               << tr("## NFO file: written next to the nzb (gen_nfo), also posted with the files (nfo_in_post),")  << "\n"
               << tr("## with the password of the archive (nfo_password) and a text of yours (nfo_text, one line, \\n = new line)") << "\n"
               << "gen_nfo = " << (_genNfo ? "ON" : "OFF") << "\n"
               << "nfo_in_post = " << (_nfoInPost ? "ON" : "OFF") << "\n"
               << "nfo_password = " << (_nfoPassword ? "ON" : "OFF") << "\n"
               << "nfo_media = " << (_nfoMedia ? "ON" : "OFF") << "\n"
               << tr("## Look of the NFO file (nfo_style: classic, clean, box, scene, elegant or retro) and nfo_ascii to use only ASCII characters") << "\n"
               << "nfo_style = " << _nfoStyle << "\n"
               << "nfo_ascii = " << (_nfoAscii ? "ON" : "OFF") << "\n"
               << (_mediaInfoPath.isEmpty() ? "#" : "") << "mediainfo_path = " << _mediaInfoPath << "\n"
               << (_nfoText.trimmed().isEmpty() ? "#" : "") << "nfo_text = " << QString(_nfoText).replace("\r", "").replace("\n", "\\n") << "\n"
               << "\n"
               << tr("## Speed limit: maximum upload speed in MB/s for all the connections together (limit_speed ON or OFF, same as --max_speed)") << "\n"
               << "limit_speed = " << (_limitSpeed ? "ON" : "OFF") << "\n"
               << "max_speed = " << QString::number(_maxSpeed, 'g', 6) << "\n"
               << "\n"
               << tr("## Warn when a post is already in the history (same name and size): the GUI asks, the auto posting and the command line skip it (ON or OFF)") << "\n"
               << "warn_duplicates = " << (_checkDuplicates ? "ON" : "OFF") << "\n"
               << "\n"
               << tr("## Queue of the posts: the posts that are not finished are saved in ngPost_queue.json next to this file") << "\n"
               << tr("## so they can be posted again after a restart (ON or OFF). It contains the archive passwords in clear text") << "\n"
               << "queue = " << (_queueEnabled ? "ON" : "OFF") << "\n"
               << tr("## command line / Docker: post again the unfinished posts at the start (same as --resume_queue)") << "\n"
               << (_resumeQueue ? "" : "#") << "resume_queue = ON\n"
               << tr("## History posting file") << "\n"
               << tr("## each succesful post will append a line with the date, the file name, the archive name, the password...") << "\n"
               << (_postHistoryFile.isEmpty()  ? "#" : "") <<"POST_HISTORY = "
               << (_postHistoryFile.isEmpty()  ? "/nzb/ngPost_history.csv" : _postHistoryFile) << "\n"
               << "\n"
               << tr("## Character used to separate fields in the history posting file") << "\n"
               << (_historyFieldSeparator == QString(sDefaultFieldSeparator) ? "#" : "") << "FIELD_SEPARATOR = " << _historyFieldSeparator << "\n"
               << "\n"
               << "GROUPS   = " << _grpList.join(",") << "\n"
               << "\n"
               << tr("## If you give several Groups (comma separated) you've 3 policies for posting:") << "\n"
               << tr("##    ALL       : everything is posted on ALL the Groups") << "\n"
               << tr("##    EACH_POST : each Post will be posted on a random Group from the list") << "\n"
               << tr("##    EACH_FILE : each File will be posted on a random Group from the list") << "\n"
               << "GROUP_POLICY = " << sGroupPolicies[_groupPolicy].toUpper()
               << "\n"
               << "\n"
               << tr("## uncomment the next line if you want a fixed uploader email (in the nzb and in the header of each articles)") << "\n"
               << tr("## if you let it commented, we'll generate ONE random email for all the posts of the session") << "\n"
               << (_saveFrom  ? "" : "#") << "FROM = " << _from.c_str() << "\n"
               << "\n"
               << tr("## Generate new random poster for each post (--auto or --monitor)") << "\n"
               << tr("## if this option is set the FROM email just above will be ignored") << "\n"
               << (_genFrom  ? "" : "#") << "GEN_FROM = true" << "\n"
               << "\n"
               << "\n"
               << tr("## uncomment the next line to limit the number of threads,  (by default it'll use the number of cores)") << "\n"
               << tr("## all the connections are spread equally on those posting threads") << "\n"
               << "thread  =  " << _nbThreads << "\n"
               << "\n"
               << "\n"
               << tr("## How to display progressbar in command line: NONE, BAR, FILES") << "\n"
               << (_dispProgressBar  ? "" : "#") << "DISP_Progress = BAR\n"
               << (_dispFilesPosting ? "" : "#") << "DISP_Progress = FILES\n"
               << "\n"
               << "\n"
               << tr("## suffix of the msg_id for all the articles (cf nzb file)") << "\n"
               << (sArticleIdSignature == "ngPost" ? "#msg_id  =  ngPost\n" : QString("msg_id  =  %1\n").arg(sArticleIdSignature.c_str()))
               << "\n"
               << tr("## article size (default 700k)") << "\n"
               << "article_size = " << sArticleSize << "\n"
               << "\n"
               << tr("## number of retry to post an Article in case of failure (probably due to an already existing msg-id)") << "\n"
               << "retry = " << NntpArticle::nbMaxTrySending() << "\n"
               << "\n"
               << "\n"
               << tr("## uncomment the following line to obfuscate the subjects of each Article") << "\n"
               << tr("## /!\\ CAREFUL you won't find your post if you lose the nzb file /!\\") << "\n"
               << (_obfuscateArticles ? "" : "#") << "obfuscate = article\n"
               << "\n"
               << tr("## remove accents and special characters from the nzb file names") << "\n"
               << (_removeAccentsOnNzbFileName  ? "" : "#") << "NZB_RM_ACCENTS = true\n"
               << "\n"
               << tr("## close Quick Post Tabs when posted successfully (for the GUI)") << "\n"
               << (_autoCloseTabs  ? "" : "#") << "AUTO_CLOSE_TABS = true\n"
               << "\n"
               << "\n"
               << tr("## Time to wait (seconds) before trying to resume a Post automatically in case of loss of Network (min: %1)").arg(
                      sDefaultResumeWaitInSec) << "\n"
               << "RESUME_WAIT = " << _waitDurationBeforeAutoResume << "\n"
               << "\n"
               << tr("## By default, ngPost tries to resume a Post if the network is down.") << "\n"
               << tr("## it won't stop trying until the network is back and the post is finished properly") << "\n"
               << tr("## you can disable this feature and thus stop a post when you loose the network") << "\n"
               << (_tryResumePostWhenConnectionLost  ? "#" : "") << "NO_RESUME_AUTO = true\n"
               << "\n"
               << tr("## if there is no activity on a connection it will be closed and restarted") << "\n"
               << tr("## The duration is in second, default: %1, min: %2)").arg(sDefaultSocketTimeOut/1000).arg(sMinSocketTimeOut/1000) << "\n"
               << "SOCK_TIMEOUT = " << _socketTimeOut / 1000 << "\n"
               << "\n"
               << tr("## when several Posts are queued, prepare the packing of the next Post while uploading the current one") << "\n"
               << (_preparePacking ? "" : "#") << "PREPARE_PACKING = true" << "\n"
               << "\n"
               << tr("## For GUI ONLY, save the logs in a file (to debug potential crashes)") << "\n"
               << tr("## ~/ngPost.log on Linux and MacOS, in the executable folder for Windows") << "\n"
               << tr("## The log is overwritten each time ngPost is launched") << "\n"
               << tr("## => after a crash, please SAVE the log before relaunching ngPost") << "\n"
               << (_logStream != nullptr ? "" : "#") << "LOG_IN_FILE = true" << "\n"
               << "\n"
               << "\n"
               << "\n"
               << "\n"
               << "##############################################################\n"
               << "##           Compression and par2 section                   ##\n"
               << "##############################################################\n"
               << "\n"
               << tr("## Shortcut for automatic packing for both GUI and CMD using --pack") << "\n"
               << tr("## coma separated list using the keywords COMPRESS, GEN_NAME, GEN_PASS and GEN_PAR2") << "\n"
               << tr("## For Auto posting and Monitoring if you don't use COMPRESS you need GEN_PA2") << "\n"
               << tr("#PACK = COMPRESS, GEN_NAME, GEN_PASS, GEN_PAR2") << "\n"
               << tr("#PACK = GEN_PAR2") << "\n"
               << (_packAuto && _packAutoKeywords.size() ? QString("PACK = %1\n").arg(_packAutoKeywords.join(", ").toUpper()) : "")
               << "\n"
               << tr("## use the same Password for all your Posts using compression") << "\n"
          #ifdef __USE_HMI__
               << (_hmi?(_hmi->useFixedPassword()?"":"#"):(_rarPassFixed.isEmpty()  ? "#" : ""))
          #else
               << (_rarPassFixed.isEmpty()  ? "#" : "")
          #endif
               << "RAR_PASS = " << (_rarPassFixed.isEmpty()  ? "yourPassword" : _rarPassFixed) << "\n"
               << "\n"
               << tr("## temporary folder where the compressed files and par2 will be stored") << "\n"
               << tr("## so we can post directly a compressed (obfuscated or not) archive of the selected files") << "\n"
               << tr("## /!\\ The directory MUST HAVE WRITE PERMISSION /!\\") << "\n"
               << tr("## this is set for Linux environment, Windows users MUST change it") << "\n"
               << "TMP_DIR = " << _tmpPath << "\n"
               << "\n";
#ifdef __USE_TMP_RAM__
        stream << tr("## temporary folder with size constraint, typically a tmpfs partition") << "\n"
               << tr("## the size of a post multiply by TMP_RAM_RATIO must available on the disk") << "\n"
               << tr("## otherwise ngPost will use TMP_DIR (with no check there)") << "\n"
               << tr("## (uncomment and define TMP_RAM to activate the feature, make sure the path is writable)") << "\n"
               << (_ramPath.isEmpty() ? "#" : "") << "TMP_RAM = " << (_ramPath.isEmpty() ? "/mnt/ngPost_tmpfs" : _ramPath) << "\n"
               << "\n"
               << tr("## Ratio used on the source files size to compensate the par2 generation") << "\n"
               << tr("## min is 10% to be sure (so 1.1), max 2.0") << "\n"
               << "TMP_RAM_RATIO = " << _ramRatio << "\n"
               << "\n";
#endif
        stream << tr("## RAR or 7zip absolute file path (external application)") << "\n"
               << tr("## /!\\ The file MUST EXIST and BE EXECUTABLE /!\\") << "\n"
               << tr("## this is set for Linux environment, Windows users MUST change it") << "\n"
               << "RAR_PATH = " << _rarPath << "\n"
               << "\n"
               << tr("## RAR EXTRA options (the first 'a' and '-idp' will be added automatically)") << "\n"
               << tr("## -hp will be added if you use a password with --gen_pass, --rar_pass or using the HMI") << "\n"
               << tr("## -v42m will be added with --rar_size or using the HMI") << "\n"
               << tr("## you could change the compression level, lock the archive, add redundancy...") << "\n"
               << "#RAR_EXTRA = -ep1 -m0 -k -rr5p\n"
               << "#RAR_EXTRA = -mx0 -mhe=on   (for 7-zip)\n"
               << (_rarArgs.isEmpty() ? "" : QString("RAR_EXTRA = %1\n").arg(_rarArgs) )
               << "\n"
               << tr("## size in MB of the RAR volumes (0 by default meaning NO split)") << "\n"
               << tr("## feel free to change the value or to comment the next line if you don't want to split the archive") << "\n"
               << "RAR_SIZE = " << _rarSize << "\n"
               << "\n"
               << tr("## maximum number of archive volumes") << "\n"
               << tr("## we'll use RAR_SIZE except if it genereates too many volumes") << "\n"
               << tr("## in that case we'll update rar_size to be <size of post> / rar_max") << "\n"
               << "RAR_MAX = " << _rarMax << "\n"
               << "\n"
               << tr("##  keep rar folder after posting (otherwise it is automatically deleted uppon successful post)") << "\n"
               << (_keepRar  ? "" : "#") << "KEEP_RAR = true\n"
               << "\n"
               << "## " << tr("Remove root (parent) folder when compressing Folders using RAR") << "\n"
               << (_rarNoRootFolder  ? "" : "#") << "RAR_NO_ROOT_FOLDER = true\n"
               << "\n"
               << tr("## par2 redundancy percentage (0 by default meaning NO par2 generation)") << "\n"
               << "PAR2_PCT = " << _par2Pct << "\n"
               << "\n"
               << tr("## par2 (or alternative) absolute file path") << "\n"
               << tr("## this is only useful if you compile from source (as par2 is included on Windows and the AppImage)") << "\n"
               << tr("## or if you wish to use an alternative to par2 (for exemple Multipar on Windows)") << "\n"
               << tr("## (in that case, you may need to set also PAR2_ARGS)") << "\n";
        if (!_par2PathConfig.isEmpty())
            stream << "PAR2_PATH = " << _par2PathConfig << "\n";
#if defined(WIN32) || defined(__MINGW64__)
        stream << "#PAR2_PATH = <your_path>parpar.exe\n"
               << "#PAR2_PATH = <your_path>par2j64.exe\n";
#else
        stream << "#PAR2_PATH = /usr/bin/par2\n"
               << "#PAR2_PATH = <your_path>/parpar\n";
#endif
        stream << "\n"
               << tr("## fixed parameters for the par2 (or alternative) command") << "\n"
               << tr("## you could for exemple use Multipar on Windows") << "\n"
               << "#PAR2_ARGS = -s5M -r1n*0.6 -m2048M -p1l --progress stdout -q   (for parpar)\n"
               << "#PAR2_ARGS = c -l -m1024 -r8 -s768000                 (for par2cmdline)\n"
               << "#PAR2_ARGS = create /rr8 /lc40 /lr /rd2 /ss768000     (for Multipar)\n"
               << (_par2Args.isEmpty() ? "" : QString("PAR2_ARGS = %1\n").arg(_par2Args))
               << "\n"
               << "\n"
               << tr("## length of the random generated archive's file name") << "\n"
               << "LENGTH_NAME = " << _lengthName << "\n"
               << "\n"
               << tr("## length of the random archive's passsword") << "\n"
               << "LENGTH_PASS = "<< _lengthPass << "\n"
               << "\n"
               << "\n"
               << "\n"
               << "\n"
               << "##############################################################\n"
               << "##                   servers section                        ##\n"
               << "##############################################################\n"
               << "\n";

        for (NntpServerParams *param : _nntpServers)
        {
            stream << "[server]\n"
                   << "host = " << param->host << "\n"
                   << "port = " << param->port << "\n"
                   << "ssl  = " << (param->useSSL ? "true" : "false") << "\n"
                   << "user = " << param->user.c_str() << "\n"
                   << "pass = " << param->pass.c_str() << "\n"
                   << "connection = " << param->nbCons << "\n"
                   << "enabled = " << (param->enabled ? "true":"false") << "\n"
                   << "nzbCheck = " << (param->nzbCheck ? "true":"false") << "\n"
                   << "\n\n";
        }
        stream << tr("## You can add as many server if you have several providers by adding other \"server\" sections") << "\n"
               << "#[server]\n"
               << "#host = news.otherprovider.com\n"
               << "#port = 563\n"
               << "#ssl  = true\n"
               << "#user = myOtherUser\n"
               << "#pass = myOtherPass\n"
               << "#connection = 15\n"
               << "#enabled = false\n"
               << "#nzbCheck = false\n"
               << "\n";


        stream.flush();
        if (!file.commit())
            _error(tr("Error: Couldn't write default configuration file: %1").arg(conf));
        else if (!quiet)
            _log(tr("the config '%1' file has been updated").arg(conf));
    }
    else
        _error(tr("Error: Couldn't write default configuration file: %1").arg(conf));

}
