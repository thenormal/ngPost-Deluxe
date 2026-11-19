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

#include "GuidedPostWidget.h"
#include "MainWindow.h"
#include "ServersWidget.h"
#include "NgPost.h"
#include "utils/NfoWriter.h"
#include "utils/MediaInfoReader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFrame>
#include <QScrollArea>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QMessageBox>
#include <QUrl>

static bool sameOptions(const PostProfile &a, const PostProfile &b)
{
    return a.compress == b.compress && a.genName == b.genName && a.genPass == b.genPass
            && a.par2 == b.par2 && a.par2Pct == b.par2Pct && a.keepRar == b.keepRar;
}

static QFrame *newCard(QWidget *parent)
{
    QFrame *card = new QFrame(parent);
    card->setObjectName("card");
    return card;
}

static QLabel *newLabel(const QString &objectName, QWidget *parent, bool wrap = true)
{
    QLabel *l = new QLabel(parent);
    if (!objectName.isEmpty())
        l->setObjectName(objectName);
    l->setWordWrap(wrap);
    return l;
}

GuidedPostWidget::GuidedPostWidget(NgPost *ngPost, MainWindow *hmi, QWidget *parent) :
    QWidget(parent), _ngPost(ngPost), _hmi(hmi), _editing(-1), _step(STEP_WHAT), _loading(false)
{
    _req = _ngPost->_guidedHasChoices ? _ngPost->_guidedChoices : PostRequest();
    _req.paths.clear();
    _req.folder.clear();
    _build();
    connect(_ngPost, &NgPost::profilesChanged, this, [this](){
        QString sel = _profileCombo->currentIndex() >= 2 ? _profileCombo->currentText() : QString();
        _loadProfiles(sel);
    });
    _fillFromRequest();
    _goTo(STEP_WHAT);
    retranslate();
}

int GuidedPostWidget::currentStep() const { return _step; }

// ============================================================ layout

void GuidedPostWidget::_build()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 18, 24, 12);
    root->setSpacing(6);

    _title    = newLabel("pageTitle", this, false);
    _subtitle = newLabel("pageSubtitle", this);
    root->addWidget(_title);
    root->addWidget(_subtitle);

    // step indicator
    QHBoxLayout *stepRow = new QHBoxLayout();
    stepRow->setContentsMargins(0, 8, 0, 0);
    _stepLabel = newLabel("hint", this, false);
    _progress = new QProgressBar(this);
    _progress->setRange(0, NB_STEPS);
    _progress->setTextVisible(false);
    _progress->setFixedHeight(8);
    stepRow->addWidget(_stepLabel);
    stepRow->addWidget(_progress, 1);
    root->addLayout(stepRow);

    _stepTitle = newLabel("cardTitle", this, false);
    _stepHint  = newLabel("hint", this);
    root->addWidget(_stepTitle);
    root->addWidget(_stepHint);

    // the steps (a window that is not tall enough scrolls)
    _pages = new QStackedWidget(this);
    _pages->addWidget(_buildStepWhat());
    _pages->addWidget(_buildStepFiles());
    _pages->addWidget(_buildStepOptions());
    _pages->addWidget(_buildStepWhere());
    _pages->addWidget(_buildStepSummary());
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pages->setObjectName("pageContent");
    scroll->setWidget(_pages);
    root->addWidget(scroll, 1);

    // buttons
    QHBoxLayout *foot = new QHBoxLayout();
    foot->setContentsMargins(0, 6, 0, 0);
    _cancelButton = new QPushButton(this);
    _backButton   = new QPushButton(this);
    _nextButton   = new QPushButton(this);
    _addAnother   = new QPushButton(this);
    _postButton   = new QPushButton(this);
    _nextButton->setObjectName("primaryButton");
    _postButton->setObjectName("primaryButton");
    for (QPushButton *b : {_cancelButton, _backButton, _nextButton, _addAnother, _postButton})
    {
        b->setMinimumHeight(34);
        b->setCursor(Qt::PointingHandCursor);
        b->setAutoDefault(false);
    }
    foot->addWidget(_cancelButton);
    foot->addStretch();
    foot->addWidget(_backButton);
    foot->addWidget(_nextButton);
    foot->addWidget(_addAnother);
    foot->addWidget(_postButton);
    root->addLayout(foot);

    connect(_cancelButton, &QAbstractButton::clicked, this, &GuidedPostWidget::_onCancel);
    connect(_backButton,   &QAbstractButton::clicked, this, &GuidedPostWidget::_onBack);
    connect(_nextButton,   &QAbstractButton::clicked, this, &GuidedPostWidget::_onNext);
    connect(_addAnother,   &QAbstractButton::clicked, this, &GuidedPostWidget::_onAddAnother);
    connect(_postButton,   &QAbstractButton::clicked, this, &GuidedPostWidget::_onPost);
}

QWidget *GuidedPostWidget::_buildStepWhat()
{
    QWidget *page = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 8, 0, 0);
    l->setSpacing(10);
    _kindGroup = new QButtonGroup(page);
    for (int i = 0; i < 4; ++i)
    {
        _kindButtons[i] = new QRadioButton(page);
        _kindHints[i]   = newLabel("hint", page);
        _kindHints[i]->setContentsMargins(26, 0, 0, 6);
        _kindGroup->addButton(_kindButtons[i], i);
        l->addWidget(_kindButtons[i]);
        l->addWidget(_kindHints[i]);
    }
    _reuseHint = newLabel("infoBanner", page);
    l->addWidget(_reuseHint);
    l->addStretch();
    connect(_kindGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this](int id){
        if (_loading)
            return;
        _req.kind = static_cast<PostRequest::Kind>(id);
        _updateButtons();
    });
    return page;
}

