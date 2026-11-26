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

#include "PostingWidget.h"
#include <QScrollArea>
#include <QDialog>
#include <QDirIterator>
#include <QStorageInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "ProfileBar.h"
#include "PostTargetsBox.h"
#include "GroupsEdit.h"
#include "nntp/NntpServerParams.h"
#include <QDateTime>
#include "PostCheckDialog.h"
#include "NfoCard.h"
#include "utils/PostProfile.h"
#include "utils/NfoWriter.h"
#include "nntp/NntpServerParams.h"
#include "utils/MediaInfoReader.h"
#include "ui_PostingWidget.h"
#include "MainWindow.h"
#include <QCoreApplication>
#include "AboutNgPost.h"
#include "NgPost.h"
#include "PostingJob.h"
#include "nntp/NntpFile.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QJsonArray>



PostingWidget::PostingWidget(NgPost *ngPost, MainWindow *hmi, uint jobNumber) :
    QWidget(hmi),
    _ui(new Ui::PostingWidget),
    _hmi(hmi),
    _ngPost(ngPost),
    _jobNumber(jobNumber),
    _filesTitle(nullptr), _releaseInfo(nullptr), _nzbTitle(nullptr), _archiveTitle(nullptr), _advancedTitle(nullptr),
    _advancedCard(nullptr), _profileBar(nullptr), _nfoCard(nullptr), _summary(nullptr), _sourceBytes(0), _advancedToggle(nullptr)
{
    _ui->setupUi(this);
    _buildLayout();

    connect(_ui->postButton, &QAbstractButton::clicked, this, &PostingWidget::onPostFiles);
    connect(_ui->nzbPassCB,  &QAbstractButton::toggled, this, &PostingWidget::onNzbPassToggled);
    connect(_ui->genPass,    &QAbstractButton::clicked, this, &PostingWidget::onGenNzbPassword);

    _ui->filesList->setEmptyState(tr("Drag files or folders here, or choose them with the buttons below."),
                                  tr("Select Files"), tr("Select Folder"));
    connect(_ui->filesList, &SignedListWidget::emptyAction1, this, &PostingWidget::onSelectFilesClicked);
    connect(_ui->filesList, &SignedListWidget::emptyAction2, this, &PostingWidget::onSelectFolderClicked);
    connect(_ui->filesList, &SignedListWidget::rightClick, this, &PostingWidget::onSelectFilesClicked);
}

PostingWidget::~PostingWidget()
{
    delete _ui;
}





bool PostingWidget::_applyFix(PostChecker::Code code)
{
    using Code = PostChecker::Code;
    switch (code)
    {
    case Code::NoServer:
        _hmi->showServersPage();
        return false;
    case Code::NoGroup:
        _hmi->showSettingsPage(MainWindow::CAT_POST);
        return false;
    case Code::FoldersNoCompress:
        _ui->compressCB->setChecked(true);
        break;
    case Code::RarMissing:
        onRarPathClicked();
        break;
    case Code::Par2Missing:
        _ui->par2CB->setChecked(false);
        break;
    case Code::PasswordEmpty:
        onGenNzbPassword();
        break;
    case Code::NzbFolderNotWritable:
        onNzbFileClicked();
        break;
    case Code::MediaInfoMissing:
        _ngPost->_nfoMedia = false;
        emit _ngPost->nfoOptionsChanged();
        break;
    default:
        break;
    }
    return true;
}

bool PostingWidget::_checkBeforePosting(const QFileInfoList &files, bool hasFolder)
{
    auto makeInput = [this, &files, hasFolder]() {
        udatePostingParams();
        PostChecker::Input in = _hmi->checkInput(files, hasFolder, _nzbFilePath(), _ui->nzbPassCB->isChecked(), _ui->nzbPassEdit->text());
        // the newsgroups and the servers of this post, when they are not the ones of the Settings
        const PostTargets t = _targets->targets();
        if (!t.groups.isEmpty())
        {
            in.hasGroups     = true;
            in.invalidGroups = _targets->groupsEdit()->invalidGroups().join(", ");
        }
        if (!t.servers.isEmpty())
        {
            int nb = 0;
            for (NntpServerParams *srv : _ngPost->_nntpServers)
                if (t.servers.contains(NgPost::serverKey(*srv)))
                    ++nb;
            if (nb > 0)
                in.nbServers = nb;
        }
        return in;
    };
    return PostCheckDialog::run(isVisible() ? static_cast<QWidget*>(this) : _hmi, makeInput, [this](PostChecker::Code code){ return _applyFix(code); });
}

void PostingWidget::_updateSummary(bool recomputeSize)
{
    if (!_summary)
        return;
    if (_ui->filesList->count() == 0)
    {
        _summary->hide();
        return;
    }
    if (recomputeSize)
    {
        QFileInfoList files;
        bool hasFolder = false;
        _buildFilesList(files, hasFolder);
        _sourceBytes = 0;
        for (const QFileInfo &fi : files)
            _sourceBytes += NgPost::recursiveSize(fi);
    }
    bool pack = _ui->compressCB->isChecked() || _ui->par2CB->isChecked();
    QString text = tr("%1 item(s), %2").arg(_ui->filesList->count()).arg(NfoWriter::humanSize(_sourceBytes));
    qint64 up = PostChecker::uploadBytes(_sourceBytes, _ui->par2CB->isChecked(), static_cast<uint>(_ui->redundancySB->value()));
    if (pack)
        text += QString::fromUtf8(" \u2192 ") + tr("about %1 to upload").arg(NfoWriter::humanSize(up));
    QString time = PostChecker::durationText(up, _ngPost->_lastUploadSpeed);
    text += QString::fromUtf8("  \u00b7  ") + (time.isEmpty() ? tr("upload time: shown after the first post") : tr("upload time: about %1").arg(time));
    _summary->setText(text);
    _summary->show();
}

