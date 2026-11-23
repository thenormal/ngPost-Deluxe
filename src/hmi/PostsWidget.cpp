//========================================================================
//
// ngPost Deluxe: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
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

#include "PostsWidget.h"
#include "MainWindow.h"
#include "NgPost.h"
#include "PostingJob.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
QString humanBytes(double bytes)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int u = 0;
    while (bytes >= 1024. && u < 4)
    {
        bytes /= 1024.;
        ++u;
    }
    return QString("%1 %2").arg(bytes, 0, 'f', u == 0 ? 0 : 2).arg(units[u]);
}

//! progress bar drawn in the cells of the column Progress (colors of the state, readable in both themes)
class ProgressDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        opt.text.clear();
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); // background, selection

        int percent = index.data(PostsWidget::sPercentRole).toInt();
        auto state  = static_cast<PostEntry::State>(index.data(PostsWidget::sStateRole).toInt());
        bool dark   = option.palette.base().color().lightness() < 128;

        QColor fill;
        switch (state)
        {
        case PostEntry::State::DONE:             fill = dark ? QColor("#3fb46d") : QColor("#2e9d57"); break;
        case PostEntry::State::DONE_WITH_ERRORS: fill = dark ? QColor("#e0a23a") : QColor("#d08a12"); break;
        case PostEntry::State::STOPPED:
        case PostEntry::State::CANCELLED:        fill = dark ? QColor("#d9534f") : QColor("#c9302c"); break;
        case PostEntry::State::PAUSED:           fill = QColor("#8a8f98"); break;
        default:                                 fill = dark ? QColor("#4aa3ff") : QColor("#0b63ce"); break;
        }

        QRect r = option.rect.adjusted(6, option.rect.height() / 2 - 8, -6, -(option.rect.height() / 2 - 8));
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(dark ? QColor(255, 255, 255, 28) : QColor(0, 0, 0, 22));
        painter->drawRoundedRect(r, 4, 4);
        bool busy = state == PostEntry::State::COMPRESSING || state == PostEntry::State::PAR2;
        if (percent > 0 || busy)
        {
            QRect done = r;
            done.setWidth(busy ? r.width() : qMax(6, r.width() * percent / 100));
            QColor c = fill;
            if (busy)
                c.setAlpha(110); // packing: no percentage yet
            painter->setBrush(c);
            painter->drawRoundedRect(done, 4, 4);
        }
        QString text = busy ? PostsWidget::stateText(state) : QString("%1 %").arg(percent);
        painter->setPen(percent > 55 && !busy ? QColor(Qt::white) : option.palette.text().color());
        QFont f = option.font;
        f.setPointSizeF(f.pointSizeF() * 0.9);
        painter->setFont(f);
        painter->drawText(r, Qt::AlignCenter, text);
        painter->restore();
    }
};
} // namespace


