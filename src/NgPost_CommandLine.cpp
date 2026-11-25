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

// Command line: options, syntax, version and parameters (member functions of NgPost, split from NgPost.cpp)

bool NgPost::parseCommandLine(int argc, char *argv[])
{
    Q_UNUSED(argc)
    QString appVersion = QString("%1 v%2").arg(sAppName, versionLabel());
    QCommandLineParser parser;
    parser.setApplicationDescription(appVersion);
    parser.addOptions(sCmdOptions);


    // Process the actual command line arguments given by the user
    QStringList args = QCoreApplication::arguments();
    if (!parser.parse(args))
    {
#ifdef __DEBUG__
        qDebug() << "cmd args: " << args;
#endif
        _error(tr("Error syntax: %1\nTo list the available options use: %2 --help\n").arg(parser.errorText(), sAppName),
                ERROR_CODE::ERR_WRONG_ARG);
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::QUIET]))
        _quiet = true;

    if (parser.isSet(sOptionNames[Opt::CONF]))
    {
        QString err = _parseConfig(parser.value(sOptionNames[Opt::CONF]));
        if (!err.isEmpty())
        {
            _error(err, ERROR_CODE::ERR_CONF_FILE);
            return false;
        }
    }
    else
    {
        QString err = parseDefaultConfig();
        if (!err.isEmpty())
        {
            _error(err, ERROR_CODE::ERR_CONF_FILE);
            return false;
        }
    }

    if (_quiet)
    {
        _debug = 0;
        _dispProgressBar  = false;
        _dispFilesPosting = false;
    }

    if (parser.isSet(sOptionNames[Opt::LANG]))
    {
        QString lang = parser.value(sOptionNames[Opt::LANG]).toLower();
        _cout << "Lang: " << lang << "\n" << MB_FLUSH;
        changeLanguage(lang);
    }

    if (parser.isSet(sOptionNames[Opt::HELP]))
    {
        _showVersionASCII();
        _syntax(argv[0]);
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::EXPORT_SETTINGS]) || parser.isSet(sOptionNames[Opt::IMPORT_SETTINGS]))
    {
        QString err;
        if (parser.isSet(sOptionNames[Opt::EXPORT_SETTINGS]))
        {
            SettingsTransfer::ExportOptions all;
            all.servers = true;
            QString path = parser.value(sOptionNames[Opt::EXPORT_SETTINGS]);
            if (!exportSettings(path, all, err))
            {
                _error(err, ERROR_CODE::ERR_CONF_FILE);
                return false;
            }
            _cout << tr("Settings exported in %1 (no password in it)").arg(path) << "\n" << MB_FLUSH;
        }
        if (parser.isSet(sOptionNames[Opt::IMPORT_SETTINGS]))
        {
            SettingsTransfer::Summary sum;
            if (!importSettings(parser.value(sOptionNames[Opt::IMPORT_SETTINGS]), SettingsTransfer::ImportOptions(), true, sum, err))
            {
                _error(err, ERROR_CODE::ERR_CONF_FILE);
                return false;
            }
            _cout << tr("Settings imported: %1 settings, %2 new profiles, %3 new servers (%4 to switch on after typing the password). The old config is kept as %5")
                     .arg(sum.settings + sum.notifications).arg(sum.profilesNew).arg(sum.serversNew).arg(sum.serversNeedPassword)
                     .arg(configFilePath() + ".before-import") << "\n" << MB_FLUSH;
        }
        return false; // nothing to post: exit
    }

    if (parser.isSet(sOptionNames[Opt::VERSION]))
    {
        _showVersionASCII();
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::DEBUG]))
    {
        _debug = 1;
        _cout << tr("Extra logs are ON\n") << MB_FLUSH;
    }
    if (parser.isSet(sOptionNames[Opt::DEBUG_FULL]))
    {
        _debug = 2;
        _cout << tr("Full debug logs are ON\n") << MB_FLUSH;
    }

    if (parser.isSet(sOptionNames[Opt::DISP_PROGRESS]))
    {
        QString val = parser.value(sOptionNames[Opt::DISP_PROGRESS]);
        val = val.trimmed().toLower();
        if (val == "bar")
        {
            _dispProgressBar  = true;
            _dispFilesPosting = false;
            qDebug() << "Display progressbar bar\n";
        }
        else if (val == "files")
        {
            _dispProgressBar  = false;
            _dispFilesPosting = true;
            qDebug() << "Display Files when start posting\n";
        }
        else if (val == "none")
        { // force it in case in the config file something was on
            _dispProgressBar  = false;
            _dispFilesPosting = false;
        }
    }

    if (parser.isSet(sOptionNames[Opt::CHECK]))
    {
        _nzbCheck = new NzbCheck();
        _nzbCheck->setDebug(_debug);
        _nzbCheck->setDispProgressBar(_dispProgressBar||_dispFilesPosting);
        _nzbCheck->setQuiet(_quiet);
        int nbArticles = _nzbCheck->parseNzb(parser.value(sOptionNames[Opt::CHECK]));
        if (nbArticles > 0 )
        {
            _nzbCheck->checkPost(_nntpServers);
            return true;
        }
        return false;
    }

    if (parser.isSet("resume_queue"))
        _resumeQueue = true;
    if (parser.isSet("allow_repost"))
        _checkDuplicates = false;
    if (parser.isSet("max_speed"))
    {
        bool ok = false;
        double mbps = parser.value("max_speed").replace(',', '.').toDouble(&ok);
        if (!ok || mbps < 0)
        {
            _error(tr("You should give a number of MB/s for the speed limit (option --max_speed), 0 for no limit"),
                   ERROR_CODE::ERR_WRONG_ARG);
            return false;
        }
        _limitSpeed = mbps > 0;
        if (mbps > 0)
            _maxSpeed = mbps;
        applySpeedLimit();
    }
    if (parser.isSet("notify_discord"))
        _notifier.settings().discord = parser.value("notify_discord");
    if (parser.isSet("notify_telegram_token"))
        _notifier.settings().telegramToken = parser.value("notify_telegram_token");
    if (parser.isSet("notify_telegram_chat"))
        _notifier.settings().telegramChat = parser.value("notify_telegram_chat");
    if (parser.isSet("notify_webhook"))
        _notifier.settings().webhook = parser.value("notify_webhook");
    if (parser.isSet("notify_slack"))
        _notifier.settings().slack = parser.value("notify_slack");
    if (parser.isSet("notify_ntfy"))
        _notifier.settings().ntfy = parser.value("notify_ntfy");
    if (parser.isSet("notify_ntfy_token"))
        _notifier.settings().ntfyToken = parser.value("notify_ntfy_token");
    if (parser.isSet("notify_gotify_url"))
        _notifier.settings().gotifyUrl = parser.value("notify_gotify_url");
    if (parser.isSet("notify_gotify_token"))
        _notifier.settings().gotifyToken = parser.value("notify_gotify_token");
    if (parser.isSet("notify_pushover_token"))
        _notifier.settings().pushoverToken = parser.value("notify_pushover_token");
    if (parser.isSet("notify_pushover_user"))
        _notifier.settings().pushoverUser = parser.value("notify_pushover_user");
    if (parser.isSet("notify_email_host"))
        _notifier.settings().emailHost = parser.value("notify_email_host");
    if (parser.isSet("notify_email_port"))
        _notifier.settings().emailPort = static_cast<quint16>(parser.value("notify_email_port").toUInt());
    if (parser.isSet("notify_email_security"))
        _notifier.settings().emailSecurity = parser.value("notify_email_security");
    if (parser.isSet("notify_email_user"))
        _notifier.settings().emailUser = parser.value("notify_email_user");
    if (parser.isSet("notify_email_pass"))
        _notifier.settings().emailPass = parser.value("notify_email_pass");
    if (parser.isSet("notify_email_from"))
        _notifier.settings().emailFrom = parser.value("notify_email_from");
    if (parser.isSet("notify_email_to"))
        _notifier.settings().emailTo = parser.value("notify_email_to");
    if (parser.isSet("notify_when"))
        _notifier.settings().allDone = (parser.value("notify_when").toLower() != "each");
    if (parser.isSet("gen_nfo") || parser.isSet("nfo_in_post"))
        _genNfo = true;
    if (parser.isSet("nfo_in_post"))
        _nfoInPost = true;
    if (parser.isSet("nfo_password"))
        _nfoPassword = true;
    if (parser.isSet("no_nfo_media"))
        _nfoMedia = false;
    if (parser.isSet("nfo_style"))
    {
        bool known = false;
        NfoWriter::Style style = NfoWriter::styleFromKey(parser.value("nfo_style"), &known);
        if (!known)
        {
            _error(tr("Error: unknown value of the option --nfo_style: %1 (classic, clean, box, scene, elegant or retro)").arg(parser.value("nfo_style")), ERROR_CODE::ERR_WRONG_ARG);
            return false;
        }
        _nfoStyle = NfoWriter::styleKey(style);
    }
    if (parser.isSet("nfo_ascii"))
        _nfoAscii = true;
    if (parser.isSet("mediainfo_path"))
        _mediaInfoPath = parser.value("mediainfo_path");
    if (parser.isSet("nfo_text"))
    {
        QFile textFile(parser.value("nfo_text"));
        if (textFile.open(QIODevice::ReadOnly|QIODevice::Text))
            _nfoText = QString::fromUtf8(textFile.readAll());
        else
        {
            _error(tr("Error: can't read the file of the option --nfo_text: %1").arg(parser.value("nfo_text")), ERROR_CODE::ERR_WRONG_ARG);
            return false;
        }
    }

    if (!parser.isSet(sOptionNames[Opt::INPUT]) && !parser.isSet(sOptionNames[Opt::AUTO_DIR]) && !parser.isSet(sOptionNames[Opt::MONITOR_DIR])
            && !_resumeQueue)
    {
        _error(tr("Error syntax: you should provide at least one input file or directory using the option -i, --auto or --monitor"),
               ERROR_CODE::ERR_NO_INPUT);
        return false;
    }
    if (parser.isSet(sOptionNames[Opt::DEL_AUTO]))
    {
        if ( !parser.isSet(sOptionNames[Opt::AUTO_DIR]) && !parser.isSet(sOptionNames[Opt::MONITOR_DIR]))
        {
            _error(tr("Error syntax: --del option is only available with --auto or --monitor"),
                   ERROR_CODE::ERR_DEL_AUTO);
            return false;
        }
        else
            _delAuto = true;
    }
    if (parser.isSet(sOptionNames[Opt::MOVE_POSTED]))
    {
        if (!parser.isSet(sOptionNames[Opt::AUTO_DIR]) && !parser.isSet(sOptionNames[Opt::MONITOR_DIR]))
        {
            _error(tr("Error syntax: --move_posted option is only available with --auto or --monitor"),
                   ERROR_CODE::ERR_DEL_AUTO);
            return false;
        }
        _movePosted = parser.value(sOptionNames[Opt::MOVE_POSTED]).trimmed();
    }
    if (parser.isSet(sOptionNames[Opt::DEL_AUTO]))
    {
        if (parser.isSet(sOptionNames[Opt::MOVE_POSTED]))
        {
            _error(tr("Error syntax: --rm_posted and --move_posted can't be used together"), ERROR_CODE::ERR_DEL_AUTO);
            return false;
        }
        _movePosted.clear(); // --rm_posted wins over move_posted of the configuration file
    }

    if (parser.isSet(sOptionNames[Opt::PACK]))
        enableAutoPacking();

    if (parser.isSet(sOptionNames[Opt::COMPRESS]))
        _doCompress = true;
    if (parser.isSet(sOptionNames[Opt::GEN_PAR2]))
        _doPar2 = true;
    if (parser.isSet(sOptionNames[Opt::GEN_NAME]))
        _genName = true;
    if (parser.isSet(sOptionNames[Opt::GEN_PASS]))
        _genPass = true;
    if (parser.isSet(sOptionNames[Opt::RAR_NO_ROOT_FOLDER]))
        _rarNoRootFolder = true;

    if (parser.isSet(sOptionNames[Opt::AUTO_DIR]))
    {
        if (!_doCompress && !_doPar2)
        {
            _error(tr("Error syntax: --auto only works with --compress or --gen_par2 or --pack with at least the keywords COMPRESS or GEN_PAR2 in PACK config"),
                   ERROR_CODE::ERR_AUTO_NO_COMPRESS);
            return false;
        }
        for (const QString &filePath : parser.values(sOptionNames[Opt::AUTO_DIR]))
        {
            QFileInfo fi(filePath);
            if (!fi.exists() || !fi.isDir())
            {
                _error(tr("Error syntax: --auto only uses folders as argument..."),
                       ERROR_CODE::ERR_AUTO_INPUT);
                return false;
            }
            else
            {
                _autoDirs << QDir(fi.absoluteFilePath());
                if (!_doCompress) {
                    // only genPar2 => only accept files
                    QStringList subFolders = _autoDirs.last().entryList(QDir::Dirs|QDir::NoDotAndDotDot);
                    if (subFolders.size())
                        _error(tr("Error: you can only --auto without compression on folders that DON'T have any subfolders.\nThat's not the case for '%1' which contains folders: %2").arg(
                                   fi.fileName(), subFolders.join(", ")));
                }
            }
        }
    }        

    bool isMonitoring = false;
    if (parser.isSet(sOptionNames[Opt::MONITOR_DIR]))
    {
        if (!_doCompress && (!_doPar2 || !_monitorIgnoreDir))
        {
            _error(tr("Error syntax: --monitor only works with --compress or with --gen_par2 ONLY IF MONITOR_IGNORE_DIR is enabled in config (--pack can be used)"),
                   ERROR_CODE::ERR_MONITOR_NO_COMPRESS);
            return false;
        }
        for (const QString &filePath : parser.values(sOptionNames[Opt::MONITOR_DIR]))
        {
            QFileInfo fi(filePath);
            if (!fi.exists() || !fi.isDir())
            {
                _error(tr("Error syntax: --monitor only uses folders as argument..."),
                       ERROR_CODE::ERR_MONITOR_INPUT);
                return false;
            }
            else
            {
                _cout << "[FolderMonitor] " << tr("start monitoring: ") << fi.absoluteFilePath()
                      << " (delay: " << FoldersMonitorForNewFiles::sMSleep << "ms)\n" << MB_FLUSH;
                isMonitoring = true;
                if (_folderMonitor)
                    _folderMonitor->addFolder(fi.absoluteFilePath());
                else
                    _startMonitoring(fi.absoluteFilePath());
            }
        }
    }

    if (parser.isSet(sOptionNames[Opt::OBFUSCATE]))
    {
        _obfuscateArticles = true;
        _cout << tr("Do article obfuscation (the subject of each Article will be a UUID)\n") << MB_FLUSH;
    }


    if (parser.isSet(sOptionNames[Opt::THREAD]))
    {
        bool ok;
        _nbThreads = parser.value(sOptionNames[Opt::THREAD]).toInt(&ok);
        if (_nbThreads < 1)
            _nbThreads = 1;
        if (!ok)
        {
            _error(tr("You should give an integer for the number of threads (option -t)"),
                   ERROR_CODE::ERR_NB_THREAD);
            return false;
        }
    }

    if (parser.isSet(sOptionNames[Opt::META]))
    {
        for (const QString &meta : parser.values(sOptionNames[Opt::META]))
        {
            QStringList mList = meta.split("=");
            if (mList.size() == 2)
                _meta.insert(escapeXML(mList[0]), escapeXML(mList[1]));
        }
    }


    if (parser.isSet(sOptionNames[Opt::GROUPS]))
        updateGroups(parser.value(sOptionNames[Opt::GROUPS]));

    if (parser.isSet(sOptionNames[Opt::FROM]))
    {
        QString val = parser.value(sOptionNames[Opt::FROM]);
        QRegularExpression email("\\w+@\\w+\\.\\w+");
        if (!email.match(val).hasMatch())
            val += "@ngPost.com";
        val = escapeXML(val);
        _from = val.toStdString();
    }


    if (parser.isSet(sOptionNames[Opt::GEN_FROM]))
    {
        _genFrom = true;
        if (_debug)
            _cout << tr("Generate new random poster for each post") << "\n" << MB_FLUSH;
    }
    else if (_from.empty())
        _from = randomStdFrom();


    if (parser.isSet(sOptionNames[Opt::MSG_ID]))
        sArticleIdSignature = escapeXML(parser.value(sOptionNames[Opt::MSG_ID])).toStdString();

    if (parser.isSet(sOptionNames[Opt::ARTICLE_SIZE]))
    {
        bool ok;
        int size = parser.value(sOptionNames[Opt::ARTICLE_SIZE]).toInt(&ok);
        if (ok)
            sArticleSize = size;
        else
        {
            _error(tr("You should give an integer for the article size (option -a)"),
                   ERROR_CODE::ERR_ARTICLE_SIZE);
            return false;
        }
    }

    if (parser.isSet(sOptionNames[Opt::NB_RETRY]))
    {
        bool ok;
        ushort nbRetry = parser.value(sOptionNames[Opt::NB_RETRY]).toUShort(&ok);
        if (ok)
            NntpArticle::setNbMaxRetry(nbRetry);
        else
        {
            _error(tr("You should give an unisgned integer for the number of retry for posting an Article (option -r)"),
                   ERROR_CODE::ERR_NB_RETRY);
            return false;
        }
    }



    // compression section
    if (parser.isSet(sOptionNames[Opt::TMP_DIR]))
        _tmpPath = parser.value(sOptionNames[Opt::TMP_DIR]);
    if (parser.isSet(sOptionNames[Opt::RAR_PATH]))
        _rarPath = parser.value(sOptionNames[Opt::RAR_PATH]);
    if (parser.isSet(sOptionNames[Opt::RAR_SIZE]))
    {
        bool ok;
        uint nb = parser.value(sOptionNames[Opt::RAR_SIZE]).toUInt(&ok);
        if (ok)
            _rarSize = nb;
    }
    if (parser.isSet(sOptionNames[Opt::RAR_MAX]))
    {
        _useRarMax = true;
        bool ok;
        uint nb = parser.value(sOptionNames[Opt::RAR_MAX]).toUInt(&ok);
        if (ok)
            _rarMax = nb;
    }
    if (parser.isSet(sOptionNames[Opt::PAR2_PCT]))
    {
        bool ok;
        uint nb = parser.value(sOptionNames[Opt::PAR2_PCT]).toUInt(&ok);
        if (ok)
        {
            _par2Pct = nb;
            if (nb > 0)
                _doPar2 = true;
        }
    }
    if (parser.isSet(sOptionNames[Opt::PAR2_PATH]))
    {
        QString val = parser.value(sOptionNames[Opt::PAR2_PATH]);
        if (!val.isEmpty())
        {
            QFileInfo fi(val);
            if (fi.exists() && fi.isFile() && fi.isExecutable())
                _par2Path = val;
        }
    }

    if (_doPar2 && _par2Pct == 0 && _par2Args.isEmpty())
    {
        _error(tr("Error: can't generate par2 if the redundancy percentage is null or PAR2_ARGS is not provided...\nEither use --par2_pct or set PAR2_PCT or PAR2_ARGS in the config file."),
               ERROR_CODE::ERR_PAR2_ARGS);
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::RAR_NAME]))
        _rarName = parser.value(sOptionNames[Opt::RAR_NAME]);
    if (parser.isSet(sOptionNames[Opt::RAR_PASS]))
    {
        _rarPass      = parser.value(sOptionNames[Opt::RAR_PASS]);
        _rarPassFixed = _rarPass;
    }

    if (parser.isSet(sOptionNames[Opt::LENGTH_NAME]))
    {
        bool ok;
        uint nb = parser.value(sOptionNames[Opt::LENGTH_NAME]).toUInt(&ok);
        if (ok)
            _lengthName = nb;
    }
    if (parser.isSet(sOptionNames[Opt::LENGTH_PASS]))
    {
        bool ok;
        uint nb = parser.value(sOptionNames[Opt::LENGTH_PASS]).toUInt(&ok);
        if (ok)
            _lengthPass = nb;
    }



    if (parser.isSet(sOptionNames[Opt::SERVER]))
    {
        _nntpServers.clear();
        QRegularExpression regExp(sNntpServerStrRegExp,  QRegularExpression::CaseInsensitiveOption);
        for (const QString &serverParam : parser.values(sOptionNames[Opt::SERVER]))
        {
            QRegularExpressionMatch match = regExp.match(serverParam);
            if (match.hasMatch())
            {
                bool    auth  = !match.captured(1).isEmpty();
                QString user  = match.captured(2);
                QString pass  = match.captured(3);
                QString host  = match.captured(4);
                ushort  port  = match.captured(5).toUShort();
                int     nbCon = match.captured(6).toInt();
                bool    ssl   = match.captured(7).isEmpty();
#ifdef __DEBUG__
                qDebug() << "NNTP Server: " << user << ":" << pass
                         << "@" << host << ":" << port << ":" << nbCon << ":" << ssl;
#endif
                NntpServerParams *server = new NntpServerParams(host,
                                                                port,
                                                                auth,
                                                                user.toStdString(),
                                                                pass.toStdString(),
                                                                nbCon,
                                                                ssl);
                _nntpServers << server;
            }
            else
            {
                _error(tr("Syntax error on server details for %1, the format should be: %2").arg(
                           serverParam).arg("(<user>:<pass>@@@)?<host>:<port>:<nbCons>:(no)?ssl"),
                       ERROR_CODE::ERR_SERVER_REGEX);
                return false;
            }
        }
    }

    // Server Section under
    // check if the server params are given in the command line
    if (parser.isSet(sOptionNames[Opt::HOST]))
    {
        QString host = parser.value(sOptionNames[Opt::HOST]);


        if (!parser.isSet(sOptionNames[Opt::SERVER]))
            _nntpServers.clear();
        NntpServerParams *server = new NntpServerParams(host);
        _nntpServers << server;

        if (parser.isSet(sOptionNames[Opt::SSL]))
        {
            server->useSSL = true;
            server->port = NntpServerParams::sDefaultSslPort;
        }

        if (parser.isSet(sOptionNames[Opt::PORT]))
        {
            bool ok;
            ushort port = parser.value(sOptionNames[Opt::PORT]).toUShort(&ok);
            if (ok)
                server->port = port;
            else
            {
                _error(tr("You should give an integer for the port (option -P)"),
                       ERROR_CODE::ERR_SERVER_PORT);
                return false;
            }
        }

        if (parser.isSet(sOptionNames[Opt::USER]))
        {
            server->auth = true;
            server->user = parser.value(sOptionNames[Opt::USER]).toStdString();

            if (parser.isSet(sOptionNames[Opt::PASS]))
                server->pass = parser.value(sOptionNames[Opt::PASS]).toStdString();

        }

        if (parser.isSet(sOptionNames[Opt::CONNECTION]))
        {
            bool ok;
            int nbCons = parser.value(sOptionNames[Opt::CONNECTION]).toInt(&ok);
            if (ok)
                server->nbCons = nbCons;
            else
            {
                _error(tr("You should give an integer for the number of connections (option -n)"),
                       ERROR_CODE::ERR_SERVER_CONS);
                return false;
            }
        }
    }



    QList<QFileInfo> filesToUpload;
    QStringList filesPath;
    for (const QString &filePath : parser.values(sOptionNames[Opt::INPUT]))
    {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable())
        {
            _error(tr("Error: the input file '%1' is not readable...").arg(parser.value("input")),
                   ERROR_CODE::ERR_INPUT_READ);
            return false;
        }
        else
        {
            if (_nzbName.isEmpty())
                setNzbName(fileInfo); // The first file will be used
            if (fileInfo.isFile())
            {
                filesToUpload << fileInfo;
                filesPath     << fileInfo.absoluteFilePath();
                if (_debug)
                    _log(tr("+ File to %2: %1").arg(fileInfo.fileName()).arg(_doCompress?tr("compress"):tr("post")));
            }
            else
            {
                if (_doCompress)
                {
                    if (_debug)
                        _log(tr("+ Adding folder to Compress: %1").arg(fileInfo.absoluteFilePath()));
                    filesToUpload << fileInfo;
                    filesPath     << fileInfo.absoluteFilePath();
                }
                else {
                    QDir dir(fileInfo.absoluteFilePath());
                    for (const QFileInfo &subFile : dir.entryInfoList(QDir::Files, QDir::Name))
                    {
                        if (subFile.isReadable())
                        {
                            filesToUpload << subFile;
                            filesPath     << subFile.absoluteFilePath();
                            if (_debug)
                                _log(tr("+ subFile to post: %1").arg(subFile.fileName()));
                        }
                        else
                        {
                            _error(tr("Error: the input file '%1' is not readable...").arg(subFile.absoluteFilePath()),
                                   ERROR_CODE::ERR_INPUT_READ);
                            return false;
                        }
                    }

                    if (filesToUpload.isEmpty())
                    {
                        _error(tr("Error: the input folder '%1' has no files... (no recursivity without --compress)").arg(fileInfo.absoluteFilePath()),
                               ERROR_CODE::ERR_INPUT_READ);
                        return false;
                    }
                }
            }
        }
    }


    if (parser.isSet("o"))
    {
        QFileInfo nzb(parser.value(sOptionNames[Opt::OUTPUT]));
        setNzbName(nzb);
        _nzbPath = nzb.absolutePath();
    }

    if (_rarName.isEmpty())
        _rarName = _nzbName;

    if (_doCompress)
    {
        if (_genName)
            _rarName = randomPass(_lengthName);

        if (_genPass)
        {
            _rarPass = randomPass(_lengthPass);
            _meta.remove("password");
        }
    }