void PostingWidget::onPostFiles()
{
    postFiles(true);
}

int PostingWidget::postFiles(bool updateMainParams)
{
    {
        if (_ui->filesList->count() == 0)
        {
            _hmi->logError(tr("There are no selected files to post..."));
            return -1;
        }

        QFileInfoList files;
        bool hasFolder = false;
        _buildFilesList(files, hasFolder);
        if (files.isEmpty())
        {
            _hmi->logError(tr("There are no existing files to post..."));
            return -1;
        }

        if (updateMainParams)
        {
            // post started with the button: the checks explain what is wrong and fix it with one click
            _hmi->updateServers();
            _hmi->updateParams();
            if (!_checkBeforePosting(files, hasFolder))
                return -1;
        }
        else if (hasFolder && !_ui->compressCB->isChecked())
        {
            _hmi->logError(tr("You can't post folders without using compression..."));
            return -1;
        }
        udatePostingParams();

        // check if the nzb file name already exist
        QString nzbPath = _nzbFilePath();
        const PostTargets targets = _targets->targets();
        // the number of {n} is used only when the name was made by the program
        const bool useCounter = PostTargets::usesCounter(targets.nameTemplate) && !_autoNzbFile.isEmpty()
                && _ui->nzbFileEdit->text() == _autoNzbFile;
        QFileInfo fiNzb(nzbPath);
        if (!_fromAuto && fiNzb.exists())
        {
            int overwrite = QMessageBox::question(_hmi,
                                                  tr("Overwrite existing nzb file?"),
                                                  tr("The nzb file '%1' already exists.\nWould you like to overwrite it ?").arg(nzbPath),
                                                  QMessageBox::Yes,
                                                  QMessageBox::No);
            if (overwrite == QMessageBox::No)
                return -1;
        }

        // already posted (same name and size in the history)?
        QString posted = _fromAuto ? QString() : _ngPost->duplicateInPost(fiNzb.fileName(), files);
        if (!posted.isEmpty())
        {
            QMessageBox box(QMessageBox::Question, tr("Already posted?"),
                            tr("This post is already in the history (%1, posted on %2).\nDo you want to post it again?").arg(fiNzb.fileName(), posted),
                            QMessageBox::NoButton, isVisible() ? static_cast<QWidget*>(this) : _hmi);
            QPushButton *cancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);
            box.addButton(tr("Post anyway"), QMessageBox::AcceptRole);
            box.setDefaultButton(cancel);
            box.exec();
            if (box.clickedButton() == cancel)
                return -1;
        }

        PostingJob *job = new PostingJob(_ngPost, nzbPath, files, nullptr,
                                         _ngPost->getPostingGroups(targets.groups),
                                         _ngPost->from(),
                                         _ngPost->_obfuscateArticles, _ngPost->_obfuscateFileName,
                                         _ngPost->_tmpPath, _ngPost->_rarPath, _ngPost->_rarArgs,
                                         _ngPost->_rarSize, _ngPost->_useRarMax, _ngPost->_par2Pct,
                                         _ngPost->_doCompress, _ngPost->_doPar2,
                                         _ngPost->_rarName, _ngPost->_rarPass,
                                         _ngPost->_keepRar, _afterAct, !_fromAuto);
        job->setMovePostedTo(_afterMoveTo);
        job->setOrigin(_fromAuto ? "auto" : "quick");
        job->setServers(targets.servers);
        if (useCounter)
        {
            ++_ngPost->_nameCounter;
            _hmi->scheduleAutoSave(); // the counter is kept
        }
        job->setHeld(_fromAuto ? _addPaused : (_addPausedCB && _addPausedCB->isChecked()));
        _ngPost->startPostingJob(job);
        int postId = job->postId(); // the post is in the list of the page Posts

        if (!_fromAuto)
        {   // the form is ready for the next release
            _resetForNextPost();
            emit postQueued(postId);
        }
        return postId;
    }
}









void PostingWidget::onNzbPassToggled(bool checked)
{
    _ui->nzbPassEdit->setEnabled(checked);
    _ui->passLengthSB->setEnabled(checked);
    _ui->genPass->setEnabled(checked);
}

void PostingWidget::onGenNzbPassword()
{
    _ui->nzbPassEdit->setText(_ngPost->randomPass(static_cast<uint>(_ui->passLengthSB->value())));
}

void PostingWidget::onSelectFilesClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                tr("Select one or more files to Post"),
                _ngPost->_inputDir);

    int currentNbFiles = _ui->filesList->count();
    for (const QString &file : files)
        addPath(file, currentNbFiles);
}