QWidget *GuidedPostWidget::_buildStepFiles()
{
    QWidget *page = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 8, 0, 0);
    _filesStack = new QStackedWidget(page);
    l->addWidget(_filesStack);

    // items chosen by the user (one release / each item)
    QWidget *files = new QWidget(_filesStack);
    QVBoxLayout *fl = new QVBoxLayout(files);
    fl->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *bar = new QHBoxLayout();
    _filesLabel   = newLabel("cardTitle", files, false);
    _selectFiles  = new QPushButton(files);
    _selectFolder = new QPushButton(files);
    _removeAll    = new QPushButton(files);
    bar->addWidget(_filesLabel);
    bar->addStretch();
    bar->addWidget(_selectFiles);
    bar->addWidget(_selectFolder);
    bar->addWidget(_removeAll);
    fl->addLayout(bar);
    _fileList = new QListWidget(files);
    _fileList->setMinimumHeight(140);
    _fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fl->addWidget(_fileList, 1);
    _fileSummary = newLabel("hint", files);
    fl->addWidget(_fileSummary);
    _filesStack->addWidget(files);

    // a folder to scan or to watch
    QWidget *folder = new QWidget(_filesStack);
    QVBoxLayout *dl = new QVBoxLayout(folder);
    dl->setContentsMargins(0, 0, 0, 0);
    _folderLabel = newLabel("cardTitle", folder, false);
    dl->addWidget(_folderLabel);
    QHBoxLayout *row = new QHBoxLayout();
    _folderEdit   = new QLineEdit(folder);
    _folderBrowse = new QPushButton(folder);
    row->addWidget(_folderEdit, 1);
    row->addWidget(_folderBrowse);
    dl->addLayout(row);
    _folderPreview = new QListWidget(folder);
    _folderPreview->setMinimumHeight(120);
    _folderPreview->setSelectionMode(QAbstractItemView::NoSelection);
    dl->addWidget(_folderPreview, 1);
    _folderSummary = newLabel("hint", folder);
    dl->addWidget(_folderSummary);
    _filesStack->addWidget(folder);

    connect(_selectFiles, &QAbstractButton::clicked, this, [this](){
        const QStringList names = QFileDialog::getOpenFileNames(this, tr("Select Files"));
        for (const QString &n : names)
            _addPath(n);
        _updateFileSummary();
    });
    connect(_selectFolder, &QAbstractButton::clicked, this, [this](){
        const QString dir = QFileDialog::getExistingDirectory(this, tr("Select a Folder"), QString(), QFileDialog::ShowDirsOnly);
        if (!dir.isEmpty())
            _addPath(dir);
        _updateFileSummary();
    });
    connect(_removeAll, &QAbstractButton::clicked, this, [this](){
        _fileList->clear();
        _updateFileSummary();
    });
    connect(_fileList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        delete _fileList->takeItem(_fileList->row(item));
        _updateFileSummary();
    });
    connect(_folderBrowse, &QAbstractButton::clicked, this, [this](){
        const QString dir = QFileDialog::getExistingDirectory(this, tr("Select a Folder"), _folderEdit->text(), QFileDialog::ShowDirsOnly);
        if (!dir.isEmpty())
            _folderEdit->setText(QDir::toNativeSeparators(dir));
    });
    connect(_folderEdit, &QLineEdit::textChanged, this, [this](){ _loadFolderPreview(); _updateButtons(); });
    return page;
}

QWidget *GuidedPostWidget::_buildStepOptions()
{
    QWidget *page = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 8, 0, 0);
    l->setSpacing(10);

    QHBoxLayout *prof = new QHBoxLayout();
    _profileLabel = newLabel("", page, false);
    _profileCombo = new QComboBox(page);
    _profileCombo->setMinimumWidth(260);
    _customize = new QPushButton(page);
    prof->addWidget(_profileLabel);
    prof->addWidget(_profileCombo, 1);
    prof->addWidget(_customize);
    l->addLayout(prof);
    _profileDesc = newLabel("hint", page);
    l->addWidget(_profileDesc);

    // single options
    _customCard = newCard(page);
    QGridLayout *cg = new QGridLayout(_customCard);
    cg->setContentsMargins(14, 12, 14, 12);
    _compressCB = new QCheckBox(_customCard);
    _genNameCB  = new QCheckBox(_customCard);
    _genPassCB  = new QCheckBox(_customCard);
    _par2CB     = new QCheckBox(_customCard);
    _keepCB     = new QCheckBox(_customCard);
    _par2Pct    = new QSpinBox(_customCard);
    _par2Pct->setRange(1, 100);
    _par2Pct->setSuffix(" %");
    cg->addWidget(_compressCB, 0, 0);
    cg->addWidget(_genNameCB,  0, 1);
    cg->addWidget(_genPassCB,  0, 2);
    cg->addWidget(_par2CB,     1, 0);
    cg->addWidget(_par2Pct,    1, 1);
    cg->addWidget(_keepCB,     1, 2);
    l->addWidget(_customCard);

    // NFO
    _nfoCard = newCard(page);
    QVBoxLayout *nl = new QVBoxLayout(_nfoCard);
    nl->setContentsMargins(14, 12, 14, 12);
    _nfoCB       = new QCheckBox(_nfoCard);
    _nfoInPostCB = new QCheckBox(_nfoCard);
    _nfoPassCB   = new QCheckBox(_nfoCard);
    _nfoMediaCB  = new QCheckBox(_nfoCard);
    nl->addWidget(_nfoCB);
    for (QCheckBox *cb : {_nfoInPostCB, _nfoPassCB, _nfoMediaCB})
    {
        QHBoxLayout *h = new QHBoxLayout();
        h->setContentsMargins(22, 0, 0, 0);
        h->addWidget(cb);
        nl->addLayout(h);
    }
    l->addWidget(_nfoCard);

    // after the post (Auto Posting only)
    _afterCard = newCard(page);
    QHBoxLayout *al = new QHBoxLayout(_afterCard);
    al->setContentsMargins(14, 12, 14, 12);
    _afterLabel = newLabel("", _afterCard, false);
    _afterCombo = new QComboBox(_afterCard);
    _afterCombo->addItems({QString(), QString(), QString()});
    _moveToEdit = new QLineEdit(_afterCard);
    al->addWidget(_afterLabel);
    al->addWidget(_afterCombo);
    al->addWidget(_moveToEdit, 1);
    l->addWidget(_afterCard);
    l->addStretch();

    connect(_profileCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int idx){ _profileChosen(idx); });
    connect(_customize, &QAbstractButton::clicked, this, [this](){ _setCustomVisible(!_customCard->isVisible()); });
    for (QCheckBox *cb : {_compressCB, _genNameCB, _genPassCB, _par2CB, _keepCB, _nfoCB, _nfoInPostCB, _nfoPassCB, _nfoMediaCB})
        connect(cb, &QAbstractButton::toggled, this, [this](){ _optionsEdited(); });
    connect(_par2Pct, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ _optionsEdited(); });
    connect(_afterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ _optionsEdited(); });
    connect(_moveToEdit, &QLineEdit::textChanged, this, [this](){ _optionsEdited(); });
    return page;
}