#ifdef __DEBUG__
    _dumpParams();
#endif


    // persistent queue: the posts that were not finished are posted first (only if asked)
    int nbResumed = 0;
    if (_resumeQueue)
        nbResumed = resumeQueue();
    else if (PostQueue *queue = postQueue())
    {
        if (queue->size())
            _log(tr("Queue: %1 unfinished post(s) are waiting, use --resume_queue (or resume_queue = on in the config) to post them again").arg(
                     queue->size()));
    }

    if (filesToUpload.isEmpty() && _autoDirs.isEmpty() && !isMonitoring && !nbResumed)
    {
        _error(tr("Nothing to do..."));
        return false;
    }

    if (filesToUpload.size())
    { // input files provided with -i
        QString nzbFilePath = nzbPath();
        if (!nzbFilePath.endsWith(".nzb"))
            nzbFilePath += ".nzb";
        startPostingJob(new PostingJob(this, nzbFilePath, filesToUpload, nullptr,
                                       getPostingGroups(), from(),
                                       _obfuscateArticles, _obfuscateFileName,
                                       _tmpPath, _rarPath, _rarArgs,
                                       _rarSize, _useRarMax, _par2Pct,
                                       _doCompress, _doPar2, _rarName, _rarPass,
                                       _keepRar, _delAuto, false));
    }

    if (_autoDirs.size())
    {
        for (const QDir &dir : _autoDirs)
        {
            _cout << "===> Auto dir: " << dir.absolutePath() << "\n" << MB_FLUSH;
            for (const QFileInfo & fileInfo : dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name))
                if (!isPostedFolder(fileInfo))
                    _post(fileInfo, _monitor_nzb_folders ? QDir(fileInfo.absolutePath()).dirName() : QString());
        }
    }


    return true;
}