void PostingWidget::onSelectFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ngPost->_inputDir,
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
        addPath(folder, _ui->filesList->count(), true);
}

void PostingWidget::onClearFilesClicked()
{
    _ui->filesList->clear2();
    _ui->nzbFileEdit->clear();
    _autoNzbFile.clear();
    _nzbBase.clear();
    _ui->compressNameEdit->clear();
    _nameFromFile = false;
    if (_hmi->hasAutoCompress())
    {
        onGenCompressName();
        onGenNzbPassword();
    }
    else
        _ui->compressNameEdit->clear();

}

QString PostingWidget::_nzbFilePath() const
{
    QString path = _nzbDir.isEmpty() ? _ngPost->nzbPath() : QDir::toNativeSeparators(QDir(_nzbDir).filePath(_ngPost->_nzbName));
    if (!path.endsWith(".nzb"))
        path += ".nzb";
    return path;
}

void PostingWidget::_setAutoNzbFile()
{
    if (_nzbBase.isEmpty())
        return;
    _ngPost->_nzbName = PostTargets::applyTemplate(_targets->targets().nameTemplate, _nzbBase,
                                                   QDateTime::currentDateTime(), _ngPost->_nameCounter + 1);
    _autoNzbFile = _nzbFilePath();
    _ui->nzbFileEdit->setText(_autoNzbFile);
}

void PostingWidget::_refreshAutoNames()
{
    if (_autoNzbFile.isEmpty() || _ui->nzbFileEdit->text() != _autoNzbFile)
        return; // the user chose the NZB file: it is kept
    _setAutoNzbFile();
    if (_nameFromFile)
        _ui->compressNameEdit->setText(_ngPost->_nzbName);
}

void PostingWidget::_setNzbDir(const QString &dir)
{
    _nzbDir = dir;
    const QString field = _ui->nzbFileEdit->text();
    if (field.isEmpty() || field == _autoNzbFile)
        return; // empty, or made by the program (_refreshAutoNames)
    if (!dir.isEmpty()) // a file chosen by the user goes in the folder of the profile
        _ui->nzbFileEdit->setText(QDir::toNativeSeparators(QDir(dir).filePath(QFileInfo(field).fileName())));
}

void PostingWidget::_applyTargets(const PostTargets &t, bool onlyFilled)
{
    _targets->setTargets(t, onlyFilled);
    if (!onlyFilled || !t.nzbDir.isEmpty())
        _setNzbDir(t.nzbDir);
    _refreshAutoNames();
}

PostProfile PostingWidget::_profileForSave() const
{
    PostProfile p = currentProfile();
    const QString field = _ui->nzbFileEdit->text().trimmed();
    if (!field.isEmpty())
        p.targets.nzbDir = QDir::toNativeSeparators(QFileInfo(field).absolutePath());
    return p;
}

void PostingWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (_targets)
        _targets->refreshHints(); // the newsgroups and the NZB folder of the Settings may have changed
}

void PostingWidget::_resetForNextPost()
{
    // the options stay; the list, the nzb and (when they were random) the archive name and password are new
    bool compress   = _ui->compressCB->isChecked();
    bool randomName = compress && !_nameFromFile && !_ui->compressNameEdit->text().isEmpty();
    bool randomPass = _ui->nzbPassCB->isChecked() && !_hmi->useFixedPassword();
    onClearFilesClicked();
    _ui->compressNameEdit->clear();
    _nameFromFile = false;
    if (randomName)
        onGenCompressName();
    if (randomPass)
        onGenNzbPassword();
    _updateSummary();
    _updateReleaseInfo();
}

void PostingWidget::onCompressCB(bool checked)
{
    _ui->compressNameEdit->setEnabled(checked);
    _ui->nameLengthSB->setEnabled(checked);
    _ui->genCompressName->setEnabled(checked);
    _ui->keepRarCB->setEnabled(checked);
}

void PostingWidget::onGenCompressName()
{
    _ui->compressNameEdit->setText(_ngPost->randomPass(static_cast<uint>(_ui->nameLengthSB->value())));
    _nameFromFile = false;
}

void PostingWidget::onCompressPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->compressPathEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
    {
        _ui->compressPathEdit->setText(path);
        emit _ui->compressPathEdit->editingFinished(); // same as a typed change: the config is saved
    }
}

void PostingWidget::onNzbFileClicked()
{
    QString path = QFileDialog::getSaveFileName(
                this,
                tr("Create nzb file"),
                _nzbDir.isEmpty() ? _ngPost->_nzbPath : _nzbDir,
                "*.nzb"
                );

    if (!path.isEmpty())
        _ui->nzbFileEdit->setText(path);
}

void PostingWidget::onRarPathClicked()
{
    QString path = QFileDialog::getOpenFileName(
                this,
                tr("Select rar executable"),
                QFileInfo(_ngPost->_rarPath).absolutePath()
                );

    if (!path.isEmpty())
    {
        QFileInfo fi(path);
        if (fi.isFile() && fi.isExecutable())
        {
            _ui->rarEdit->setText(path);
            emit _ui->rarEdit->editingFinished();
        }
        else
            _ngPost->error(tr("the selected file is not executable..."));
    }
}