PostsWidget::PostsWidget(NgPost *ngPost, MainWindow *hmi, QWidget *parent) :
    QWidget(parent), _ngPost(ngPost), _hmi(hmi), _list(ngPost->postList())
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 18, 24, 18);
    root->setSpacing(4);

    _title = new QLabel(this);
    _title->setObjectName("pageTitle");
    _subtitle = new QLabel(this);
    _subtitle->setObjectName("pageSubtitle");
    _subtitle->setWordWrap(true);
    root->addWidget(_title);
    root->addWidget(_subtitle);
    root->addSpacing(10);

    // tool bar: actions on the selected post, then on the whole list
    QHBoxLayout *bar = new QHBoxLayout();
    bar->setSpacing(8);
    _stopButton   = new QPushButton(QIcon(":/icons/stop.png"), QString(), this);
    _repostButton = new QPushButton(this);
    _folderButton = new QPushButton(QIcon(":/icons/folder.png"), QString(), this);
    _passButton   = new QPushButton(this);
    _removeButton = new QPushButton(QIcon(":/icons/clear.png"), QString(), this);
    _clearButton  = new QPushButton(QIcon(":/icons/clear.png"), QString(), this);
    bar->addWidget(_stopButton);
    bar->addWidget(_repostButton);
    bar->addWidget(_folderButton);
    bar->addWidget(_passButton);
    bar->addWidget(_removeButton);
    bar->addStretch();
    bar->addWidget(_clearButton);
    root->addLayout(bar);
    root->addSpacing(6);

    _summary = new QLabel(this);
    _summary->setObjectName("cardTitle");
    root->addWidget(_summary);
    root->addSpacing(4);

    _splitter = new QSplitter(Qt::Vertical, this);
    _splitter->setChildrenCollapsible(false);

    _table = new QTableWidget(0, NB_COLUMNS, _splitter);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setAlternatingRowColors(true);
    _table->setShowGrid(false);
    _table->setWordWrap(false);
    _table->verticalHeader()->hide();
    _table->verticalHeader()->setDefaultSectionSize(32);
    _table->horizontalHeader()->setHighlightSections(false);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _table->horizontalHeader()->setSectionResizeMode(COL_NAME, QHeaderView::Stretch);
    _table->setColumnWidth(COL_STATE, 150);
    _table->setColumnWidth(COL_PROGRESS, 170);
    _table->setColumnWidth(COL_SIZE, 95);
    _table->setColumnWidth(COL_SPEED, 100);
    _table->setColumnWidth(COL_ETA, 105);
    _table->setColumnWidth(COL_ORIGIN, 120);
    _table->setItemDelegateForColumn(COL_PROGRESS, new ProgressDelegate(_table));
    _table->setMinimumHeight(140);

    _emptyLbl = new QLabel(_table->viewport());
    _emptyLbl->setObjectName("hint");
    _emptyLbl->setAlignment(Qt::AlignCenter);
    _emptyLbl->setWordWrap(true);

    // details and log of the selected post
    QWidget *bottom = new QWidget(_splitter);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottom);
    bottomLayout->setContentsMargins(0, 8, 0, 0);
    bottomLayout->setSpacing(16);
    QWidget *detailsBox = new QWidget(bottom);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsBox);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    _detailsTitle = new QLabel(detailsBox);
    _detailsTitle->setObjectName("cardTitle");
    _details = new QLabel(detailsBox);
    _details->setWordWrap(true);
    _details->setTextFormat(Qt::RichText);
    _details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    _details->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    detailsLayout->addWidget(_detailsTitle);
    detailsLayout->addWidget(_details, 1);
    detailsBox->setMinimumWidth(260);
    detailsBox->setMaximumWidth(420);
    QWidget *logBox = new QWidget(bottom);
    QVBoxLayout *logLayout = new QVBoxLayout(logBox);
    logLayout->setContentsMargins(0, 0, 0, 0);
    _logTitle = new QLabel(logBox);
    _logTitle->setObjectName("cardTitle");
    _log = new QPlainTextEdit(logBox);
    _log->setReadOnly(true);
    _log->setMaximumBlockCount(PostList::sMaxLogLines + 1);
    logLayout->addWidget(_logTitle);
    logLayout->addWidget(_log, 1);
    bottomLayout->addWidget(detailsBox, 0);
    bottomLayout->addWidget(logBox, 1);

    _splitter->addWidget(_table);
    _splitter->addWidget(bottom);
    _splitter->setStretchFactor(0, 3);
    _splitter->setStretchFactor(1, 2);
    root->addWidget(_splitter, 1);

    connect(_list, &PostList::postAdded,   this, &PostsWidget::onPostAdded);
    connect(_list, &PostList::postChanged, this, &PostsWidget::onPostChanged);
    connect(_list, &PostList::postRemoved, this, &PostsWidget::onPostRemoved);
    connect(_list, &PostList::logAdded,    this, &PostsWidget::onLogAdded);
    connect(_table, &QTableWidget::itemSelectionChanged, this, &PostsWidget::onSelectionChanged);
    connect(_stopButton,   &QAbstractButton::clicked, this, &PostsWidget::onStop);
    connect(_repostButton, &QAbstractButton::clicked, this, &PostsWidget::onRepost);
    connect(_folderButton, &QAbstractButton::clicked, this, &PostsWidget::onOpenFolder);
    connect(_passButton,   &QAbstractButton::clicked, this, &PostsWidget::onCopyPassword);
    connect(_removeButton, &QAbstractButton::clicked, this, &PostsWidget::onRemove);
    connect(_clearButton,  &QAbstractButton::clicked, this, &PostsWidget::onRemoveFinished);
    connect(_table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onOpenFolder(); });

    for (const PostEntry &e : _list->entries())
        onPostAdded(e.id);
    retranslate();
}