void NgPost::setNzbName(const QFileInfo &fileInfo)
{
    _nzbName = fileInfo.isDir() ? QDir(fileInfo.absoluteFilePath()).dirName() : fileInfo.completeBaseName();
    if (_removeAccentsOnNzbFileName)
    {
        _nzbName.replace(QRegularExpression("[ÀÁÂÃÄÅ]"), "A");
        _nzbName.replace(QRegularExpression("[àáâãäå]"), "a");
        _nzbName.replace("Ç","C");
        _nzbName.replace("ç","c");
        _nzbName.replace(QRegularExpression("[ÈÉÊË]"),   "E");
        _nzbName.replace(QRegularExpression("[èéêë]"),   "e");
        _nzbName.replace(QRegularExpression("[ÌÍÎÏ]"),   "I");
        _nzbName.replace(QRegularExpression("[ìíîï]"),   "i");
        _nzbName.replace("Ñ","N");
        _nzbName.replace("ñ","n");
        _nzbName.replace(QRegularExpression("[ÒÓÔÕÖØ]"), "O");
        _nzbName.replace(QRegularExpression("[òóôõöø]"), "o");
        _nzbName.replace(QRegularExpression("[ÙÚÛÜ]"),   "U");
        _nzbName.replace(QRegularExpression("[ùúûü]"),   "u");
        _nzbName.replace(QRegularExpression("[ÿý]"),     "y");
        _nzbName.replace(QRegularExpression("[^A-Za-z0-9\\.,_\\-\\(\\)\\[\\]\\{\\}!#&'\\+ ]"), "");
    }
}