void PostingWidget::handleKeyEvent(QKeyEvent *keyEvent)
{
    qDebug() << "[PostingWidget::handleKeyEvent] key event: " << keyEvent->key();

    if(keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
    {
        for (QListWidgetItem *item : _ui->filesList->selectedItems())
        {
            qDebug() << "[PostingWidget::handleKeyEvent] remove item: " << item->text();
            _ui->filesList->removeItemWidget2(item);
            delete item;
        }
    }
    else if (keyEvent->matches(QKeySequence::Paste))
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        if (mimeData->hasImage()) {
            qDebug() << "[PostingWidget::handleKeyEvent] try to paste image...";
        } else if (mimeData->hasHtml()) {
            qDebug() << "[PostingWidget::handleKeyEvent] try to paste html: ";
        } else if (mimeData->hasText()) {
            QString txt = mimeData->text();
            qDebug() << "[PostingWidget::handleKeyEvent] paste text: " << txt;
            int currentNbFiles = _ui->filesList->count();
            for (const QString &path : txt.split(QRegularExpression("\n|\r|\r\n")))
            {
                QFileInfo fileInfo(path);
                if (!fileInfo.exists())
                    qDebug() << "[PostingWidget::handleKeyEvent] NOT A FILE: " << path;
                else
                    addPath(path, currentNbFiles, fileInfo.isDir());
//                        else if (fileInfo.isDir())
//                        {
//                            QDir dir(fileInfo.absoluteFilePath());
//                            for (const QFileInfo &subFile : dir.entryInfoList(QDir::Files, QDir::Name))
//                            {
//                                if (subFile.isReadable())
//                                    _addFile(subFile.absoluteFilePath(), currentNbFiles);
//                            }
//                        }
            }

        } else if (mimeData->hasUrls()) {
            qDebug() << "[PostingWidget::handleKeyEvent] paste urls...";

        } else {
            qDebug() << "[PostingWidget::handleKeyEvent] unknown type...";
        }
    }

}


void PostingWidget::handleDropEvent(QDropEvent *e)
{
    int currentNbFiles = _ui->filesList->count();
    for (const QUrl &url : e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        addPath(fileName, currentNbFiles, QFileInfo(fileName).isDir());
    }
}

void PostingWidget::_buildFilesList(QFileInfoList &files, bool &hasFolder)
{
    for (int i = 0 ; i < _ui->filesList->count() ; ++i)
    {
        QFileInfo fileInfo(_ui->filesList->item(i)->text());
        if (fileInfo.exists())
        {
            files << fileInfo;
            if (fileInfo.isDir())
                hasFolder = true;
        }
    }
}

void PostingWidget::init()
{
    _ui->rarMaxCB->setChecked(_ngPost->_useRarMax);

    _ui->nzbPassCB->setChecked(false);
    onNzbPassToggled(false);

    _ui->compressPathEdit->setText(_ngPost->_tmpPath);
    _ui->rarEdit->setText(_ngPost->_rarPath);

    _ui->rarSizeEdit->setText(QString::number(_ngPost->_rarSize));
    _ui->rarSizeEdit->setValidator(new QIntValidator(1, 1000000, _ui->rarSizeEdit));

    _ui->keepRarCB->setChecked(_ngPost->_keepRar);

    _ui->redundancySB->setRange(0, 100);
    _ui->redundancySB->setValue(static_cast<int>(_ngPost->_par2Pct));
    _ui->redundancySB->setEnabled(_ngPost->_par2Args.isEmpty());

    if (!_ngPost->_rarPassFixed.isEmpty())
    {
        _ui->nzbPassCB->setChecked(true);
        _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
    }

    _ui->nameLengthSB->setRange(5, 50);
    _ui->nameLengthSB->setValue(static_cast<int>(_ngPost->_lengthName));
    _ui->passLengthSB->setRange(5, 50);
    _ui->passLengthSB->setValue(static_cast<int>(_ngPost->_lengthPass));

    _ui->filesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(_ui->selectFilesButton, &QAbstractButton::clicked, this, &PostingWidget::onSelectFilesClicked);
    connect(_ui->selectFolderButton,&QAbstractButton::clicked, this, &PostingWidget::onSelectFolderClicked);
    connect(_ui->clearFilesButton,  &QAbstractButton::clicked, this, &PostingWidget::onClearFilesClicked);
    connect(_ui->filesList,         &SignedListWidget::empty,  this, &PostingWidget::onClearFilesClicked, Qt::QueuedConnection);

    connect(_ui->compressCB,        &QAbstractButton::toggled, this, &PostingWidget::onCompressCB);
    connect(_ui->genCompressName,   &QAbstractButton::clicked, this, &PostingWidget::onGenCompressName);
    connect(_ui->compressNameEdit,  &QLineEdit::textEdited, this, [this](){ _nameFromFile = false; });


    connect(_ui->compressPathButton,&QAbstractButton::clicked, this, &PostingWidget::onCompressPathClicked);
    connect(_ui->rarPathButton,     &QAbstractButton::clicked, this, &PostingWidget::onRarPathClicked);

    connect(_ui->nzbFileButton,     &QAbstractButton::clicked, this, &PostingWidget::onNzbFileClicked);

    connect(_ui->aboutButton,       &QAbstractButton::clicked, _ngPost, &NgPost::onAboutClicked);


    onCompressCB(_ngPost->_doCompress);
    if (_ngPost->_doCompress)
        _ui->compressCB->setChecked(true);
    if (_ngPost->_genName)
        onGenCompressName();
    if (_ngPost->_genPass)
    {
        _ui->nzbPassCB->setChecked(true);
        onGenNzbPassword();
    }
    if (_ngPost->_doPar2)
        _ui->par2CB->setChecked(true);
    if (_ngPost->_keepRar)
        _ui->keepRarCB->setChecked(true);

    QString fixedPass = _hmi->fixedArchivePassword();
    if (!fixedPass.isEmpty())
        _ui->nzbPassEdit->setText(fixedPass);

    // the options of the last session, unless the options of the command line / PACK decide
    if (_ngPost->_packAuto)
        _profileBar->showMatching(_ngPost->_quickProfile); // the options were set by "compress automatically": show the matching profile
    else if (_ngPost->_quickOptionsSet)
        _profileBar->restore(_ngPost->_quickOptions, _ngPost->_quickProfile);
    else
        _profileBar->applyRecommendedIfNew(); // first start of a new installation
    // newsgroups, servers, NZB folder and name template of the last session
    _applyTargets(_ngPost->_quickOptions.targets, false);
}