QWidget *GuidedPostWidget::_buildStepWhere()
{
    QWidget *page = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 8, 0, 0);
    l->setSpacing(10);

    auto section = [this, page, l](QLabel *&title, QLabel *&text, QPushButton *&button) {
        QFrame *card = newCard(page);
        QHBoxLayout *h = new QHBoxLayout(card);
        h->setContentsMargins(14, 12, 14, 12);
        QVBoxLayout *v = new QVBoxLayout();
        title = newLabel("cardTitle", card, false);
        text  = newLabel("", card);
        text->setTextFormat(Qt::RichText);
        v->addWidget(title);
        v->addWidget(text);
        button = new QPushButton(card);
        button->setCursor(Qt::PointingHandCursor);
        h->addLayout(v, 1);
        h->addWidget(button, 0, Qt::AlignTop);
        l->addWidget(card);
    };
    section(_serversTitle, _serversText, _serversButton);
    section(_groupsTitle,  _groupsText,  _groupsButton);
    section(_nzbTitle,     _nzbText,     _nzbButton);
    _whereState = newLabel("infoBanner", page);
    l->addWidget(_whereState);
    _skipWhereCB = new QCheckBox(page);
    l->addWidget(_skipWhereCB);
    l->addStretch();

    connect(_serversButton, &QAbstractButton::clicked, this, [this](){ _hmi->showServersPage(); });
    connect(_groupsButton,  &QAbstractButton::clicked, this, [this](){ _hmi->showSettingsPage(MainWindow::CAT_POST); });
    connect(_nzbButton,     &QAbstractButton::clicked, this, [this](){ _hmi->showSettingsPage(MainWindow::CAT_NZB); });
    connect(_skipWhereCB, &QAbstractButton::toggled, this, [this](bool on){
        if (_loading)
            return;
        _ngPost->_guidedSkipWhere = on;
        _hmi->scheduleAutoSave();
    });
    return page;
}

QWidget *GuidedPostWidget::_buildStepSummary()
{
    QWidget *page = new QWidget();
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(0, 8, 0, 0);
    l->setSpacing(8);

    _readyBox = new QWidget(page);
    QVBoxLayout *readyLayout = new QVBoxLayout(_readyBox);
    readyLayout->setContentsMargins(0, 0, 0, 0);
    readyLayout->setSpacing(6);
    _readyTitle = newLabel("cardTitle", _readyBox, false);
    readyLayout->addWidget(_readyTitle);
    QHBoxLayout *rr = new QHBoxLayout();
    _readyList = new QListWidget(_readyBox);
    _readyList->setMaximumHeight(110);
    QVBoxLayout *rb = new QVBoxLayout();
    _editReady   = new QPushButton(_readyBox);
    _removeReady = new QPushButton(_readyBox);
    rb->addWidget(_editReady);
    rb->addWidget(_removeReady);
    rb->addStretch();
    rr->addWidget(_readyList, 1);
    rr->addLayout(rb);
    readyLayout->addLayout(rr);
    l->addWidget(_readyBox);

    _thisTitle = newLabel("cardTitle", page, false);
    l->addWidget(_thisTitle);
    QFrame *card = newCard(page);
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(14, 10, 14, 10);
    _thisText = newLabel("", card);
    cl->addWidget(_thisText);
    l->addWidget(card);

    _checksTitle = newLabel("cardTitle", page, false);
    l->addWidget(_checksTitle);
    _checksBox = new QWidget(page);
    _checksLayout = new QVBoxLayout(_checksBox);
    _checksLayout->setContentsMargins(0, 0, 0, 0);
    _checksLayout->setSpacing(4);
    l->addWidget(_checksBox);

    QHBoxLayout *sp = new QHBoxLayout();
    _saveProfileCB = new QCheckBox(page);
    _profileName   = new QLineEdit(page);
    _profileName->setEnabled(false);
    sp->addWidget(_saveProfileCB);
    sp->addWidget(_profileName, 1);
    l->addLayout(sp);
    l->addStretch();

    connect(_saveProfileCB, &QAbstractButton::toggled, _profileName, &QWidget::setEnabled);
    connect(_readyList, &QListWidget::currentRowChanged, this, [this](){ _updateButtons(); });
    connect(_removeReady, &QAbstractButton::clicked, this, [this](){
        int row = _readyList->currentRow();
        if (row >= 0 && row < _ready.size())
        {
            _ready.removeAt(row);
            _refreshSummary();
        }
    });
    connect(_editReady, &QAbstractButton::clicked, this, [this](){
        int row = _readyList->currentRow();
        if (row < 0 || row >= _ready.size())
            return;
        if (_req.isValid()) // the post being prepared is kept as one more ready post
            _commitCurrent();
        row = qMin(row, _ready.size() - 1);
        _req = _ready.takeAt(row);
        _editing = -1;
        _fillFromRequest();
        _goTo(STEP_WHAT);
    });
    return page;
}

// ============================================================ texts

void GuidedPostWidget::retranslate()
{
    _title->setText(tr("Guided Post"));
    _subtitle->setText(tr("Answer a few questions and the post is prepared for you. It is made by Quick Post or Auto Posting, with the same checks."));

    _cancelButton->setText(tr("Cancel"));
    _backButton->setText(tr("Back"));
    _nextButton->setText(tr("Next"));
    _addAnother->setText(tr("Add another post"));

    _kindButtons[0]->setText(tr("Post these files or folders as one release"));
    _kindHints[0]->setText(tr("One NZB file and, with compression, one archive for everything."));
    _kindButtons[1]->setText(tr("Post each file or folder as its own release"));
    _kindHints[1]->setText(tr("Each item becomes a post with its own NZB file."));
    _kindButtons[2]->setText(tr("Scan a folder once and post what is inside"));
    _kindHints[2]->setText(tr("Each file or folder inside it becomes a post."));
    _kindButtons[3]->setText(tr("Watch a folder and post what arrives"));
    _kindHints[3]->setText(tr("New files and folders are posted by themselves, until you stop watching."));

    _filesLabel->setText(tr("Files and folders to post"));
    _selectFiles->setText(tr("Select Files"));
    _selectFolder->setText(tr("Select Folder"));
    _removeAll->setText(tr("Remove All"));
    _folderBrowse->setText(tr("Choose..."));

    _profileLabel->setText(tr("Profile:"));
    _customize->setText(_customCard->isVisible() ? tr("Hide the options") : tr("Customize"));
    _compressCB->setText(tr("Compress"));
    _genNameCB->setText(tr("Random name"));
    _genPassCB->setText(tr("Random password"));
    _par2CB->setText(tr("par2"));
    _keepCB->setText(tr("Keep the archives"));
    _nfoCB->setText(tr("Write an NFO file next to the NZB"));
    _nfoInPostCB->setText(tr("Also post the NFO file with the files"));
    _nfoPassCB->setText(tr("Write the password of the archive in the NFO file"));
    _nfoMediaCB->setText(tr("Media information (MediaInfo)"));
    _afterLabel->setText(tr("After posting:"));
    _afterCombo->setItemText(0, tr("Do nothing with the files"));
    _afterCombo->setItemText(1, tr("Move them to a \"Posted\" folder"));
    _afterCombo->setItemText(2, tr("Delete them"));
    _moveToEdit->setPlaceholderText(tr("Posted (a name, or a full path)"));
    QString sel = _profileCombo->currentIndex() >= 2 ? _profileCombo->currentText() : QString();
    int idx = _profileCombo->currentIndex();
    _loadProfiles(sel);
    if (sel.isEmpty() && idx >= 0 && idx < 2)
        _profileCombo->setCurrentIndex(idx);

    _serversTitle->setText(tr("Servers"));
    _groupsTitle->setText(tr("Newsgroups"));
    _nzbTitle->setText(tr("Folder of the NZB files"));
    _serversButton->setText(tr("Open Servers"));
    _groupsButton->setText(tr("Open Settings"));
    _nzbButton->setText(tr("Open Settings"));
    _skipWhereCB->setText(tr("Skip this step when everything is set"));

    _readyTitle->setText(tr("Posts ready"));
    _editReady->setText(tr("Edit"));
    _removeReady->setText(tr("Remove"));
    _thisTitle->setText(tr("This post"));
    _checksTitle->setText(tr("Checks"));
    _saveProfileCB->setText(tr("Save these choices as a profile"));
    _profileName->setPlaceholderText(tr("Name of the profile"));

    _updateProfileDesc();
    _applyStepTexts();
    _refreshWhere();
    _refreshSummary();
}