QString NgPost::nzbPath(const QString &monitorFolder)
{
    if (monitorFolder.isEmpty())
        return nzbPath();
    else
    {
        QString path;
        if (_nzbPath.isEmpty())
            path = monitorFolder;
        else
            path = QString("%1/%2").arg(_nzbPath).arg(monitorFolder);
        QDir dir(path);
        if (!dir.exists())
            QDir().mkdir(path);

        return QString("%1/%2").arg(path).arg(_nzbName);
    }
}

void NgPost::_syntax(char *appName)
{
    QString app = QFileInfo(appName).fileName();
    _cout << desc() << "\n"
          << tr("Syntax: ") << app << " (options)* (-i <file or folder> | --auto <folder> | --monitor <folder>)+\n";
    for (const QCommandLineOption & opt : sCmdOptions)
    {
        if (opt.valueName() == sOptionNames[Opt::SERVER])
            _cout << "\n// " << tr("you can provide servers in one string using -S and/or split the parameters for ONE SINGLE server (this will overwrite the configuration file)") << "\n";
        else if (opt.valueName() == sOptionNames[Opt::TMP_DIR])
            _cout << "\n// " << tr("for compression and par2 support") << "\n";
        else if (opt.valueName() == sOptionNames[Opt::AUTO_DIR])
            _cout << "\n// " << tr("automated posting (scanning and/or monitoring)") << "\n";
        else if (opt.valueName() == sOptionNames[Opt::INPUT])
            _cout << "\n// " << tr("quick posting (several files/folders)") << "\n";
        else if (opt.valueName() == sOptionNames[Opt::OBFUSCATE])
            _cout << "\n// " << tr("general options") << "\n";

        if (opt.names().size() == 1)
            _cout << QString("\t--%1: %2\n").arg(opt.names().first(), -17).arg(tr(opt.description().toLocal8Bit().constData()));
        else
            _cout << QString("\t-%1: %2\n").arg(opt.names().join(" or --"), -18).arg(tr(opt.description().toLocal8Bit().constData()));
    }

    _cout << "\n" << tr("Examples:") << "\n"
          << "  - " << tr("with monitoring") << ": " << app << " --monitor /data/folder1 --monitor /data/folder2 --auto_compress --rm_posted --disp_progress files\n"
          << "  - " << tr("with auto post")  << ": " << app << " --auto /data/folder1 --auto /data/folder2 --compress --gen_par2 --gen_name --gen_pass --rar_size 42 --disp_progress files\n"
          << "  - " << tr("with compression, filename obfuscation, random password and par2") << ": " << app << " -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2\n"
          << "  - " << tr("with config file") << ": " << app << " -c ~/.ngPost -m \"password=qwerty42\" -f ngPost@nowhere.com -i /tmp/file1 -i /tmp/file2 -i /tmp/folderToPost1 -i /tmp/folderToPost2\n"
          << "  - " << tr("with all params") << ":  " << app << " -t 1 -m \"password=qwerty42\" -m \"metaKey=someValue\" -h news.newshosting.com -P 443 -s -u user -p pass -n 30 -f ngPost@nowhere.com \
 -g \"alt.binaries.test,alt.binaries.test2\" -a 64000 -i /tmp/folderToPost -o /tmp/folderToPost.nzb\n"
          << "\n"
          << tr("If you don't provide the output file (nzb file), we will create it in the nzbPath with the name of the first file or folder given in the command line.") << "\n"
          << tr("so in the second example above, the nzb would be: /tmp/file1.nzb") << "\n"
          << MB_FLUSH;
}

void NgPost::_showVersionASCII() const
{
    _cout << sNgPostASCII
          << "                          " << sAppName << " v" << versionLabel() << "\n\n"
          << PostingJob::sslSupportInfo()  << "\n"
          << MB_FLUSH;
}
