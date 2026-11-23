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
#include "utils/QueueOrder.h"
#include "StatsWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QLineEdit>
#include <QDesktopServices>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMenu>
#include <QPainterPath>
#include <functional>
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
//! the table of the page Posts: a waiting post can be dragged to another place of the queue
//! (the rows are not moved by the table itself: the queue changes and the list gives the new order)
class PostsTable : public QTableWidget
{
public:
    std::function<bool(int row)>              canDrag;
    std::function<void(int row, int before)>  dropped; //!< before: row the post goes before (rowCount(): at the end)

    PostsTable(int rows, int columns, QWidget *parent) : QTableWidget(rows, columns, parent)
    {
        setDragEnabled(true);
        setAcceptDrops(true);
        viewport()->setAcceptDrops(true);
        setDragDropMode(QAbstractItemView::InternalMove);
        setDefaultDropAction(Qt::MoveAction);
        setDropIndicatorShown(true);
        setDragDropOverwriteMode(false);
    }

protected:
    void startDrag(Qt::DropActions supportedActions) override
    {
        if (canDrag && canDrag(currentRow()))
            QTableWidget::startDrag(supportedActions);
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        QTableWidget::dragMoveEvent(event); // auto scroll and drop indicator
        if (event->source() != this)
            event->ignore();
        else
            event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override
    {
        if (event->source() != this)
        {
            event->ignore();
            return;
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPoint pos = event->position().toPoint();
#else
        QPoint pos = event->pos();
#endif
        QModelIndex index = indexAt(pos);
        int before = rowCount();
        if (index.isValid())
        {
            QRect rect = visualRect(index);
            before = pos.y() < rect.center().y() ? index.row() : index.row() + 1;
        }
        int row = currentRow();
        // nothing is moved by the table (the default drop would move the cells)
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
        setState(QAbstractItemView::NoState);
        viewport()->update();
        if (dropped && row >= 0)
            dropped(row, before);
    }
};

//! black icon of the resources in the color of the text (readable in the dark theme)
QIcon tintedIcon(const QString &path, const QColor &color)
{
    QPixmap src(path);
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.drawPixmap(0, 0, src);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pix.rect(), color);
    p.end();
    return QIcon(pix);
}

//! arrow of the move buttons (bar: to the top or the bottom), drawn in the color of the text
QIcon arrowIcon(bool up, bool bar, const QColor &color)
{
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    QPainterPath tri;
    if (up)
    {
        tri.moveTo(16, bar ? 10 : 7);
        tri.lineTo(27, bar ? 24 : 22);
        tri.lineTo(5, bar ? 24 : 22);
    }
    else
    {
        tri.moveTo(16, bar ? 22 : 25);
        tri.lineTo(27, bar ? 8 : 10);
        tri.lineTo(5, bar ? 8 : 10);
    }
    tri.closeSubpath();
    p.drawPath(tri);
    if (bar)
        p.drawRoundedRect(QRectF(5, up ? 3 : 26, 22, 3.5), 1.5, 1.5);
    p.end();
    return QIcon(pix);
}
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
    _pauseButton  = new QPushButton(this);
    _priorityButton = new QPushButton(this);
    _priorityMenu = new QMenu(this);
    for (int prio : {QueueOrder::HIGH, QueueOrder::NORMAL, QueueOrder::LOW})
    {
        QAction *act = _priorityMenu->addAction(QString());
        act->setData(prio);
        act->setCheckable(true);
        connect(act, &QAction::triggered, this, [this, prio](){ _setPriority(prio); });
    }
    _priorityButton->setMenu(_priorityMenu);
    _topButton    = new QPushButton(this);
    _upButton     = new QPushButton(this);
    _downButton   = new QPushButton(this);
    _bottomButton = new QPushButton(this);
    for (QPushButton *b : {_topButton, _upButton, _downButton, _bottomButton})
    {
        b->setIconSize(QSize(16, 16));
        b->setFixedWidth(34);
    }
    _pauseButton->setMinimumWidth(100);
    _stopButton->setMinimumWidth(90);
    _repostButton = new QPushButton(this);
    _folderButton = new QPushButton(QIcon(":/icons/folder.png"), QString(), this);
    _passButton   = new QPushButton(this);
    _removeButton = new QPushButton(QIcon(":/icons/clear.png"), QString(), this);
    _clearButton  = new QPushButton(QIcon(":/icons/clear.png"), QString(), this);
    bar->addWidget(_pauseButton);
    bar->addWidget(_stopButton);
    bar->addSpacing(8);
    bar->addWidget(_priorityButton);
    QHBoxLayout *moves = new QHBoxLayout();
    moves->setSpacing(2);
    for (QPushButton *b : {_topButton, _upButton, _downButton, _bottomButton})
        moves->addWidget(b);
    bar->addLayout(moves);
    bar->addSpacing(8);
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