void PostingWidget::genNameAndPassword(bool genName, bool genPass, bool doPar2, bool useRarMax)
{
    _ui->compressCB->setChecked(_ngPost->_doCompress);
    if (genName)
        onGenCompressName();
    if (genPass && _ngPost->_rarPassFixed.isEmpty())
    {
        _ui->nzbPassCB->setChecked(true);        
        onGenNzbPassword();
    }
    _ui->rarMaxCB->setChecked(useRarMax);

    if (doPar2)
        _ui->par2CB->setChecked(true);

    _ui->keepRarCB->setChecked(_ngPost->_keepRar);
}



void PostingWidget::udatePostingParams()
{
    if (!_ui->nzbFileEdit->text().isEmpty())
    {
        QFileInfo nzb(_ui->nzbFileEdit->text());
        if (!nzb.absolutePath().isEmpty())
        {
            if (_nzbDir.isEmpty())
                _ngPost->_nzbPath = nzb.absolutePath();
            else if (QDir(nzb.absolutePath()) == QDir(_ngPost->_nzbPath))
                _nzbDir.clear(); // back in the folder of the Settings
            else
                _nzbDir = nzb.absolutePath(); // the folder of a profile is not the one of the Settings
        }
        _ngPost->setNzbName(nzb);
    }

    // fetch compression settings
    _ngPost->_tmpPath    = _ui->compressPathEdit->text();
    _ngPost->_doCompress = _ui->compressCB->isChecked();
    _ngPost->_rarPath    = _ui->rarEdit->text();
    _ngPost->_rarName    = _ui->compressNameEdit->text();
    if (_ui->nzbPassCB->isChecked())
        _ngPost->_rarPass = _ui->nzbPassEdit->text().toLocal8Bit();
    else
        _ngPost->_rarPass = QString();
    _ngPost->_lengthName = static_cast<uint>(_ui->nameLengthSB->value());
    _ngPost->_lengthPass = static_cast<uint>(_ui->passLengthSB->value());
    uint val = 0;
    bool ok  = true;
    _ngPost->_rarSize = 0;
    if (!_ui->rarSizeEdit->text().isEmpty())
    {
        val = _ui->rarSizeEdit->text().toUInt(&ok);
        if (ok)
            _ngPost->_rarSize = val;
    }
    _ngPost->_useRarMax = _ui->rarMaxCB->isChecked();

    // fetch par2 settings
    _ngPost->_doPar2  = _ui->par2CB->isChecked();
    if (_ngPost->_par2Args.isEmpty())
        _ngPost->_par2Pct = static_cast<uint>(_ui->redundancySB->value());

    _ngPost->_keepRar = _ui->keepRarCB->isChecked();
}

void PostingWidget::retranslate()
{
    _ui->retranslateUi(this);
    _retranslateLayout();
    if (_profileBar)
        _profileBar->retranslate();
    if (_targets)
        _targets->retranslate();
    if (_nfoCard)
        _nfoCard->retranslate();
    _ui->filesList->setEmptyState(tr("Drag files or folders here, or choose them with the buttons below."),
                                  tr("Select Files"), tr("Select Folder"));
    _ui->rarMaxCB->setToolTip(tr("limit the number of archive volume to %1 (cf config RAR_MAX)").arg(_ngPost->_rarMax));
    _ui->redundancySB->setToolTip(tr("Using PAR2_ARGS from config file: %1").arg(_ngPost->_par2Args));
    _ui->filesList->setToolTip(QString("%1<ul><li>%2</li><li>%3</li><li>%4</li></ul>%5").arg(
                                   tr("You can add files or folder by:")).arg(
                                   tr("Drag & Drop files/folders")).arg(
                                   tr("Right Click to add Files")).arg(
                                   tr("Click on Select Files/Folder buttons")).arg(
                                   tr("Bare in mind you can select items in the list and press DEL to remove them")));
}

void PostingWidget::setNzbPassword(const QString &pass)
{
    _ui->nzbPassCB->setChecked(true);
    _ui->nzbPassEdit->setText(pass);
}