void GuidedPostWidget::_applyStepTexts()
{
    static const int stepNumber[NB_STEPS] = {1, 2, 3, 4, 5};
    _stepLabel->setText(tr("Step %1 of %2").arg(stepNumber[_step]).arg(NB_STEPS));
    _progress->setValue(_step + 1);
    const bool files = !_req.usesAutoPosting() || _req.kind == PostRequest::Kind::EachItem;
    switch (_step)
    {
    case STEP_WHAT:
        _stepTitle->setText(tr("What do you want to do?"));
        _stepHint->setText(tr("Choose how the files will be posted."));
        break;
    case STEP_FILES:
        _stepTitle->setText(tr("What to post"));
        _stepHint->setText(files ? tr("Add the files or folders you want to post. Double click an item to remove it.")
                                 : tr("Choose the folder."));
        break;
    case STEP_OPTIONS:
        _stepTitle->setText(tr("Options"));
        _stepHint->setText(tr("Choose how the post is prepared. You can leave the suggested profile."));
        break;
    case STEP_WHERE:
        _stepTitle->setText(tr("Where to post"));
        _stepHint->setText(tr("The servers, the newsgroups and the folder of the NZB files are those of the program."));
        break;
    default:
        _stepTitle->setText(tr("Summary"));
        _stepHint->setText(tr("Check what will be posted, then press Post."));
        break;
    }
}

QString GuidedPostWidget::_kindName(PostRequest::Kind kind) const
{
    return _kindButtons[static_cast<int>(kind)]->text();
}

QString GuidedPostWidget::_describeProfile(const PostProfile &p)
{
    QStringList parts;
    if (p.compress)
        parts << tr("compression");
    if (p.compress && p.genName)
        parts << tr("random name");
    if (p.compress && p.genPass)
        parts << tr("random password");
    if (p.par2)
        parts << tr("par2 %1 %").arg(p.par2Pct);
    if (p.compress && p.keepRar)
        parts << tr("the archives are kept");
    if (parts.isEmpty())
        return tr("The files are posted as they are.");
    return tr("Uses: %1.").arg(parts.join(", "));
}

QString GuidedPostWidget::_describe(const PostRequest &r) const
{
    QStringList lines;
    lines << QString("<b>%1</b>").arg(_kindName(r.kind).toHtmlEscaped());
    if (r.kind == PostRequest::Kind::ScanFolder || r.kind == PostRequest::Kind::WatchFolder)
        lines << tr("Folder: %1").arg(QDir::toNativeSeparators(r.folder).toHtmlEscaped());
    else
    {
        bool hasFolder = false;
        QFileInfoList files = _requestFiles(r, &hasFolder);
        qint64 bytes = 0;
        for (const QFileInfo &fi : files)
            bytes += NgPost::recursiveSize(fi);
        lines << tr("%1 item(s), %2").arg(files.size()).arg(NfoWriter::humanSize(bytes));
    }
    lines << _describeProfile(r.profile).toHtmlEscaped();
    if (r.genNfo || r.nfoInPost)
        lines << tr("An NFO file is written next to the NZB.");
    if (r.usesAutoPosting())
    {
        if (r.after == PostRequest::After::MoveToPosted)
            lines << tr("Once posted, the files are moved to \"%1\".").arg((r.moveTo.isEmpty() ? QString("Posted") : r.moveTo).toHtmlEscaped());
        else if (r.after == PostRequest::After::Delete)
            lines << tr("Once posted, the files are deleted.");
    }
    return lines.join("<br>");
}

// ============================================================ navigation

void GuidedPostWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    _loadProfiles(_profileCombo->currentIndex() >= 2 ? _profileCombo->currentText() : QString());
    _refreshWhere();
    if (_step == STEP_SUMMARY)
        _refreshSummary();
    _updateButtons();
}

void GuidedPostWidget::_goTo(int step)
{
    _step = qBound(0, step, NB_STEPS - 1);
    if (_step == STEP_FILES)
        _showFilesPage();
    if (_step == STEP_OPTIONS)
    {
        _afterCard->setVisible(_req.usesAutoPosting());
        _updateOptionsEnabled();
    }
    if (_step == STEP_WHERE)
        _refreshWhere();
    if (_step == STEP_SUMMARY)
        _refreshSummary();
    _pages->setCurrentIndex(_step);
    _applyStepTexts();
    _updateButtons();
}

int GuidedPostWidget::_nextStep() const
{
    if (_step == STEP_OPTIONS && _ngPost->_guidedSkipWhere && whereIsReady())
        return STEP_SUMMARY;
    return _step + 1;
}

int GuidedPostWidget::_previousStep() const
{
    if (_step == STEP_SUMMARY && _ngPost->_guidedSkipWhere && whereIsReady())
        return STEP_OPTIONS;
    return _step - 1;
}