    PostsTable *table = new PostsTable(0, NB_COLUMNS, _splitter);
    table->canDrag = [this](int row){ return _canDrag(_rowId(row)); };
    table->dropped = [this](int row, int before){ _dropped(_rowId(row), before); };
    _table = table;
    _table->setContextMenuPolicy(Qt::CustomContextMenu);
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
    _table->setColumnWidth(COL_PRIORITY, 80);
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
    _speedTitle = new QLabel(detailsBox);
    _speedTitle->setObjectName("cardTitle");
    _speedChart = new ChartWidget(ChartWidget::Type::Line, ngPost, detailsBox);
    _speedChart->setBinaryScale(true);
    _speedChart->setMinimumHeight(130);
    _speedChart->setMaximumHeight(160);
    _speedInfo = new QLabel(detailsBox);
    _speedInfo->setWordWrap(true);
    detailsLayout->addWidget(_detailsTitle);
    detailsLayout->addWidget(_details, 1);
    detailsLayout->addWidget(_speedTitle);
    detailsLayout->addWidget(_speedChart);
    detailsLayout->addWidget(_speedInfo);
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
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(8);
    _logSearch = new QLineEdit(logBox);
    _logSearch->setClearButtonEnabled(true);
    _logSearch->setMinimumWidth(140);
    filterLayout->addWidget(_logSearch, 1);
    for (int i = 0; i < 6; ++i)
    {
        _logLevels[i] = new QCheckBox(logBox);
        _logLevels[i]->setChecked(i != PostEntry::LOG_DEBUG); // the debug lines are hidden by default
        filterLayout->addWidget(_logLevels[i]);
        connect(_logLevels[i], &QCheckBox::toggled, this, [this](){ _refreshLog(); });
    }
    _logCopy = new QPushButton(logBox);
    filterLayout->addWidget(_logCopy);
    _logCount = new QLabel(logBox);
    connect(_logSearch, &QLineEdit::textChanged, this, [this](){ _refreshLog(); });
    connect(_logCopy, &QAbstractButton::clicked, this, [this](){ QApplication::clipboard()->setText(_log->toPlainText()); });
    logLayout->addWidget(_logTitle);
    logLayout->addLayout(filterLayout);
    logLayout->addWidget(_log, 1);
    logLayout->addWidget(_logCount);
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
    connect(_list, &PostList::orderChanged, this, &PostsWidget::onOrderChanged);
    connect(_table, &QWidget::customContextMenuRequested, this, &PostsWidget::onContextMenu);
    connect(_pauseButton,  &QAbstractButton::clicked, this, &PostsWidget::onPauseResume);
    connect(_topButton,    &QAbstractButton::clicked, this, [this](){ _move(static_cast<int>(QueueOrder::Move::TOP)); });
    connect(_upButton,     &QAbstractButton::clicked, this, [this](){ _move(static_cast<int>(QueueOrder::Move::UP)); });
    connect(_downButton,   &QAbstractButton::clicked, this, [this](){ _move(static_cast<int>(QueueOrder::Move::DOWN)); });
    connect(_bottomButton, &QAbstractButton::clicked, this, [this](){ _move(static_cast<int>(QueueOrder::Move::BOTTOM)); });
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
    _makeIcons();
    retranslate();
}

void PostsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange)
    {   // light or dark theme
        _makeIcons();
        for (int row = 0; row < _table->rowCount(); ++row)
            if (const PostEntry *e = _list->entry(_rowId(row)))
                _fillRow(row, *e);
        _updateButtons();
    }
    QWidget::changeEvent(event);
}

void PostsWidget::_makeIcons()
{
    QColor color = palette().color(QPalette::ButtonText);
    _topButton->setIcon(arrowIcon(true, true, color));
    _upButton->setIcon(arrowIcon(true, false, color));
    _downButton->setIcon(arrowIcon(false, false, color));
    _bottomButton->setIcon(arrowIcon(false, true, color));
}

QString PostsWidget::priorityText(int priority)
{
    switch (QueueOrder::clampPriority(priority))
    {
    case QueueOrder::HIGH: return tr("High");
    case QueueOrder::LOW:  return tr("Low");
    default:               return tr("Normal");
    }
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
    _table->setHorizontalHeaderLabels({tr("Name"), tr("State"), tr("Priority"), tr("Progress"), tr("Size"),
                                       tr("Speed"), tr("Remaining"), tr("Started by")});
    _priorityButton->setText(tr("Priority"));
    _priorityButton->setToolTip(tr("Priority of the selected waiting post: the posts with a higher priority start first"));
    for (QAction *act : _priorityMenu->actions())
        act->setText(priorityText(act->data().toInt()));
    _topButton->setToolTip(tr("Move to the top of the queue"));
    _upButton->setToolTip(tr("Move up"));
    _downButton->setToolTip(tr("Move down"));
    _bottomButton->setToolTip(tr("Move to the bottom of the queue"));
    _folderButton->setText(tr("Open NZB folder"));
    _passButton->setText(tr("Copy password"));
    _removeButton->setText(tr("Remove"));
    _removeButton->setToolTip(tr("Remove the selected post from the list (only a finished post)"));
    _clearButton->setText(tr("Remove finished"));
    _clearButton->setToolTip(tr("Remove all the finished posts from the list"));
    _emptyLbl->setText(tr("No post yet.\nPrepare one in Guided Post, Quick Post or Auto Posting: it will appear here."));
    _detailsTitle->setText(tr("Details"));
    _logTitle->setText(tr("Log of the post"));
    _speedTitle->setText(tr("Upload speed"));
    _logSearch->setPlaceholderText(tr("Search in the log"));
    // order of PostEntry::LogLevel: info, phase, ok, warn, debug, error
    const QString names[6] = {tr("Info"), tr("Phases"), tr("OK"), tr("Warnings"), tr("Debug"), tr("Errors")};
    for (int i = 0; i < 6; ++i)
        _logLevels[i]->setText(names[i]);
    _logCopy->setText(tr("Copy"));
    _logCopy->setToolTip(tr("Copy the lines shown in the log"));
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
                 : e.paused ? QString()
                 : e.state == PostEntry::State::WAITING ? ":/icons/pending.png" : ":/icons/uploading.png";
    name->setIcon(icon.isEmpty() ? tintedIcon(":/icons/nav_pause.png", _table->palette().color(QPalette::Text))
                                 : QIcon(icon));

    QString state = stateText(e.state);
    QString stateTip;
    if (e.state == PostEntry::State::DONE_WITH_ERRORS && e.nbFailed)
        state = tr("Done, %1 failed").arg(e.nbFailed);
    else if (e.state == PostEntry::State::WAITING && e.paused)
    {
        state = tr("Paused");
        stateTip = tr("The next posts go before it. Resume it to put it back in the queue.");
    }
    else if (e.state == PostEntry::State::WAITING && e.prepared == 1)
    {
        state = tr("Preparing (%1)").arg(e.queuePos + 1);
        stateTip = tr("Compressed now, while the current post is uploading: it starts next");
    }
    else if (e.state == PostEntry::State::WAITING && e.prepared == 2)
    {
        state = tr("Ready (%1)").arg(e.queuePos + 1);
        stateTip = tr("Already compressed: it starts next");
    }
    else if (e.state == PostEntry::State::WAITING && e.queuePos >= 0)
        state = tr("Waiting (%1)").arg(e.queuePos + 1);
    else if ((e.state == PostEntry::State::COMPRESSING || e.state == PostEntry::State::PAR2) && e.paused)
    {
        state = tr("%1, then paused").arg(stateText(e.state));
        stateTip = tr("It pauses when the upload starts");
    }
    _table->item(row, COL_STATE)->setText(state);
    _table->item(row, COL_STATE)->setToolTip(stateTip);
    bool waiting = e.queuePos >= 0;
    _table->item(row, COL_PRIORITY)->setText(waiting || e.priority != QueueOrder::NORMAL ? priorityText(e.priority) : QString());

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
    else if (e.isFinished() && e.state != PostEntry::State::CANCELLED && e.durationMs > 0)
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

    int waiting = 0, running = 0, done = 0, errors = 0, paused = 0;
    for (const PostEntry &e : _list->entries())
    {
        if (e.paused && !e.isFinished())
            ++paused;
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
    QStringList parts({tr("%1 post(s)").arg(_list->count()), tr("%1 running").arg(running),
                       tr("%1 waiting").arg(waiting)});
    if (paused)
        parts << tr("%1 paused").arg(paused);
    parts << tr("%1 done").arg(done) << tr("%1 with problems").arg(errors);
    _summary->setText(parts.join(sep));
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
    html += row(tr("State"), e->paused && !e->isFinished() ? tr("%1 (paused)").arg(stateText(e->state)) : stateText(e->state));
    if (!e->isFinished())
        html += row(tr("Priority"), priorityText(e->priority));
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
    _showSpeed();
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

bool PostsWidget::_logVisible(const PostEntry::LogLine &line) const
{
    int level = qBound(0, line.level, 5);
    if (!_logLevels[level]->isChecked())
        return false;
    const QString search = _logSearch->text().trimmed();
    if (search.isEmpty())
        return true;
    QString text = line.text;
    if (line.level == PostEntry::LOG_INFO)
        text.remove(QRegularExpression("<[^>]*>"));
    return text.contains(search, Qt::CaseInsensitive);
}

bool PostsWidget::_logFiltered() const
{
    if (!_logSearch->text().trimmed().isEmpty())
        return true;
    for (int i = 0; i < 6; ++i)
        if (!_logLevels[i]->isChecked())
            return true;
    return false;
}

void PostsWidget::_refreshLog()
{
    _logOf = -2; // forces _showLog
    _showLog();
}

void PostsWidget::_showLog()
{
    int id = selectedId();
    if (id == _logOf && id >= 0)
        return;
    _logOf = id;
    _log->clear();
    _logCount->clear();
    const PostEntry *e = _list->entry(id);
    if (!e)
        return;
    if (e->logDropped)
        _log->appendHtml(QString("<i>%1</i>").arg(tr("(%1 older line(s) not kept)").arg(e->logDropped).toHtmlEscaped()));
    int shown = 0;
    for (const PostEntry::LogLine &line : e->log)
        if (_logVisible(line))
        {
            _log->appendHtml(_logHtml(line));
            ++shown;
        }
    _logCount->setText(_logFiltered() ? tr("%1 of %2 lines shown").arg(shown).arg(e->log.size()) : QString());
    _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
}

void PostsWidget::onLogAdded(int id, const PostEntry::LogLine &line, bool continuesLine)
{
    if (id != _logOf)
        return;
    if (_logFiltered())
    {
        if (continuesLine && !_logSearch->text().trimmed().isEmpty())
        {   // the text of the line changed: it may start or stop matching
            _refreshLog();
            return;
        }
        if (!_logVisible(line))
            return;
        if (const PostEntry *e = _list->entry(id))
            _logCount->setText(tr("%1 of %2 lines shown").arg(_log->document()->blockCount()).arg(e->log.size()));
    }
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

void PostsWidget::_showSpeed()
{
    const PostEntry *e = _list->entry(selectedId());
    const QVector<float> *samples = e ? &e->speedSamples : nullptr;
    if (!samples || samples->size() < 2)
    {
        _speedChart->hide();
        _speedInfo->setText(e && e->isActive() ? tr("The graph starts when the upload starts.")
                                               : tr("No graph for this post (only the posts uploaded since the program started have one)."));
        return;
    }
    _speedChart->show();
    // at most 60 points: the samples are averaged in groups
    const int n = samples->size();
    const int group = qMax(1, (n + 59) / 60);
    const int stepSec = e->sampleStepSec * group;
    QStringList labels;
    QVector<double> values;
    double sum = 0, best = 0;
    for (float v : *samples)
    {
        sum += v;
        best = qMax(best, static_cast<double>(v));
    }
    for (int i = 0; i < n; i += group)
    {
        double g = 0;
        int count = 0;
        for (int j = i; j < qMin(n, i + group); ++j, ++count)
            g += samples->at(j);
        values.append(g / count);
        int sec = (i / group) * stepSec;
        labels.append(QString("%1:%2").arg(sec / 60).arg(sec % 60, 2, 10, QChar('0')));
    }
    _speedChart->setData(labels, {{QString(), values, 0}}, [](double v){ return QString("%1/s").arg(humanBytes(v)); });
    _speedInfo->setText(tr("Last %1/s - best %2/s - average %3/s")
                        .arg(humanBytes(samples->last()), humanBytes(best), humanBytes(sum / n)));
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

    int id = e ? e->id : -1;
    bool paused = e && !finished && _ngPost->isPostPaused(id);
    _pauseButton->setEnabled(e && !finished && (paused || _ngPost->canPausePost(id)));
    _pauseButton->setText(paused ? tr("Resume") : tr("Pause"));
    _pauseButton->setIcon(tintedIcon(paused ? ":/icons/nav_play.png" : ":/icons/nav_pause.png",
                                     palette().color(QPalette::ButtonText)));
    if (paused)
        _pauseButton->setToolTip(tr("Resume this post"));
    else if (e && _ngPost->isPostPrepared(id))
        _pauseButton->setToolTip(tr("This post is already compressed for the next start: it can't be paused or moved"));
    else if (e && e->state == PostEntry::State::WAITING)
        _pauseButton->setToolTip(tr("Keep this post in the queue: the next posts go before it"));
    else
        _pauseButton->setToolTip(tr("Pause the upload of this post (the other posts wait)"));
    _priorityButton->setEnabled(e && e->queuePos >= 0);
    for (QAction *act : _priorityMenu->actions())
        act->setChecked(e && act->data().toInt() == e->priority);
    _topButton->setEnabled(e && _ngPost->canMovePost(id, QueueOrder::Move::TOP));
    _upButton->setEnabled(e && _ngPost->canMovePost(id, QueueOrder::Move::UP));
    _downButton->setEnabled(e && _ngPost->canMovePost(id, QueueOrder::Move::DOWN));
    _bottomButton->setEnabled(e && _ngPost->canMovePost(id, QueueOrder::Move::BOTTOM));
}

int PostsWidget::_rowId(int row) const
{
    if (row < 0 || row >= _table->rowCount() || !_table->item(row, COL_NAME))
        return -1;
    return _table->item(row, COL_NAME)->data(sIdRole).toInt();
}

void PostsWidget::onPauseResume()
{
    int id = selectedId();
    if (id < 0)
        return;
    if (_ngPost->isPostPaused(id))
        _ngPost->resumePost(id);
    else
        _ngPost->pausePost(id);
    _updateButtons();
    _showDetails();
}

void PostsWidget::_move(int move)
{
    int id = selectedId();
    if (id >= 0)
        _ngPost->movePost(id, static_cast<QueueOrder::Move>(move));
    _updateButtons();
}

void PostsWidget::_setPriority(int priority)
{
    int id = selectedId();
    if (id >= 0)
        _ngPost->setPostPriority(id, priority);
    _updateButtons();
    _showDetails();
}

bool PostsWidget::_canDrag(int id) const
{
    return id >= 0 && _ngPost->canDragPost(id);
}

void PostsWidget::_dropped(int id, int beforeRow)
{
    if (id < 0)
        return;
    // the post goes before the first waiting post at or after the row (none: at the end of the queue)
    int beforeId = -1;
    for (int row = qMax(0, beforeRow); row < _table->rowCount() && beforeId < 0; ++row)
    {
        int other = _rowId(row);
        const PostEntry *e = _list->entry(other);
        if (other != id && e && e->queuePos >= 0)
            beforeId = other;
    }
    _ngPost->movePostBefore(id, beforeId);
    selectPost(id);
    _updateButtons();
}

void PostsWidget::onOrderChanged()
{
    // the rows follow the order of the list (the waiting posts at the end, in the order they will start)
    int selected = selectedId();
    _table->blockSignals(true);
    QHash<int, QList<QTableWidgetItem*>> itemsOf;
    for (int row = 0; row < _table->rowCount(); ++row)
    {
        QList<QTableWidgetItem*> items;
        for (int col = 0; col < NB_COLUMNS; ++col)
            items << _table->takeItem(row, col);
        itemsOf.insert(items.first()->data(sIdRole).toInt(), items);
    }
    int row = 0;
    for (const PostEntry &e : _list->entries())
    {
        auto it = itemsOf.find(e.id);
        if (it == itemsOf.end() || row >= _table->rowCount())
            continue;
        for (int col = 0; col < NB_COLUMNS; ++col)
            _table->setItem(row, col, it.value().at(col));
        itemsOf.erase(it);
        ++row;
    }
    for (const QList<QTableWidgetItem*> &items : itemsOf) // (should not happen: every row is in the list)
    {
        for (int col = 0; col < NB_COLUMNS && row < _table->rowCount(); ++col)
            _table->setItem(row, col, items.at(col));
        ++row;
    }
    _table->clearSelection();
    _table->blockSignals(false);
    if (selected >= 0)
    {
        int r = _rowOf(selected);
        if (r >= 0)
        {
            _table->blockSignals(true);
            _table->selectRow(r);
            _table->blockSignals(false);
        }
    }
    _updateButtons();
}

void PostsWidget::onContextMenu(const QPoint &pos)
{
    int row = _table->rowAt(pos.y());
    if (row < 0)
        return;
    _table->selectRow(row);
    const PostEntry *e = _list->entry(_rowId(row));
    if (!e)
        return;
    _updateButtons();
    QMenu menu(this);
    auto addFrom = [&menu](QPushButton *b, const QString &text = QString()) {
        QAction *act = menu.addAction(b->icon(), text.isEmpty() ? b->text() : text);
        act->setEnabled(b->isEnabled() && b->isVisible());
        act->setToolTip(b->toolTip());
        QObject::connect(act, &QAction::triggered, b, &QAbstractButton::click);
        return act;
    };
    if (!e->isFinished())
    {
        addFrom(_pauseButton);
        addFrom(_stopButton);
        if (_priorityButton->isEnabled())
        {
            QMenu *prio = menu.addMenu(tr("Priority"));
            for (QAction *act : _priorityMenu->actions())
                prio->addAction(act);
        }
        if (e->queuePos >= 0)
        {
            menu.addSeparator();
            addFrom(_topButton, _topButton->toolTip());
            addFrom(_upButton, _upButton->toolTip());
            addFrom(_downButton, _downButton->toolTip());
            addFrom(_bottomButton, _bottomButton->toolTip());
        }
        menu.addSeparator();
    }
    if (_repostButton->isVisible())
        addFrom(_repostButton);
    addFrom(_folderButton);
    addFrom(_passButton);
    if (e->isFinished())
        addFrom(_removeButton);
    menu.exec(_table->viewport()->mapToGlobal(pos));
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