void PostingWidget::setPackingAuto(bool enabled, const QStringList &keys)
{
    bool compress = false, genName = false, genPass = false, doPar2 = false;
    if (enabled)
    {
        for (auto it = keys.cbegin(), itEnd = keys.cend(); it != itEnd; ++it)
        {
            QString keyWord = (*it).toLower();
            if (keyWord == NgPost::optionName(NgPost::Opt::COMPRESS))
                compress = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_NAME))
                genName = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_PASS))
                genPass = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_PAR2))
                doPar2 = true;
        }
    }
    _ui->nzbPassCB->setChecked(genPass);
    _ui->compressCB->setChecked(compress);
    _ui->par2CB->setChecked(doPar2);


    if (compress)
    {
        if (_ui->nzbPassEdit->text().isEmpty())
        {
            if (_hmi->useFixedPassword())
                _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
            else if (genPass)
                onGenNzbPassword();
        }
        if (genName && _ui->compressNameEdit->text().isEmpty())
            onGenCompressName();
    }
}

void PostingWidget::addPath(const QString &path, int currentNbFiles, int isDir)
{
    if (_ui->filesList->addPathIfNotInList(path, currentNbFiles, isDir))
    {
        QFileInfo fileInfo(path);
        if (_ui->nzbFileEdit->text().isEmpty())
        {
            _ngPost->setNzbName(fileInfo);
            _nzbBase = _ngPost->_nzbName;
            _setAutoNzbFile(); // template and folder of the profile
        }
        if (_ui->compressNameEdit->text().isEmpty())
        {
            _ui->compressNameEdit->setText(_ngPost->_nzbName);
            _nameFromFile = true;
        }
    }
}

bool PostingWidget::_fileAlreadyInList(const QString &fileName, int currentNbFiles) const
{
    for (int i = 0 ; i < currentNbFiles ; ++i)
    {
        if (_ui->filesList->item(i)->text() == fileName)
            return true;
    }
    return false;
}




//========================================================================
// ngPost Deluxe layout: file list, then the NZB and Archive cards,
// advanced options (folders, rar, volumes) hidden by default, Post button
//========================================================================
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>

bool PostingWidget::sShowAdvanced = false;

static QFrame *newCard(QLabel *&title, QGridLayout *&grid)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground, true);
    QVBoxLayout *v = new QVBoxLayout(card);
    v->setContentsMargins(16, 12, 16, 14);
    v->setSpacing(10);
    title = new QLabel(card);
    title->setObjectName("cardTitle");
    v->addWidget(title);
    grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    v->addLayout(grid);
    return card;
}

static QHBoxLayout *row(std::initializer_list<QWidget*> widgets, bool stretch = false)
{
    QHBoxLayout *h = new QHBoxLayout();
    h->setSpacing(8);
    for (QWidget *w : widgets)
        h->addWidget(w);
    if (stretch)
        h->addStretch();
    return h;
}