bool GuidedPostWidget::_canGoNext(QString *why) const
{
    using Kind = PostRequest::Kind;
    if (_step == STEP_FILES)
    {
        if (_req.kind == Kind::OneRelease || _req.kind == Kind::EachItem)
        {
            if (_fileList->count() == 0)
            {
                if (why) *why = tr("Add at least one file or folder.");
                return false;
            }
        }
        else
        {
            QFileInfo fi(_folderEdit->text().trimmed());
            if (!fi.exists() || !fi.isDir() || !fi.isReadable())
            {
                if (why) *why = tr("Choose a folder that exists.");
                return false;
            }
        }
    }
    else if (_step == STEP_OPTIONS)
    {
        if (_req.kind == Kind::WatchFolder && !_req.profile.compress)
        {
            if (why) *why = tr("A folder can only be watched with compression.");
            return false;
        }
    }
    return true;
}

void GuidedPostWidget::_updateButtons()
{
    const bool summary = _step == STEP_SUMMARY;
    QString why;
    const bool canNext = _canGoNext(&why);
    _backButton->setEnabled(_step > 0);
    _nextButton->setVisible(!summary);
    _nextButton->setEnabled(canNext);
    _nextButton->setToolTip(canNext ? QString() : why);
    _addAnother->setVisible(summary);
    _postButton->setVisible(summary);
    if (summary)
    {
        const bool blocked = _hasBlockingIssue();
        const bool validNow = _req.isValid();
        const int total = _ready.size() + (validNow ? 1 : 0);
        _addAnother->setEnabled(validNow && !blocked);
        _postButton->setEnabled(total > 0 && !blocked && (validNow || !_ready.isEmpty()));
        if (total > 1)
            _postButton->setText(tr("Post all (%1)").arg(total));
        else if (validNow && _req.kind == PostRequest::Kind::WatchFolder)
            _postButton->setText(tr("Start watching"));
        else
            _postButton->setText(tr("Post"));
        _editReady->setEnabled(_readyList->currentRow() >= 0);
        _removeReady->setEnabled(_readyList->currentRow() >= 0);
    }
    _reuseHint->setVisible(_step == STEP_WHAT && !_ready.isEmpty());
    if (!_ready.isEmpty())
        _reuseHint->setText(tr("%1 post(s) are already prepared: this is another one.").arg(_ready.size()));
}

void GuidedPostWidget::_onBack()
{
    if (_step > 0)
        _goTo(_previousStep());
}

void GuidedPostWidget::_onNext()
{
    if (!_canGoNext())
        return;
    _readStep(_step);
    _goTo(_nextStep());
}

void GuidedPostWidget::_onCancel()
{
    if (!_ready.isEmpty())
    {
        int res = QMessageBox::question(this, tr("Guided Post"),
                                        tr("Cancel the %1 post(s) already prepared?").arg(_ready.size()));
        if (res != QMessageBox::Yes)
            return;
    }
    _reset(true);
}

void GuidedPostWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        _onCancel();
        event->accept();
    }
    else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !qobject_cast<QPushButton*>(focusWidget()))
    {
        QPushButton *b = _step == STEP_SUMMARY ? _postButton : _nextButton;
        if (b->isEnabled())
            b->click();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

void GuidedPostWidget::_reset(bool keepChoices)
{
    _ready.clear();
    _editing = -1;
    PostRequest fresh = keepChoices && _ngPost->_guidedHasChoices ? _ngPost->_guidedChoices : PostRequest();
    fresh.paths.clear();
    fresh.folder.clear();
    _req = fresh;
    _fileList->clear();
    _folderEdit->clear();
    _saveProfileCB->setChecked(false);
    _profileName->clear();
    _fillFromRequest();
    _goTo(STEP_WHAT);
}

// ============================================================ request <-> widgets

void GuidedPostWidget::_fillFromRequest()
{
    _loading = true;
    _kindButtons[static_cast<int>(_req.kind)]->setChecked(true);
    _fileList->clear();
    for (const QString &p : _req.paths)
        _addPath(p);
    _folderEdit->setText(QDir::toNativeSeparators(_req.folder));
    _compressCB->setChecked(_req.profile.compress);
    _genNameCB->setChecked(_req.profile.genName);
    _genPassCB->setChecked(_req.profile.genPass);
    _par2CB->setChecked(_req.profile.par2);
    _par2Pct->setValue(static_cast<int>(_req.profile.par2Pct > 0 ? _req.profile.par2Pct : 10));
    _keepCB->setChecked(_req.profile.keepRar);
    _nfoCB->setChecked(_req.genNfo || _req.nfoInPost);
    _nfoInPostCB->setChecked(_req.nfoInPost);
    _nfoPassCB->setChecked(_req.nfoPassword);
    _nfoMediaCB->setChecked(_req.nfoMedia);
    _afterCombo->setCurrentIndex(static_cast<int>(_req.after));
    _moveToEdit->setText(_req.moveTo);
    _skipWhereCB->setChecked(_ngPost->_guidedSkipWhere);
    _loadProfiles();
    _loading = false;
    _updateOptionsEnabled();
    _updateFileSummary();
    _loadFolderPreview();
    _updateButtons();
}

void GuidedPostWidget::_readStep(int step)
{
    using Kind = PostRequest::Kind;
    if (step == STEP_FILES)
    {
        _req.paths.clear();
        for (int i = 0; i < _fileList->count(); ++i)
            _req.paths << _fileList->item(i)->data(Qt::UserRole).toString();
        _req.folder = _folderEdit->text().trimmed();
        if (_req.kind == Kind::ScanFolder || _req.kind == Kind::WatchFolder)
            _req.paths.clear();
    }
    else if (step == STEP_OPTIONS)
        _readOptions();
}

void GuidedPostWidget::_readOptions()
{
    _req.profile.compress = _compressCB->isChecked();
    _req.profile.genName  = _genNameCB->isChecked();
    _req.profile.genPass  = _genPassCB->isChecked();
    _req.profile.par2     = _par2CB->isChecked();
    _req.profile.par2Pct  = static_cast<uint>(_par2Pct->value());
    _req.profile.keepRar  = _keepCB->isChecked();
    _req.genNfo      = _nfoCB->isChecked();
    _req.nfoInPost   = _nfoCB->isChecked() && _nfoInPostCB->isChecked();
    _req.nfoPassword = _nfoPassCB->isChecked();
    _req.nfoMedia    = _nfoMediaCB->isChecked();
    _req.after       = static_cast<PostRequest::After>(qBound(0, _afterCombo->currentIndex(), 2));
    _req.moveTo      = _moveToEdit->text().trimmed();
    if (!_req.usesAutoPosting())
        _req.after = PostRequest::After::Nothing;
}

void GuidedPostWidget::_loadProfiles(const QString &select)
{
    QSignalBlocker block(_profileCombo);
    _profileCombo->clear();
    _profileCombo->addItem(tr("Custom"));
    _profileCombo->addItem(tr("Recommended (compression, random name and password, par2 10 %)"));
    for (const PostProfile &p : _ngPost->_profiles)
        _profileCombo->addItem(p.name);
    int idx = 0;
    if (!select.isEmpty())
        idx = _profileCombo->findText(select);
    else
    {
        for (int i = 0; i < _ngPost->_profiles.size(); ++i)
            if (_ngPost->_profiles.at(i).name == _req.profile.name && sameOptions(_ngPost->_profiles.at(i), _req.profile))
                idx = i + 2;
        if (idx == 0 && sameOptions(_req.profile, PostProfile::recommended()))
            idx = 1;
    }
    _profileCombo->setCurrentIndex(idx < 0 ? 0 : idx);
    _updateProfileDesc();
}

void GuidedPostWidget::_profileChosen(int index)
{
    if (index == 0)
        return; // Custom: the options are left as they are
    PostProfile p = index == 1 ? PostProfile::recommended() : _ngPost->_profiles.value(index - 2);
    _req.profile = p;
    _loading = true;
    _compressCB->setChecked(p.compress);
    _genNameCB->setChecked(p.genName);
    _genPassCB->setChecked(p.genPass);
    _par2CB->setChecked(p.par2);
    _par2Pct->setValue(static_cast<int>(p.par2Pct > 0 ? p.par2Pct : 10));
    _keepCB->setChecked(p.keepRar);
    _loading = false;
    _updateOptionsEnabled();
    _updateProfileDesc();
    _updateButtons();
}

void GuidedPostWidget::_optionsEdited()
{
    if (_loading)
        return;
    _readOptions();
    // the options are no longer those of the chosen profile
    int idx = _profileCombo->currentIndex();
    PostProfile chosen = idx == 1 ? PostProfile::recommended() : idx >= 2 ? _ngPost->_profiles.value(idx - 2) : PostProfile();
    if (idx > 0 && !sameOptions(chosen, _req.profile))
    {
        QSignalBlocker block(_profileCombo);
        _profileCombo->setCurrentIndex(0);
    }
    _updateOptionsEnabled();
    _updateProfileDesc();
    _updateButtons();
}

void GuidedPostWidget::_updateProfileDesc()
{
    PostProfile p = _req.profile;
    _profileDesc->setText(_describeProfile(p));
}

void GuidedPostWidget::_setCustomVisible(bool visible)
{
    _customCard->setVisible(visible);
    _customize->setText(visible ? tr("Hide the options") : tr("Customize"));
}

void GuidedPostWidget::_updateOptionsEnabled()
{
    const bool c = _compressCB->isChecked();
    _genNameCB->setEnabled(c);
    _genPassCB->setEnabled(c);
    _keepCB->setEnabled(c);
    _par2Pct->setEnabled(_par2CB->isChecked());
    const bool nfo = _nfoCB->isChecked();
    _nfoInPostCB->setEnabled(nfo);
    _nfoPassCB->setEnabled(nfo);
    _nfoMediaCB->setEnabled(nfo);
    _moveToEdit->setVisible(_afterCombo->currentIndex() == 1);
    if (!_customCard->isVisible() && !_customize->text().isEmpty() && _customCard->isHidden())
        ; // collapsed
}

// ============================================================ step 2

void GuidedPostWidget::_showFilesPage()
{
    using Kind = PostRequest::Kind;
    const bool files = _req.kind == Kind::OneRelease || _req.kind == Kind::EachItem;
    _filesStack->setCurrentIndex(files ? 0 : 1);
    _folderLabel->setText(_req.kind == Kind::WatchFolder ? tr("Folder to watch") : tr("Folder to scan"));
    _loadFolderPreview();
    _updateFileSummary();
}

void GuidedPostWidget::_addPath(const QString &path)
{
    QFileInfo fi(path);
    if (path.trimmed().isEmpty() || !fi.exists())
        return;
    const QString abs = fi.absoluteFilePath();
    for (int i = 0; i < _fileList->count(); ++i)
        if (_fileList->item(i)->data(Qt::UserRole).toString() == abs)
            return;
    QListWidgetItem *item = new QListWidgetItem(QIcon(fi.isDir() ? ":/icons/folder.png" : ":/icons/file.png"),
                                                QDir::toNativeSeparators(abs));
    item->setData(Qt::UserRole, abs);
    _fileList->addItem(item);
}

void GuidedPostWidget::_updateFileSummary()
{
    if (_fileList->count() == 0)
    {
        _fileSummary->setText(tr("Nothing added yet."));
        _updateButtons();
        return;
    }
    qint64 bytes = 0;
    for (int i = 0; i < _fileList->count(); ++i)
        bytes += NgPost::recursiveSize(QFileInfo(_fileList->item(i)->data(Qt::UserRole).toString()));
    QString text = tr("%1 item(s), %2").arg(_fileList->count()).arg(NfoWriter::humanSize(bytes));
    if (_req.profile.compress || _req.profile.par2)
        text += QString(" %1 ").arg(QChar(0x2192)) + tr("about %1 to upload").arg(
                    NfoWriter::humanSize(PostChecker::uploadBytes(bytes, _req.profile.par2, _req.profile.par2Pct)));
    _fileSummary->setText(text);
    _updateButtons();
}

void GuidedPostWidget::_loadFolderPreview()
{
    _folderPreview->clear();
    QFileInfo fi(_folderEdit->text().trimmed());
    if (_folderEdit->text().trimmed().isEmpty())
    {
        _folderSummary->setText(tr("Choose a folder."));
        return;
    }
    if (!fi.exists() || !fi.isDir())
    {
        _folderSummary->setText(tr("This folder does not exist."));
        return;
    }
    QDir dir(fi.absoluteFilePath());
    const QFileInfoList entries = dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name);
    int shown = 0, total = 0;
    for (const QFileInfo &e : entries)
    {
        if (_ngPost->isPostedFolder(e))
            continue;
        ++total;
        if (shown < 100)
        {
            _folderPreview->addItem(new QListWidgetItem(QIcon(e.isDir() ? ":/icons/folder.png" : ":/icons/file.png"), e.fileName()));
            ++shown;
        }
    }
    _folderSummary->setText(total == 0 ? tr("The folder is empty.") : tr("%1 item(s) in the folder.").arg(total));
}