QString PostsWidget::stateText(PostEntry::State state)
{
    switch (state)
    {
    case PostEntry::State::WAITING:          return tr("Waiting");
    case PostEntry::State::COMPRESSING:      return tr("Compressing");
    case PostEntry::State::PAR2:             return tr("par2");
    case PostEntry::State::UPLOADING:        return tr("Uploading");
    case PostEntry::State::PAUSED:           return tr("Paused");
    case PostEntry::State::DONE:             return tr("Done");
    case PostEntry::State::DONE_WITH_ERRORS: return tr("Done with errors");
    case PostEntry::State::STOPPED:          return tr("Stopped");
    case PostEntry::State::CANCELLED:        return tr("Cancelled");
    }
    return QString();
}

QString PostsWidget::originText(const QString &origin)
{
    if (origin == "quick")  return tr("Quick Post");
    if (origin == "choose") return tr("Auto Posting");
    if (origin == "scan")   return tr("Folder scan");
    if (origin == "watch")  return tr("Watched folder");
    if (origin == "queue")  return tr("Resumed");
    if (origin == "repost") return tr("Missing Articles");
    if (origin == "cli")    return tr("Command line");
    return origin;
}

void PostsWidget::retranslate()
{
    _title->setText(tr("Posts"));
    _subtitle->setText(tr("All the posts of this session: the ones waiting, the one being posted and the finished ones. "
                          "Select a post to see its details and its log."));
    _table->setHorizontalHeaderLabels({tr("Name"), tr("State"), tr("Progress"), tr("Size"),
                                       tr("Speed"), tr("Remaining"), tr("Started by")});
    _folderButton->setText(tr("Open NZB folder"));
    _passButton->setText(tr("Copy password"));
    _removeButton->setText(tr("Remove"));
    _removeButton->setToolTip(tr("Remove the selected post from the list (only a finished post)"));
    _clearButton->setText(tr("Remove finished"));
    _clearButton->setToolTip(tr("Remove all the finished posts from the list"));
    _emptyLbl->setText(tr("No post yet.\nPrepare one in Guided Post, Quick Post or Auto Posting: it will appear here."));
    _detailsTitle->setText(tr("Details"));
    _logTitle->setText(tr("Log of the post"));
    for (int row = 0; row < _table->rowCount(); ++row)
        if (const PostEntry *e = _list->entry(_table->item(row, COL_NAME)->data(sIdRole).toInt()))
            _fillRow(row, *e);
    _showDetails();
    _updateButtons();
    _updateEmpty();
}

int PostsWidget::_rowOf(int id) const
{
    for (int row = 0; row < _table->rowCount(); ++row)
        if (_table->item(row, COL_NAME)->data(sIdRole).toInt() == id)
            return row;
    return -1;
}

int PostsWidget::selectedId() const
{
    QList<QTableWidgetItem*> items = _table->selectedItems();
    for (QTableWidgetItem *item : items)
        if (item->column() == COL_NAME)
            return item->data(sIdRole).toInt();
    return -1;
}

void PostsWidget::selectPost(int id)
{
    int row = _rowOf(id);
    if (row < 0)
        return;
    _table->selectRow(row);
    _table->scrollToItem(_table->item(row, COL_NAME));
}

void PostsWidget::onPostAdded(int id)
{
    const PostEntry *e = _list->entry(id);
    if (!e || _rowOf(id) >= 0)
        return;
    int row = _list->indexOf(id); // same order as the list (the order of the posts)
    if (row < 0 || row > _table->rowCount())
        row = _table->rowCount();
    _table->insertRow(row);
    for (int col = 0; col < NB_COLUMNS; ++col)
        _table->setItem(row, col, new QTableWidgetItem());
    _table->item(row, COL_NAME)->setData(sIdRole, id);
    _fillRow(row, *e);
    _updateEmpty();
}

void PostsWidget::onPostChanged(int id)
{
    int row = _rowOf(id);
    const PostEntry *e = _list->entry(id);
    if (row < 0 || !e)
        return;
    _fillRow(row, *e);
    if (id == selectedId())
    {
        _showDetails();
        _updateButtons();
    }
    _updateEmpty(); // the summary
}