void PostingWidget::_buildLayout()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(page);
    root->setContentsMargins(2, 12, 2, 2);
    root->setSpacing(12);

    // files: title + small tool bar, then the list (with its empty screen)
    QHBoxLayout *head = new QHBoxLayout();
    head->setSpacing(8);
    _filesTitle = new QLabel(page);
    _filesTitle->setObjectName("cardTitle");
    head->addWidget(_filesTitle);
    head->addStretch();
    head->addWidget(_ui->selectFilesButton);
    head->addWidget(_ui->selectFolderButton);
    head->addWidget(_ui->clearFilesButton);
    root->addLayout(head);
    _ui->filesList->setMinimumHeight(100); // small windows: the list gets smaller before the page has to scroll
    _ui->filesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    root->addWidget(_ui->filesList, 1);

    // reminder: everything in the list is one release
    _releaseInfo = new QLabel(page);
    _releaseInfo->setObjectName("infoBanner");
    _releaseInfo->setWordWrap(true);
    _releaseInfo->setTextFormat(Qt::RichText);
    _releaseInfo->hide();
    root->addWidget(_releaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsInserted, this, &PostingWidget::_updateReleaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsRemoved,  this, &PostingWidget::_updateReleaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::modelReset,   this, &PostingWidget::_updateReleaseInfo);

    // profiles of options (shared with Auto Posting)
    _profileBar = new ProfileBar(_ngPost, page);
    _profileBar->setHandlers([this](){ return _profileForSave(); }, [this](const PostProfile &p){ applyProfile(p); });
    root->addWidget(_profileBar);

    // NZB and Archive cards side by side
    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(12);
    QGridLayout *g = nullptr;

    QFrame *nzb = newCard(_nzbTitle, g);
    g->addWidget(_ui->nzbFileLbl, 0, 0);
    g->addLayout(row({_ui->nzbFileEdit, _ui->nzbFileButton}), 0, 1);
    _targets = new PostTargetsBox(_ngPost, nzb, false);
    _targets->addRows(g, 1);
    g->setColumnStretch(1, 1);
    _ui->nzbFileEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->nzbPassEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    static_cast<QVBoxLayout*>(nzb->layout())->addStretch(1); // the title and the file field stay at the top
    cards->addWidget(nzb, 1);

    QFrame *archive = newCard(_archiveTitle, g);
    g->addWidget(_ui->compressCB, 0, 0);
    g->addLayout(row({_ui->compressNameEdit, _ui->nameLengthSB, _ui->genCompressName}), 0, 1);
    g->addWidget(_ui->par2CB, 1, 0);
    g->addLayout(row({_ui->redundancyLbl, _ui->redundancySB, _ui->keepRarCB}, true), 1, 1);
    g->addWidget(_ui->nzbPassCB, 2, 0);
    g->addLayout(row({_ui->nzbPassEdit, _ui->passLengthSB, _ui->genPass}), 2, 1);
    g->setColumnStretch(1, 1);
    _ui->compressNameEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    cards->addWidget(archive, 1);
    root->addLayout(cards);

    // NFO file (shared with Auto Posting)
    _nfoCard = new NfoCard(_ngPost, page);
    root->addWidget(_nfoCard);

    // advanced options
    _advancedCard = newCard(_advancedTitle, g);
    g->addWidget(_ui->compressPathLbl, 0, 0);
    g->addLayout(row({_ui->compressPathEdit, _ui->compressPathButton}), 0, 1);
    g->addWidget(_ui->rarLbl, 1, 0);
    g->addLayout(row({_ui->rarEdit, _ui->rarPathButton}), 1, 1);
    g->addWidget(_ui->rarSizeLbl, 2, 0);
    g->addLayout(row({_ui->rarSizeEdit, _ui->rarMaxCB}, true), 2, 1);
    g->setColumnStretch(1, 1);
    _ui->compressPathEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarSizeEdit->setMaximumWidth(100);
    root->addWidget(_advancedCard);

    // summary of the post (size, estimated upload time)
    _summary = new QLabel(page);
    _summary->setObjectName("hint");
    _summary->hide();
    root->addWidget(_summary);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsInserted, this, [this](){ _updateSummary(true); });
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsRemoved,  this, [this](){ _updateSummary(true); });
    connect(_ui->filesList->model(), &QAbstractItemModel::modelReset,   this, [this](){ _updateSummary(true); });
    connect(_ui->compressCB, &QAbstractButton::toggled, this, [this](){ _updateSummary(false); });
    connect(_ui->par2CB,     &QAbstractButton::toggled, this, [this](){ _updateSummary(false); });
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ _updateSummary(false); });

    // bottom: advanced toggle on the left, Post button in the middle of the page
    QGridLayout *bottom = new QGridLayout();
    _advancedToggle = new QPushButton(page);
    _advancedToggle->setObjectName("linkButton");
    _advancedToggle->setCursor(Qt::PointingHandCursor);
    connect(_advancedToggle, &QAbstractButton::clicked, this, [this](){ _showAdvanced(!sShowAdvanced); });
    bottom->addWidget(_advancedToggle, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    _ui->postButton->setObjectName("accentButton");
    _ui->postButton->setIcon(QIcon());
    _ui->postButton->setMinimumSize(170, 38);
    bottom->addWidget(_ui->postButton, 0, 1, Qt::AlignCenter);
    _addPausedCB = new QCheckBox(page);
    bottom->addWidget(_addPausedCB, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    bottom->setHorizontalSpacing(0); // the left and right columns have the same stretch: the button is really in the middle
    bottom->setColumnStretch(0, 1);
    bottom->setColumnStretch(1, 0);
    bottom->setColumnStretch(2, 1);
    // the bottom row is added under the scroll area (see below): the Post button is always in view

    _ui->aboutButton->hide(); // About is in the side bar

    // the .ui limited the width of some labels and check boxes (they were on one long line)
    for (QWidget *w : page->findChildren<QWidget*>())
        if (qobject_cast<QLabel*>(w) || qobject_cast<QCheckBox*>(w))
            w->setMaximumWidth(QWIDGETSIZE_MAX);

    // the .ui limited the width of some labels and check boxes (they were on one long line)
    for (QWidget *w : page->findChildren<QWidget*>())
        if (qobject_cast<QLabel*>(w) || qobject_cast<QCheckBox*>(w))
            w->setMaximumWidth(QWIDGETSIZE_MAX);

    delete layout(); // the old layout of the .ui (the widgets have been moved in the new one)
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    // a window that is not tall enough scrolls the page instead of squeezing the cards on top of each other
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    page->setObjectName("pageContent");
    scroll->setWidget(page);
    outer->addWidget(scroll, 1);

    // fixed bar under the scrolling page: the Post button must not disappear when the page gets taller than the window
    QWidget *footer = new QWidget(this);
    bottom->setContentsMargins(2, 4, 2, 2);
    footer->setLayout(bottom);
    outer->addWidget(footer);

    // a change made by hand: the profile is now "Custom"
    for (QAbstractButton *b : {static_cast<QAbstractButton*>(_ui->compressCB), static_cast<QAbstractButton*>(_ui->par2CB),
                               static_cast<QAbstractButton*>(_ui->keepRarCB), static_cast<QAbstractButton*>(_ui->nzbPassCB)})
        connect(b, &QAbstractButton::clicked, _profileBar, &ProfileBar::markCustom);
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), _profileBar, &ProfileBar::markCustom);
    // name template, newsgroups or servers changed by hand: Custom if the profile sets them
    connect(_targets, &PostTargetsBox::changed, this, [this](int part) {
        if (part == PostTargetsBox::NAME)
            _refreshAutoNames();
        if (isVisible())
            _hmi->autoSaveForm(0);
        const PostProfile *p = _profileBar->selectedProfile();
        if (!p)
            return;
        if ((part == PostTargetsBox::NAME && !p->targets.nameTemplate.isEmpty())
                || (part == PostTargetsBox::GROUPS && !p->targets.groups.isEmpty())
                || (part == PostTargetsBox::SERVERS && !p->targets.servers.isEmpty()))
            _profileBar->markCustom();
    });

    _showAdvanced(sShowAdvanced);
    _retranslateLayout();
}