QFileInfoList GuidedPostWidget::_requestFiles(const PostRequest &r, bool *hasFolder) const
{
    QFileInfoList files;
    if (r.kind == PostRequest::Kind::OneRelease || r.kind == PostRequest::Kind::EachItem)
    {
        for (const QString &p : r.paths)
        {
            QFileInfo fi(p);
            if (!p.trimmed().isEmpty() && fi.exists())
                files << fi;
        }
    }
    else if (r.kind == PostRequest::Kind::ScanFolder)
    {
        QDir dir(r.folder);
        for (const QFileInfo &e : dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name))
            if (!_ngPost->isPostedFolder(e))
                files << e;
    }
    if (hasFolder)
    {
        *hasFolder = false;
        for (const QFileInfo &fi : files)
            if (fi.isDir())
                *hasFolder = true;
    }
    return files;
}

void GuidedPostWidget::handleDropEvent(QDropEvent *e)
{
    using Kind = PostRequest::Kind;
    if (_step != STEP_FILES || !e->mimeData()->hasUrls())
        return;
    const bool files = _req.kind == Kind::OneRelease || _req.kind == Kind::EachItem;
    for (const QUrl &url : e->mimeData()->urls())
    {
        if (!url.isLocalFile())
            continue;
        if (files)
            _addPath(url.toLocalFile());
        else if (QFileInfo(url.toLocalFile()).isDir())
            _folderEdit->setText(QDir::toNativeSeparators(url.toLocalFile()));
    }
    _updateFileSummary();
}

// ============================================================ step 4

bool GuidedPostWidget::whereIsReady(QString *problem) const
{
    _hmi->updateServers();
    PostChecker::Input in = _hmi->checkInput(QFileInfoList(), false,
                                             _ngPost->_nzbPath.isEmpty() ? QString() : QDir(_ngPost->_nzbPath).filePath("ngpost.nzb"),
                                             false, QString(), true);
    for (const PostChecker::Issue &issue : PostChecker::check(in))
    {
        if (issue.code == PostChecker::Code::NoServer || issue.code == PostChecker::Code::NoGroup
                || issue.code == PostChecker::Code::NzbFolderNotWritable)
        {
            if (problem)
                *problem = issue.text;
            return false;
        }
    }
    int enabled = 0, failed = 0;
    for (const ServersWidget::Info &s : _hmi->serversWidget()->infos())
    {
        if (!s.enabled)
            continue;
        ++enabled;
        if (s.status == ServersWidget::Status::Failed)
            ++failed;
    }
    if (enabled > 0 && failed == enabled)
    {
        if (problem)
            *problem = tr("The last connection test failed on every enabled server: check them in Servers.");
        return false;
    }
    return true;
}

void GuidedPostWidget::_refreshWhere()
{
    QString servers;
    for (const ServersWidget::Info &s : _hmi->serversWidget()->infos())
    {
        QString color = !s.enabled ? "#8a8a8a" : s.status == ServersWidget::Status::Ok ? "#2e9e4f"
                      : s.status == ServersWidget::Status::Failed ? "#d13438" : "#8a8a8a";
        QString note = !s.enabled ? tr(" (off)") : s.status == ServersWidget::Status::Ok ? tr(" (the test succeeded)")
                     : s.status == ServersWidget::Status::Failed ? tr(" (the last test failed)") : QString();
        servers += QString("<span style='color:%1'>&#9679;</span> %2%3<br>").arg(color, s.host.toHtmlEscaped(), note.toHtmlEscaped());
    }
    _serversText->setText(servers.isEmpty() ? tr("No server yet.") : servers);
    const QStringList groups = _hmi->groupsList();
    _groupsText->setText(groups.isEmpty() ? tr("No newsgroup yet.") : groups.join(", ").toHtmlEscaped());
    _nzbText->setText(_ngPost->_nzbPath.isEmpty() ? tr("Not set: the NZB files are written in the default folder.")
                                                   : QDir::toNativeSeparators(_ngPost->_nzbPath).toHtmlEscaped());
    QString problem;
    const bool ready = whereIsReady(&problem);
    _whereState->setText(ready ? tr("Everything is ready.") : problem);
    _skipWhereCB->setEnabled(true);
}

// ============================================================ step 5