void PostsWidget::onPostRemoved(int id)
{
    int row = _rowOf(id);
    if (row >= 0)
        _table->removeRow(row);
    if (_logOf == id)
        _logOf = -1;
    onSelectionChanged();
    _updateEmpty();
}

void PostsWidget::_fillRow(int row, const PostEntry &e)
{
    QTableWidgetItem *name = _table->item(row, COL_NAME);
    QString shown = e.name;
    if (shown.endsWith(".nzb", Qt::CaseInsensitive))
        shown.chop(4);
    name->setText(shown);
    name->setToolTip(e.nzbPath.isEmpty() ? e.name : QDir::toNativeSeparators(e.nzbPath));
    QString icon = e.state == PostEntry::State::DONE ? ":/icons/ok.png"
                 : e.state == PostEntry::State::DONE_WITH_ERRORS || e.state == PostEntry::State::STOPPED
                   || e.state == PostEntry::State::CANCELLED ? ":/icons/ko.png"
                 : e.state == PostEntry::State::WAITING ? ":/icons/pending.png" : ":/icons/uploading.png";
    name->setIcon(QIcon(icon));

    QString state = stateText(e.state);
    if (e.state == PostEntry::State::DONE_WITH_ERRORS && e.nbFailed)
        state = tr("Done, %1 failed").arg(e.nbFailed);
    _table->item(row, COL_STATE)->setText(state);

    QTableWidgetItem *progress = _table->item(row, COL_PROGRESS);
    progress->setData(sPercentRole, e.progressPercent());
    progress->setData(sStateRole, static_cast<int>(e.state));
    progress->setToolTip(e.nbArticles ? tr("%1 / %2 Articles").arg(e.nbUploaded).arg(e.nbArticles) : QString());

    _table->item(row, COL_SIZE)->setText(e.sizeTotal ? humanBytes(static_cast<double>(e.sizeTotal)) : QString());
    _table->item(row, COL_SIZE)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    bool showSpeed = e.speed > 0. && (e.isActive() || e.isFinished());
    _table->item(row, COL_SPEED)->setText(showSpeed ? QString("%1/s").arg(humanBytes(e.speed)) : QString());
    _table->item(row, COL_SPEED)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QString eta;
    if (e.state == PostEntry::State::UPLOADING && e.etaSec >= 0)
        eta = PostingJob::humanDuration(e.etaSec * 1000);
    else if (e.isFinished() && e.durationMs > 0)
        eta = tr("took %1").arg(PostingJob::humanDuration(e.durationMs));
    _table->item(row, COL_ETA)->setText(eta);
    _table->item(row, COL_ORIGIN)->setText(originText(e.origin));
}

void PostsWidget::_updateEmpty()
{
    bool empty = _table->rowCount() == 0;
    _emptyLbl->setVisible(empty);
    if (empty)
        _emptyLbl->setGeometry(_table->viewport()->rect().adjusted(20, 20, -20, -20));

    int waiting = 0, running = 0, done = 0, errors = 0;
    for (const PostEntry &e : _list->entries())
    {
        if (e.state == PostEntry::State::WAITING)
            ++waiting;
        else if (e.isActive())
            ++running;
        else if (e.state == PostEntry::State::DONE)
            ++done;
        else
            ++errors;
    }
    const QString sep = QString::fromUtf8("  ·  ");
    _summary->setText(QStringList({tr("%1 post(s)").arg(_list->count()), tr("%1 running").arg(running),
                                   tr("%1 waiting").arg(waiting), tr("%1 done").arg(done),
                                   tr("%1 with problems").arg(errors)}).join(sep));
    _clearButton->setEnabled(done + errors > 0);
}

void PostsWidget::onSelectionChanged()
{
    _showDetails();
    _showLog();
    _updateButtons();
}