QString PostingWidget::profileName() const
{
    return _profileBar ? _profileBar->selectedName() : QString();
}

PostProfile PostingWidget::currentProfile() const
{
    PostProfile p;
    p.targets = _targets->targets();
    p.targets.nzbDir = _nzbDir;
    p.compress = _ui->compressCB->isChecked();
    p.genName  = p.compress && !_ui->compressNameEdit->text().isEmpty();
    p.genPass  = _ui->nzbPassCB->isChecked();
    p.par2     = _ui->par2CB->isChecked();
    p.par2Pct  = static_cast<uint>(_ui->redundancySB->value());
    p.keepRar  = _ui->keepRarCB->isChecked();
    return p;
}

bool PostingWidget::isEmptyAndIdle() const
{
    return _ui->filesList->count() == 0;
}

void PostingWidget::applyRequest(const PostRequest &request)
{
    // the profile first: a random name is only made while the name of the archive is empty (the files fill it)
    _profileBar->select(request.profile);
    for (const QString &path : request.paths)
    {
        QFileInfo fi(path);
        if (!path.trimmed().isEmpty())
            addPath(fi.absoluteFilePath(), _ui->filesList->count(), fi.isDir());
    }
    _updateReleaseInfo();
    _updateSummary();
}

int PostingWidget::startFromRequest()
{
    if (_ui->filesList->count() == 0)
        return -1;
    return postFiles(true);
}

void PostingWidget::applyProfile(const PostProfile &p)
{
    _ui->compressCB->setChecked(p.compress);
    _ui->par2CB->setChecked(p.par2);
    if (_ngPost->_par2Args.isEmpty() && p.par2Pct > 0)
        _ui->redundancySB->setValue(static_cast<int>(p.par2Pct));
    _ui->keepRarCB->setChecked(p.keepRar);
    _ui->nzbPassCB->setChecked(p.genPass);
    if (p.compress)
    {
        if (p.genPass && _ui->nzbPassEdit->text().isEmpty())
        {
            if (_hmi->useFixedPassword())
                _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
            else
                onGenNzbPassword();
        }
        // the name taken from the first file is replaced by a random one, a name typed by the user is kept
        if (p.genName && (_ui->compressNameEdit->text().isEmpty() || _nameFromFile))
            onGenCompressName();
    }
    _applyTargets(p.targets, true); // the parts that the profile doesn't have are not changed
}

void PostingWidget::_showAdvanced(bool show)
{
    sShowAdvanced = show;
    _advancedCard->setVisible(show);
    _retranslateLayout();
}

void PostingWidget::_updateReleaseInfo()
{
    if (!_releaseInfo)
        return;
    _releaseInfo->setVisible(_ui->filesList->count() > 1);
}

void PostingWidget::_retranslateLayout()
{
    if (!_filesTitle)
        return;
    _releaseInfo->setText(tr("<b>One release:</b> all the files and folders of this list are posted together, "
                             "with a single NZB file and, if Compress is on, a single archive. "
                             "To post separate releases, post them one after the other (the page <i>Posts</i> shows them all) "
                             "or use the <i>%1</i> tab, which makes one post for each file or folder.").arg(
                              _ngPost->folderMonitoringName()));
    _updateReleaseInfo();
    _filesTitle->setText(tr("Files to post"));
    _nzbTitle->setText(tr("NZB"));
    _archiveTitle->setText(tr("Archive"));
    _advancedTitle->setText(tr("Advanced options"));
    _advancedToggle->setText(sShowAdvanced ? tr("▾ Hide advanced options") : tr("▸ Advanced options (temporary folder, rar / 7z, volumes)"));

    _ui->nzbFileLbl->setText(tr("File"));
    _ui->nzbPassCB->setText(tr("Password"));
    _ui->compressCB->setText(tr("Compress"));
    _ui->par2CB->setText(tr("par2"));
    _ui->redundancyLbl->setText(tr("redundancy (%)"));
    _ui->keepRarCB->setText(tr("keep the archives"));
    _ui->compressPathLbl->setText(tr("Temporary folder"));
    _ui->rarLbl->setText(tr("rar / 7z program"));
    _ui->rarSizeLbl->setText(tr("Volume size (MB)"));
    _ui->rarMaxCB->setText(tr("limit the number of volumes"));
    _ui->clearFilesButton->setText(tr("Remove All"));
    _ui->postButton->setText(tr("Post Files"));
    if (_addPausedCB)
    {
        _addPausedCB->setText(tr("Add paused"));
        _addPausedCB->setToolTip(tr("The post goes in the queue (page Posts) but doesn't start: resume it there when you want"));
    }
}