QList<PostChecker::Issue> GuidedPostWidget::_checksFor(const PostRequest &r) const
{
    bool hasFolder = false;
    const bool watching = r.kind == PostRequest::Kind::WatchFolder;
    QFileInfoList files = _requestFiles(r, &hasFolder);
    _hmi->updateServers();
    PostChecker::Input in = _hmi->checkInput(files, hasFolder,
                                             _ngPost->_nzbPath.isEmpty() ? QString() : QDir(_ngPost->_nzbPath).filePath("ngpost.nzb"),
                                             false, QString(), watching);
    in.compress = r.profile.compress;
    in.par2     = r.profile.par2;
    in.par2Pct  = r.profile.par2Pct;
    in.mediaInfoWanted = r.genNfo && r.nfoMedia;
    in.mediaInfoFound  = true;
    in.hasMediaFiles   = false;
    if (in.mediaInfoWanted)
    {
        in.mediaInfoFound = !MediaInfoReader::findProgram(_ngPost->_mediaInfoPath).isEmpty();
        in.hasMediaFiles  = watching;
        for (const QFileInfo &fi : files)
        {
            if (fi.isDir())
            {
                QDirIterator it(fi.absoluteFilePath(), QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                while (!in.hasMediaFiles && it.hasNext())
                    in.hasMediaFiles = MediaInfoReader::isMediaFile(it.next());
            }
            else
                in.hasMediaFiles = MediaInfoReader::isMediaFile(fi.fileName());
            if (in.hasMediaFiles)
                break;
        }
    }
    QList<PostChecker::Issue> issues = PostChecker::check(in);
    // the program (rar, par2) is chosen by Quick Post when the post starts: here it is only a warning
    for (PostChecker::Issue &issue : issues)
        if (issue.code == PostChecker::Code::RarMissing)
        {
            issue.blocking = false;
            issue.fixLabel.clear();
        }
    return issues;
}

bool GuidedPostWidget::_hasBlockingIssue() const
{
    for (const PostChecker::Issue &issue : _issues)
        if (issue.blocking)
            return true;
    return false;
}

void GuidedPostWidget::_showChecks(const PostRequest &r)
{
    while (QLayoutItem *item = _checksLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }
    _issues = r.isValid() ? _checksFor(r) : QList<PostChecker::Issue>();
    if (_issues.isEmpty())
    {
        QLabel *ok = newLabel("hint", _checksBox);
        ok->setText(r.isValid() ? tr("Everything is fine.") : tr("Nothing to check yet."));
        _checksLayout->addWidget(ok);
        return;
    }
    for (const PostChecker::Issue &issue : _issues)
    {
        QWidget *row = new QWidget(_checksBox);
        QHBoxLayout *h = new QHBoxLayout(row);
        h->setContentsMargins(0, 0, 0, 0);
        QLabel *text = newLabel("", row);
        text->setText(QString("%1 %2").arg(issue.blocking ? QString(QChar(0x26D4)) : QString(QChar(0x26A0)) + QChar(0xFE0F), issue.text.toHtmlEscaped()));
        h->addWidget(text, 1);
        if (!issue.fixLabel.isEmpty())
        {
            QPushButton *fix = new QPushButton(issue.fixLabel, row);
            fix->setCursor(Qt::PointingHandCursor);
            const PostChecker::Code code = issue.code;
            connect(fix, &QAbstractButton::clicked, this, [this, code](){ _applyFix(code); });
            h->addWidget(fix);
        }
        _checksLayout->addWidget(row);
    }
}

void GuidedPostWidget::_applyFix(PostChecker::Code code)
{
    using Code = PostChecker::Code;
    switch (code)
    {
    case Code::NoServer:
        _hmi->showServersPage();
        return;
    case Code::NoGroup:
        _hmi->showSettingsPage(MainWindow::CAT_POST);
        return;
    case Code::NzbFolderNotWritable:
        _hmi->showSettingsPage(MainWindow::CAT_NZB);
        return;
    case Code::FoldersNoCompress:
        _compressCB->setChecked(true);
        break;
    case Code::Par2Missing:
        _par2CB->setChecked(false);
        break;
    case Code::MediaInfoMissing:
        _nfoMediaCB->setChecked(false);
        break;
    default:
        break;
    }
    _readOptions();
    _refreshSummary();
}

void GuidedPostWidget::_refreshSummary()
{
    if (!_readyList)
        return;
    _readyList->clear();
    for (const PostRequest &r : _ready)
    {
        QString what = (r.kind == PostRequest::Kind::ScanFolder || r.kind == PostRequest::Kind::WatchFolder)
                ? QDir::toNativeSeparators(r.folder) : tr("%1 item(s)").arg(r.paths.size());
        _readyList->addItem(QString("%1: %2").arg(_kindName(r.kind), what));
    }
    const bool any = !_ready.isEmpty();
    _readyBox->setVisible(any);
    _thisText->setText(_req.isValid() ? _describe(_req) : tr("Nothing is being prepared: add another post or press Post."));
    _thisText->setTextFormat(Qt::RichText);
    _showChecks(_req);
    _updateButtons();
}

bool GuidedPostWidget::_commitCurrent()
{
    _readOptions();
    if (!_req.isValid())
        return false;
    _ready.append(_req);
    return true;
}

void GuidedPostWidget::_remember(const PostRequest &r)
{
    PostRequest keep = r;
    keep.paths.clear();
    keep.folder.clear();
    _ngPost->_guidedChoices    = keep;
    _ngPost->_guidedHasChoices = true;
    _hmi->scheduleAutoSave();
}

void GuidedPostWidget::_onAddAnother()
{
    if (_hasBlockingIssue() || !_commitCurrent())
        return;
    _remember(_ready.last());
    // the next post starts from the same choices, with other files
    PostRequest next = _ready.last();
    next.paths.clear();
    next.folder.clear();
    _req = next;
    _fileList->clear();
    _folderEdit->clear();
    _fillFromRequest();
    _goTo(STEP_WHAT);
}

void GuidedPostWidget::_onPost()
{
    // the post being prepared joins the ones that are ready
    if (_req.isValid())
    {
        if (_hasBlockingIssue() || !_commitCurrent())
            return;
        _req.paths.clear(); // it is in the list now
        _req.folder.clear();
    }
    if (_ready.isEmpty())
        return;

    bool deletes = false;
    for (const PostRequest &r : _ready)
        if (r.usesAutoPosting() && r.after == PostRequest::After::Delete)
            deletes = true;
    if (deletes)
    {
        int res = QMessageBox::warning(this, tr("Deleting files/folders once posted"),
                                       tr("You're about to delete files from your computer once they've been posted!\nUse it at your own risk!\nIt will be irreversible..."),
                                       QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (res != QMessageBox::Ok)
        {
            _refreshSummary();
            return;
        }
    }

    // a profile saved from these choices
    if (_saveProfileCB->isChecked())
    {
        QString name = PostProfile::cleanName(_profileName->text());
        if (name.isEmpty()
                || name.compare("Custom", Qt::CaseInsensitive) == 0 || name.compare(tr("Custom"), Qt::CaseInsensitive) == 0
                || name.compare("Recommended", Qt::CaseInsensitive) == 0 || name.startsWith("Recommended ", Qt::CaseInsensitive))
        {
            QMessageBox::information(this, tr("Save as profile"), tr("Choose a name for the profile (\"Custom\" and \"Recommended\" are reserved)."));
            _refreshSummary();
            return;
        }
        PostProfile p = _ready.last().profile;
        p.name = name;
        bool replaced = false;
        for (PostProfile &old : _ngPost->_profiles)
            if (old.name.compare(name, Qt::CaseInsensitive) == 0)
            {
                old = p;
                replaced = true;
            }
        if (!replaced)
            _ngPost->_profiles.append(p);
        for (PostRequest &r : _ready)
            if (sameOptions(r.profile, p))
                r.profile.name = name;
        emit _ngPost->profilesChanged();
    }

    const PostRequest last = _ready.last();
    int started = 0;
    const int total = _ready.size();
    while (!_ready.isEmpty())
    {
        if (!_hmi->startPost(_ready.first()))
            break; // the page that does the checks has told why: the post stays in the list
        _ready.removeFirst();
        ++started;
    }
    if (started > 0)
        _remember(last);
    if (_ready.isEmpty())
    {
        _hmi->log(tr("Guided Post: %1 post(s) started").arg(started));
        _reset(true);
    }
    else
    {
        _hmi->log(tr("Guided Post: %1 of %2 post(s) started, the others are still prepared in Guided Post").arg(started).arg(total));
        _req = PostRequest();
        _req.paths.clear();
        _goTo(STEP_SUMMARY);
    }
}