void PostsWidget::_showDetails()
{
    const PostEntry *e = _list->entry(selectedId());
    if (!e)
    {
        _details->setText(QString("<i>%1</i>").arg(tr("Select a post in the list.").toHtmlEscaped()));
        return;
    }
    auto row = [](const QString &label, const QString &value) {
        return QString("<tr><td style='padding-right:10px;'><b>%1</b></td><td>%2</td></tr>").arg(
                    label.toHtmlEscaped(), value.toHtmlEscaped());
    };
    QString html = "<table cellspacing='0' cellpadding='2'>";
    html += row(tr("State"), stateText(e->state));
    if (!e->nzbPath.isEmpty())
        html += row(tr("NZB file"), QDir::toNativeSeparators(e->nzbPath));
    html += row(tr("Archive"), e->archiveName.isEmpty() ? tr("no compression") : e->archiveName);
    html += row(tr("Password"), e->password.isEmpty() ? tr("none") : QString(8, QChar(0x2022)));
    if (!e->groups.isEmpty())
        html += row(tr("Newsgroups"), e->groups);
    if (!e->files.isEmpty())
    {
        QStringList names;
        for (int i = 0; i < e->files.size() && i < 5; ++i)
            names << QFileInfo(e->files.at(i)).fileName();
        if (e->files.size() > 5)
            names << tr("and %1 more").arg(e->files.size() - 5);
        html += row(tr("Files"), names.join(", "));
    }
    if (e->nbArticles)
        html += row(tr("Articles"), e->nbFailed ? tr("%1 / %2, %3 failed").arg(e->nbUploaded).arg(e->nbArticles).arg(e->nbFailed)
                                                : tr("%1 / %2").arg(e->nbUploaded).arg(e->nbArticles));
    int missing = _ngPost->nbMissingArticles(e->id);
    if (missing)
        html += row(tr("To post again"), tr("%1 Article(s)").arg(missing));
    html += row(tr("Added"), e->added.toString("yyyy-MM-dd HH:mm:ss"));
    if (e->started.isValid())
        html += row(tr("Started"), e->started.toString("HH:mm:ss"));
    if (e->finished.isValid())
        html += row(tr("Finished"), e->finished.toString("HH:mm:ss"));
    html += "</table>";
    _details->setText(html);
}

QString PostsWidget::_logHtml(const PostEntry::LogLine &line) const
{
    bool dark = _log->palette().base().color().lightness() < 128;
    QString color;
    bool bold = false;
    switch (line.level)
    {
    case PostEntry::LOG_PHASE: color = dark ? "#6cb0ff" : "#1a5fb4"; bold = true; break;
    case PostEntry::LOG_OK:    color = dark ? "#5fd38a" : "#1e7e34"; break;
    case PostEntry::LOG_WARN:  color = dark ? "#ffb454" : "#b45f06"; break;
    case PostEntry::LOG_DEBUG: color = dark ? "#9a9a9a" : "#7a7a7a"; break;
    case PostEntry::LOG_ERROR: color = dark ? "#ff6b6b" : "#c62828"; break;
    default: break;
    }
    QString text = line.text;
    bool isHtml = line.level == PostEntry::LOG_INFO && text.contains(QRegularExpression("<(h\\d|b|br|font|span|i)[ >/]"));
    if (isHtml && text.contains(QRegularExpression("<h\\d>"))) // titles: bold, without the big margins of <h3>
        text = text.replace(QRegularExpression("</?h\\d>"), QString()).prepend("<b>").append("</b>");
    if (!isHtml)
        text = text.toHtmlEscaped();
    QString time = QString("<span style='color:%1;'>%2</span> ").arg(dark ? "#7d8590" : "#8a8f98", line.time.toString("HH:mm:ss"));
    if (color.isEmpty())
        return time + text;
    return time + QString("<span style='color:%1;%2'>%3</span>").arg(color, bold ? "font-weight:bold;" : "", text);
}

void PostsWidget::_showLog()
{
    int id = selectedId();
    if (id == _logOf && id >= 0)
        return;
    _logOf = id;
    _log->clear();
    const PostEntry *e = _list->entry(id);
    if (!e)
        return;
    if (e->logDropped)
        _log->appendHtml(QString("<i>%1</i>").arg(tr("(%1 older line(s) not kept)").arg(e->logDropped).toHtmlEscaped()));
    for (const PostEntry::LogLine &line : e->log)
        _log->appendHtml(_logHtml(line));
    _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
}

void PostsWidget::onLogAdded(int id, const PostEntry::LogLine &line, bool continuesLine)
{
    if (id != _logOf)
        return;
    QScrollBar *sb = _log->verticalScrollBar();
    bool atEnd = sb->value() >= sb->maximum() - 4;
    if (continuesLine)
    {   // the line goes on (progress stars...): it is written again
        QTextCursor cursor(_log->document()->lastBlock());
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        if (_log->document()->blockCount() == 1 && _log->document()->firstBlock().text().isEmpty())
            _log->clear();
    }
    _log->appendHtml(_logHtml(line));
    if (atEnd)
        sb->setValue(sb->maximum());
}

void PostsWidget::_updateButtons()
{
    const PostEntry *e = _list->entry(selectedId());
    bool finished = e && e->isFinished();
    _stopButton->setEnabled(e && !finished);
    _stopButton->setText(e && e->state == PostEntry::State::WAITING ? tr("Cancel") : tr("Stop"));
    _stopButton->setToolTip(e && e->state == PostEntry::State::WAITING ? tr("Remove this post from the queue before it starts")
                                                                         : tr("Stop this post"));
    int missing = e ? _ngPost->nbMissingArticles(e->id) : 0;
    _repostButton->setVisible(missing > 0);
    _repostButton->setText(tr("Post the missing Articles again (%1)").arg(missing));
    _folderButton->setEnabled(e && !e->nzbPath.isEmpty());
    _passButton->setEnabled(e && !e->password.isEmpty());
    _removeButton->setEnabled(finished);
}

void PostsWidget::onStop()
{
    const PostEntry *e = _list->entry(selectedId());
    if (!e || e->isFinished())
        return;
    int id = e->id;
    if (e->state != PostEntry::State::WAITING
            && QMessageBox::question(this, tr("Stop the post?"),
                                     tr("Stop the post %1?\nWhat was already uploaded stays on the server, the rest is not posted.").arg(e->name),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    _ngPost->stopPost(id);
}

void PostsWidget::onRepost()
{
    int id = selectedId();
    if (id >= 0 && _ngPost->repostMissing(id))
    {
        _showDetails();
        _updateButtons();
    }
}

void PostsWidget::onRemove()
{
    const PostEntry *e = _list->entry(selectedId());
    if (!e || !e->isFinished())
        return;
    int id = e->id;
    int missing = _ngPost->nbMissingArticles(id);
    if (missing > 0 && QMessageBox::question(this, tr("Articles are missing"),
                                             tr("%1 Article(s) of this post couldn't be posted and the nzb file points to them.\n"
                                                "If you remove it from the list you won't be able to post them again.\n\n"
                                                "Remove it anyway?").arg(missing),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    _ngPost->forgetMissing(id);
    _list->remove(id);
}

void PostsWidget::onRemoveFinished()
{
    int missing = 0;
    for (const PostEntry &e : _list->entries())
        if (e.isFinished())
            missing += _ngPost->nbMissingArticles(e.id);
    if (missing > 0)
    {
        QMessageBox box(QMessageBox::Question, tr("Articles are missing"),
                        tr("Some finished posts have %1 Article(s) that can still be posted again.\n"
                           "Keep them in the list?").arg(missing), QMessageBox::NoButton, this);
        QPushButton *keep = box.addButton(tr("Keep them"), QMessageBox::AcceptRole);
        box.addButton(tr("Remove them too"), QMessageBox::DestructiveRole);
        QPushButton *cancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);
        box.setDefaultButton(keep);
        box.exec();
        if (box.clickedButton() == cancel)
            return;
        bool keepMissing = box.clickedButton() == keep;
        QList<int> ids;
        for (const PostEntry &e : _list->entries())
            if (e.isFinished() && (!keepMissing || _ngPost->nbMissingArticles(e.id) == 0))
                ids << e.id;
        for (int id : ids)
        {
            _ngPost->forgetMissing(id);
            _list->remove(id);
        }
        return;
    }
    _list->removeFinished();
}

void PostsWidget::onOpenFolder()
{
    const PostEntry *e = _list->entry(selectedId());
    if (!e || e->nzbPath.isEmpty())
        return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(e->nzbPath).absolutePath()));
}

void PostsWidget::onCopyPassword()
{
    const PostEntry *e = _list->entry(selectedId());
    if (e && !e->password.isEmpty())
        QApplication::clipboard()->setText(e->password);
}
