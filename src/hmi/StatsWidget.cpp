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
//
//========================================================================

#include "StatsWidget.h"
#include "NgPost.h"
#include "utils/NfoWriter.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMap>
#include <QLocale>
#include <QtMath>
#include <algorithm>

namespace {

struct Palette {
    QColor text, sub, grid, empty, hover;
    QColor roles[4];
};

Palette chartPalette(NgPost *ngPost)
{
    Palette p;
    if (ngPost && ngPost->isDarkTheme())
    {
        p.text  = QColor("#e6e6e6");
        p.sub   = QColor("#9d9d9d");
        p.grid  = QColor("#3a3a3a");
        p.empty = QColor("#2f2f2f");
        p.hover = QColor(255, 255, 255, 18);
        p.roles[0] = QColor("#4cc2ff");
        p.roles[1] = QColor("#b4a0ff");
        p.roles[2] = QColor("#4cc38a");
        p.roles[3] = QColor("#ff6b6b");
    }
    else
    {
        p.text  = QColor("#1f1f1f");
        p.sub   = QColor("#6b6b6b");
        p.grid  = QColor("#e6e6e6");
        p.empty = QColor("#f1f1f1");
        p.hover = QColor(0, 0, 0, 14);
        p.roles[0] = QColor("#0067c0");
        p.roles[1] = QColor("#8764b8");
        p.roles[2] = QColor("#2e9e4f");
        p.roles[3] = QColor("#d13438");
    }
    return p;
}

//! 1, 2 or 5 times a power of 10, so the scale is easy to read
double niceStep(double raw)
{
    if (raw <= 0)
        return 1;
    double p = qPow(10, qFloor(std::log10(raw)));
    double f = raw / p;
    double n = f <= 1 ? 1 : f <= 2 ? 2 : f <= 5 ? 5 : 10;
    return n * p;
}

QString hs(double bytes) { return NfoWriter::humanSize(static_cast<qint64>(bytes)); }

} // namespace

// ============================================================ ChartWidget

ChartWidget::ChartWidget(Type type, NgPost *ngPost, QWidget *parent) :
    QWidget(parent), _type(type), _ngPost(ngPost)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ChartWidget::setData(const QStringList &labels, const QVector<Series> &series, const Formatter &fmt)
{
    _labels = labels;
    _series = series;
    _fmt    = fmt;
    _hover  = -1;
    updateGeometry();
    update();
}

void ChartWidget::setHeatmap(const QVector<QVector<double>> &grid, const QStringList &rowLabels, const Formatter &fmt)
{
    _grid   = grid;
    _labels = rowLabels;
    _fmt    = fmt;
    _hover  = -1;
    update();
}

bool ChartWidget::isEmpty() const
{
    if (_type == Type::Heatmap)
    {
        for (const QVector<double> &row : _grid)
            for (double v : row)
                if (v > 0)
                    return false;
        return true;
    }
    for (const Series &s : _series)
        for (double v : s.values)
            if (!qIsNaN(v) && v > 0)
                return false;
    return true;
}

QSize ChartWidget::sizeHint() const
{
    if (_type == Type::HBars)
        return QSize(360, qMax(64, _labels.size() * 26 + 8));
    return QSize(360, 210);
}

QSize ChartWidget::minimumSizeHint() const
{
    if (_type == Type::HBars)
        return QSize(220, qMax(64, _labels.size() * 26 + 8));
    return QSize(220, 170);
}

QRectF ChartWidget::_plotRect() const
{
    QRectF r = rect();
    switch (_type)
    {
    case Type::Bars:
    case Type::Line:
        return r.adjusted(68, _series.size() > 1 ? 26 : 8, -10, -24);
    case Type::HBars:
    {
        double labelW = qMin(r.width() * 0.38, 210.0);
        return r.adjusted(labelW, 4, -96, -4);
    }
    case Type::Heatmap:
        return r.adjusted(48, 4, -6, -22);
    }
    return r;
}

double ChartWidget::_maxValue() const
{
    double max = 0;
    if (_type == Type::Bars) // stacked
    {
        for (int i = 0; i < _labels.size(); ++i)
        {
            double sum = 0;
            for (const Series &s : _series)
                if (i < s.values.size() && !qIsNaN(s.values.at(i)))
                    sum += s.values.at(i);
            max = qMax(max, sum);
        }
    }
    else if (_type == Type::Heatmap)
    {
        for (const QVector<double> &row : _grid)
            for (double v : row)
                max = qMax(max, v);
    }
    else
    {
        for (const Series &s : _series)
            for (double v : s.values)
                if (!qIsNaN(v))
                    max = qMax(max, v);
    }
    return max;
}

int ChartWidget::_hit(const QPoint &pos, int *col) const
{
    const QRectF plot = _plotRect();
    const int n = _labels.size();
    if (n == 0)
        return -1;
    switch (_type)
    {
    case Type::Bars:
    {
        if (pos.x() < plot.left() || pos.x() > plot.right() || pos.y() < plot.top() - 4 || pos.y() > plot.bottom() + 20)
            return -1;
        int i = static_cast<int>((pos.x() - plot.left()) / (plot.width() / n));
        return qBound(0, i, n - 1);
    }
    case Type::Line:
    {
        if (pos.x() < plot.left() - 6 || pos.x() > plot.right() + 6 || pos.y() < plot.top() - 4 || pos.y() > plot.bottom() + 20)
            return -1;
        if (n == 1)
            return 0;
        int i = qRound((pos.x() - plot.left()) / (plot.width() / (n - 1)));
        return qBound(0, i, n - 1);
    }
    case Type::HBars:
    {
        int i = static_cast<int>((pos.y() - plot.top()) / 26);
        return pos.y() >= plot.top() && i < n ? i : -1;
    }
    case Type::Heatmap:
    {
        if (!plot.contains(pos))
            return -1;
        int r = static_cast<int>((pos.y() - plot.top()) / (plot.height() / 7));
        int c = static_cast<int>((pos.x() - plot.left()) / (plot.width() / 24));
        if (col)
            *col = qBound(0, c, 23);
        return qBound(0, r, 6);
    }
    }
    return -1;
}

QString ChartWidget::_tooltip(int index, int col) const
{
    if (index < 0)
        return QString();
    if (_type == Type::Heatmap)
    {
        double v = _grid.value(index).value(col);
        return QString("%1 %2:00 - %2:59\n%3").arg(_labels.value(index)).arg(col, 2, 10, QChar('0')).arg(_fmt(v));
    }
    QStringList lines;
    lines << _labels.value(index);
    for (const Series &s : _series)
    {
        double v = s.values.value(index);
        QString value = qIsNaN(v) ? QString::fromUtf8("—") : _fmt(v);
        lines << (s.name.isEmpty() ? value : QString("%1: %2").arg(s.name, value));
    }
    return lines.join("\n");
}

void ChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    int col = -1;
    int idx = _hit(event->pos(), &col);
    if (idx != _hover || col != _hoverCol)
    {
        _hover    = idx;
        _hoverCol = col;
        update();
    }
    if (idx >= 0 && !isEmpty())
        QToolTip::showText(event->globalPos(), _tooltip(idx, col), this);
    else
        QToolTip::hideText();
}

void ChartWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    _hover = -1;
    update();
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const Palette pal = chartPalette(_ngPost);
    QFont small = font();
    small.setPointSizeF(qMax(7.0, font().pointSizeF() - 1));
    p.setFont(small);
    const QFontMetrics fm(small);

    if (isEmpty())
    {
        p.setPen(pal.sub);
        p.drawText(rect(), Qt::AlignCenter, tr("No data for this period."));
        return;
    }

    const QRectF plot = _plotRect();
    const int n = _labels.size();
    const double max = _maxValue();

    if (_type == Type::Bars || _type == Type::Line)
    {
        // scale: a round step, as many lines as needed (at most 5)
        double unit = 1;
        if (_binary && max > 0)
            unit = qPow(1024, qFloor(std::log(max) / std::log(1024.0)));
        double step = niceStep(max / unit / 4) * unit;
        int lines = qBound(1, static_cast<int>(std::ceil(max / step - 1e-9)), 5);
        double top = step * lines;
        if (top <= 0)
            top = 1;
        for (int i = 0; i <= lines; ++i)
        {
            double y = plot.bottom() - plot.height() * i / lines;
            p.setPen(QPen(pal.grid, 1));
            p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            p.setPen(pal.sub);
            p.drawText(QRectF(0, y - 9, plot.left() - 8, 18), Qt::AlignRight | Qt::AlignVCenter, _fmt(step * i));
        }
        // legend
        if (_series.size() > 1)
        {
            double x = plot.left();
            for (const Series &s : _series)
            {
                p.setPen(Qt::NoPen);
                p.setBrush(pal.roles[s.role & 3]);
                p.drawRoundedRect(QRectF(x, 6, 10, 10), 2, 2);
                p.setPen(pal.text);
                p.drawText(QPointF(x + 15, 15), s.name);
                x += 15 + fm.horizontalAdvance(s.name) + 18;
            }
        }
        const double slot = plot.width() / n;
        // labels under the axis: only as many as fit
        int longest = 0;
        for (const QString &l : _labels)
            longest = qMax(longest, fm.horizontalAdvance(l));
        const double xStep = _type == Type::Line && n > 1 ? plot.width() / (n - 1) : slot;
        int every = qMax(1, static_cast<int>(std::ceil((longest + 10) / qMax(1.0, xStep))));
        p.setPen(pal.sub);
        for (int i = 0; i < n; i += every)
        {
            double cx = _type == Type::Line ? (n == 1 ? plot.center().x() : plot.left() + i * xStep) : plot.left() + slot * (i + 0.5);
            const double w = fm.horizontalAdvance(_labels.at(i)) + 4;
            const double x0 = qBound(0.0, cx - w / 2, width() - w); // the first and last labels stay inside
            p.drawText(QRectF(x0, plot.bottom() + 4, w, 18), Qt::AlignHCenter | Qt::AlignTop, _labels.at(i));
        }

        if (_type == Type::Bars)
        {
            if (_hover >= 0)
            {
                p.setPen(Qt::NoPen);
                p.setBrush(pal.hover);
                p.drawRect(QRectF(plot.left() + slot * _hover, plot.top(), slot, plot.height()));
            }
            const double barW = qMax(1.0, qMin(slot * 0.72, 46.0));
            for (int i = 0; i < n; ++i)
            {
                double base = 0;
                double x = plot.left() + slot * i + (slot - barW) / 2;
                for (const Series &s : _series)
                {
                    double v = s.values.value(i);
                    if (qIsNaN(v) || v <= 0)
                        continue;
                    double h = plot.height() * v / top;
                    QRectF bar(x, plot.bottom() - base - h, barW, h);
                    p.setPen(Qt::NoPen);
                    p.setBrush(pal.roles[s.role & 3]);
                    p.drawRect(bar);
                    base += h;
                }
            }
        }
        else // Line
        {
            auto pointAt = [&](int i, double v) {
                double x = n == 1 ? plot.center().x() : plot.left() + i * xStep;
                return QPointF(x, plot.bottom() - plot.height() * v / top);
            };
            if (_hover >= 0)
            {
                double x = pointAt(_hover, 0).x();
                p.setPen(QPen(pal.grid, 1, Qt::DashLine));
                p.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
            }
            for (int k = _series.size() - 1; k >= 0; --k) // the first series is drawn last, on top
            {
                const Series &s = _series.at(k);
                const QColor c = pal.roles[s.role & 3];
                QPainterPath path;
                bool started = false;
                for (int i = 0; i < n; ++i)
                {
                    double v = s.values.value(i);
                    if (qIsNaN(v))
                    {
                        started = false;
                        continue;
                    }
                    QPointF pt = pointAt(i, v);
                    if (started)
                        path.lineTo(pt);
                    else
                        path.moveTo(pt);
                    started = true;
                }
                p.setPen(QPen(c, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                p.setBrush(Qt::NoBrush);
                p.drawPath(path);
                p.setPen(Qt::NoPen);
                p.setBrush(c);
                for (int i = 0; i < n; ++i)
                {
                    double v = s.values.value(i);
                    if (!qIsNaN(v))
                        p.drawEllipse(pointAt(i, v), i == _hover ? 4.5 : 2.5, i == _hover ? 4.5 : 2.5);
                }
            }
        }
    }
    else if (_type == Type::HBars)
    {
        const Series s = _series.value(0);
        const double rowH = 26;
        for (int i = 0; i < n; ++i)
        {
            QRectF row(0, plot.top() + i * rowH, width(), rowH);
            if (i == _hover)
            {
                p.setPen(Qt::NoPen);
                p.setBrush(pal.hover);
                p.drawRect(row);
            }
            p.setPen(pal.text);
            p.drawText(QRectF(4, row.top(), plot.left() - 12, rowH), Qt::AlignRight | Qt::AlignVCenter,
                       fm.elidedText(_labels.at(i), Qt::ElideMiddle, static_cast<int>(plot.left() - 16)));
            double v = s.values.value(i);
            double w = max > 0 ? plot.width() * v / max : 0;
            p.setPen(Qt::NoPen);
            p.setBrush(pal.roles[s.role & 3]);
            p.drawRoundedRect(QRectF(plot.left(), row.top() + 5, qMax(2.0, w), rowH - 10), 3, 3);
            p.setPen(pal.sub);
            p.drawText(QRectF(plot.left() + w + 6, row.top(), 90, rowH), Qt::AlignLeft | Qt::AlignVCenter, _fmt(v));
        }
    }
    else // Heatmap
    {
        const double cw = plot.width() / 24, ch = plot.height() / 7;
        QColor accent = pal.roles[0];
        for (int r = 0; r < 7; ++r)
        {
            p.setPen(pal.sub);
            p.drawText(QRectF(0, plot.top() + r * ch, plot.left() - 6, ch), Qt::AlignRight | Qt::AlignVCenter, _labels.value(r));
            for (int c = 0; c < 24; ++c)
            {
                double v = _grid.value(r).value(c);
                QColor fill = pal.empty;
                if (v > 0 && max > 0)
                {
                    fill = accent;
                    fill.setAlphaF(0.18 + 0.82 * v / max);
                }
                QRectF cell(plot.left() + c * cw + 1, plot.top() + r * ch + 1, cw - 2, ch - 2);
                p.setPen(r == _hover && c == _hoverCol ? QPen(pal.text, 1.5) : Qt::NoPen);
                p.setBrush(fill);
                p.drawRoundedRect(cell, 2, 2);
            }
        }
        p.setPen(pal.sub);
        for (int c = 0; c < 24; c += 3)
            p.drawText(QRectF(plot.left() + c * cw - 10, plot.bottom() + 4, 2 * cw + 20, 16), Qt::AlignLeft | Qt::AlignTop,
                       QString("%1").arg(c, 2, 10, QChar('0')));
    }
}

// ============================================================ StatsWidget

StatsWidget::StatsWidget(NgPost *ngPost, QWidget *parent) :
    QWidget(parent), _ngPost(ngPost)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 8, 8);
    root->setSpacing(10);

    QHBoxLayout *bar = new QHBoxLayout();
    _period = new QComboBox(this);
    for (int i = 0; i < NB_PERIODS; ++i)
        _period->addItem(QString());
    _period->setCurrentIndex(PERIOD_30_DAYS);
    bar->addWidget(_period);
    bar->addStretch();
    root->addLayout(bar);

    _note = new QLabel(this);
    _note->setObjectName("hint");
    _note->setWordWrap(true);
    root->addWidget(_note);

    _empty = new QLabel(this);
    _empty->setObjectName("emptyTitle");
    _empty->setAlignment(Qt::AlignCenter);
    _empty->setWordWrap(true);
    _empty->setMinimumHeight(160);
    root->addWidget(_empty);

    _content = new QWidget(this);
    QVBoxLayout *cl = new QVBoxLayout(_content);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(12);
    root->addWidget(_content);
    root->addStretch();

    // totals
    QGridLayout *tiles = new QGridLayout();
    tiles->setSpacing(10);
    for (int i = 0; i < 8; ++i)
    {
        QFrame *card = new QFrame(_content);
        card->setObjectName("card");
        QVBoxLayout *v = new QVBoxLayout(card);
        v->setContentsMargins(14, 10, 14, 10);
        v->setSpacing(2);
        _tileValue[i] = new QLabel(card);
        _tileValue[i]->setObjectName("statValue");
        _tileLabel[i] = new QLabel(card);
        _tileLabel[i]->setObjectName("hint");
        _tileLabel[i]->setWordWrap(true);
        v->addWidget(_tileValue[i]);
        v->addWidget(_tileLabel[i]);
        tiles->addWidget(card, i / 4, i % 4);
    }
    cl->addLayout(tiles);

    // charts, two by row
    const ChartWidget::Type types[NB_CHARTS] = {ChartWidget::Type::Bars, ChartWidget::Type::Line, ChartWidget::Type::Bars,
                                                ChartWidget::Type::HBars, ChartWidget::Type::HBars, ChartWidget::Type::HBars,
                                                ChartWidget::Type::HBars, ChartWidget::Type::Heatmap};
    QGridLayout *charts = new QGridLayout();
    charts->setSpacing(10);
    static const int order[NB_CHARTS] = {CH_DATA, CH_SPEED, CH_RESULTS, CH_HEATMAP, CH_GROUPS, CH_SERVERS, CH_KINDS, CH_OPTIONS};
    for (int k = 0; k < NB_CHARTS; ++k)
    {
        int i = order[k];
        _charts[i] = new ChartWidget(types[i], _ngPost, _content);
        charts->addWidget(_chartCard(_chartTitle[i], _charts[i], _chartHint[i]), k / 2, k % 2);
    }
    charts->setColumnStretch(0, 1);
    charts->setColumnStretch(1, 1);
    cl->addLayout(charts);

    // rankings
    auto table = [this](int cols) {
        QTableWidget *t = new QTableWidget(0, cols, _content);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionMode(QAbstractItemView::NoSelection);
        t->setFocusPolicy(Qt::NoFocus);
        t->setShowGrid(false);
        t->setAlternatingRowColors(true);
        t->verticalHeader()->hide();
        t->verticalHeader()->setDefaultSectionSize(26);
        t->horizontalHeader()->setStretchLastSection(false);
        t->horizontalHeader()->setHighlightSections(false);
        t->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        t->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        t->setFixedHeight(26 * 10 + 34);
        return t;
    };
    QGridLayout *ranks = new QGridLayout();
    ranks->setSpacing(10);
    _biggest = table(4);
    _days    = table(3);
    _biggest->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _biggest->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _days->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QFrame *bigCard = new QFrame(_content), *dayCard = new QFrame(_content);
    for (QFrame *f : {bigCard, dayCard})
        f->setObjectName("card");
    QVBoxLayout *bl = new QVBoxLayout(bigCard), *dl = new QVBoxLayout(dayCard);
    _biggestTitle = new QLabel(bigCard);
    _daysTitle    = new QLabel(dayCard);
    _biggestTitle->setObjectName("cardTitle");
    _daysTitle->setObjectName("cardTitle");
    bl->addWidget(_biggestTitle);
    bl->addWidget(_biggest);
    dl->addWidget(_daysTitle);
    dl->addWidget(_days);
    ranks->addWidget(bigCard, 0, 0);
    ranks->addWidget(dayCard, 0, 1);
    ranks->setColumnStretch(0, 3);
    ranks->setColumnStretch(1, 2);
    cl->addLayout(ranks);

    connect(_period, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ _refresh(); });
}

QFrame *StatsWidget::_chartCard(QLabel *&title, ChartWidget *chart, QLabel *&hint)
{
    QFrame *card = new QFrame(_content);
    card->setObjectName("card");
    QVBoxLayout *v = new QVBoxLayout(card);
    v->setContentsMargins(14, 10, 14, 10);
    v->setSpacing(6);
    title = new QLabel(card);
    title->setObjectName("cardTitle");
    hint = new QLabel(card);
    hint->setObjectName("hint");
    hint->setWordWrap(true);
    v->addWidget(title);
    v->addWidget(chart, 1);
    v->addWidget(hint);
    return card;
}

QString StatsWidget::_locDate(const QDate &date, const QString &format) const
{
    return QLocale(_ngPost->_lang).toString(date, format);
}

QString StatsWidget::_kindName(const QString &key) const
{
    if (key == "quick")  return tr("Quick Post");
    if (key == "choose") return tr("Auto Posting: chosen items");
    if (key == "scan")   return tr("Auto Posting: folder scan");
    if (key == "watch")  return tr("Auto Posting: watched folder");
    if (key == "cli")    return tr("Command line");
    return key;
}

void StatsWidget::retranslate()
{
    _period->setItemText(PERIOD_TODAY,     tr("Today"));
    _period->setItemText(PERIOD_7_DAYS,    tr("Last 7 days"));
    _period->setItemText(PERIOD_30_DAYS,   tr("Last 30 days"));
    _period->setItemText(PERIOD_12_MONTHS, tr("Last 12 months"));
    _period->setItemText(PERIOD_ALL,       tr("All"));
    _chartTitle[CH_DATA]->setText(tr("Data uploaded"));
    _chartTitle[CH_SPEED]->setText(tr("Speed"));
    _chartTitle[CH_RESULTS]->setText(tr("Results"));
    _chartTitle[CH_GROUPS]->setText(tr("Most used newsgroups"));
    _chartTitle[CH_SERVERS]->setText(tr("Servers"));
    _chartTitle[CH_KINDS]->setText(tr("How the posts were started"));
    _chartTitle[CH_OPTIONS]->setText(tr("Options"));
    _chartTitle[CH_HEATMAP]->setText(tr("When you post (day and hour)"));
    _biggestTitle->setText(tr("Largest posts"));
    _daysTitle->setText(tr("Busiest days"));
    _biggest->setHorizontalHeaderLabels({tr("Date"), tr("Name"), tr("Size"), tr("Speed")});
    _days->setHorizontalHeaderLabels({tr("Day"), tr("Posts"), tr("Size")});
    _refresh();
}

void StatsWidget::setEntries(const QList<HistoryWidget::Entry> &entries, bool historyOn)
{
    _entries   = entries;
    _historyOn = historyOn;
    _refresh();
}

static QString durationText(qint64 sec)
{
    if (sec >= 86400)
        return StatsWidget::tr("%1 d %2 h").arg(sec / 86400).arg((sec % 86400) / 3600);
    if (sec >= 3600)
        return StatsWidget::tr("%1 h %2 min").arg(sec / 3600).arg((sec % 3600) / 60);
    return StatsWidget::tr("%1 min %2 s").arg(sec / 60).arg(sec % 60);
}

void StatsWidget::_refresh()
{
    using Entry = HistoryWidget::Entry;
    const QDateTime now = QDateTime::currentDateTime();
    const QDate today = now.date();
    const int period = _period->currentIndex();
    QDateTime from;
    switch (period)
    {
    case PERIOD_TODAY:     from = QDateTime(today, QTime(0, 0)); break;
    case PERIOD_7_DAYS:    from = QDateTime(today.addDays(-6), QTime(0, 0)); break;
    case PERIOD_30_DAYS:   from = QDateTime(today.addDays(-29), QTime(0, 0)); break;
    case PERIOD_12_MONTHS: from = QDateTime(QDate(today.year(), today.month(), 1).addMonths(-11), QTime(0, 0)); break;
    default: break;
    }

    QList<const Entry*> sel;
    QDate first;
    int known = 0, exact = 0;
    for (const Entry &e : _entries)
    {
        if (!first.isValid() || e.date.date() < first)
            first = e.date.date();
        if (from.isValid() && e.date < from)
            continue;
        sel << &e;
        if (!e.kind.isEmpty())
            ++known;
        if (!e.exactBytes.isEmpty())
            ++exact;
    }

    if (!_historyOn || sel.isEmpty())
    {
        _empty->setText(!_historyOn ? tr("The history is turned off: turn it on in the Settings to get statistics.")
                                    : _entries.isEmpty() ? tr("No upload yet: the statistics appear after your first post.")
                                                         : tr("No upload in this period."));
        _empty->show();
        _content->hide();
        _note->setVisible(_historyOn && !_entries.isEmpty());
        if (first.isValid())
            _note->setText(tr("Data since %1 (%2 post(s) in the history).").arg(_locDate(first, "d MMMM yyyy")).arg(_entries.size()));
        return;
    }
    _empty->hide();
    _content->show();
    _note->show();
    QString note = tr("Data since %1 (%2 post(s) in the history).").arg(_locDate(first, "d MMMM yyyy")).arg(_entries.size());
    if (exact < sel.size())
        note += " " + tr("The sizes of the posts made before version 1.4 are rounded.");
    _note->setText(note);

    // ---------- totals
    const int n = sel.size();
    double bytes = 0, weighted = 0, best = 0, biggest = 0;
    qint64 seconds = 0;
    int errors = 0;
    QString biggestName;
    for (const Entry *e : sel)
    {
        bytes += e->bytes;
        if (e->speedBps > 0)
        {
            weighted += e->bytes / e->speedBps;
            best = qMax(best, e->speedBps);
        }
        if (e->durationSec > 0)
            seconds += e->durationSec;
        if (e->status.startsWith("ERRORS"))
            ++errors;
        if (e->bytes > biggest)
        {
            biggest = e->bytes;
            biggestName = e->name;
        }
    }
    const double avgSpeed = weighted > 0 ? bytes / weighted : 0;
    int days = 1;
    if (period == PERIOD_7_DAYS)
        days = 7;
    else if (period == PERIOD_30_DAYS)
        days = 30;
    else if (period == PERIOD_12_MONTHS)
        days = static_cast<int>(from.date().daysTo(today)) + 1;
    else if (period == PERIOD_ALL)
        days = static_cast<int>(first.daysTo(today)) + 1;
    const QLocale loc(_ngPost->_lang);
    const QString values[8] = {
        loc.toString(n), hs(bytes), durationText(seconds),
        QString("%1 %").arg(loc.toString(100.0 * (n - errors) / n, 'f', 1)),
        avgSpeed > 0 ? hs(avgSpeed) + "/s" : QString::fromUtf8("—"),
        best > 0 ? hs(best) + "/s" : QString::fromUtf8("—"),
        hs(biggest), hs(bytes / qMax(1, days))};
    const QString labels[8] = {tr("Posts"), tr("Data uploaded"), tr("Time spent posting"), tr("Posts without errors"),
                               tr("Average speed"), tr("Best speed"), tr("Largest post"), tr("Average per day")};
    for (int i = 0; i < 8; ++i)
    {
        _tileValue[i]->setText(values[i]);
        _tileLabel[i]->setText(labels[i]);
    }
    _tileValue[6]->setToolTip(biggestName);
    _tileValue[2]->setToolTip(tr("Compression, par2 and upload"));

    // ---------- buckets of time
    enum class Unit {Hour, Day, Month};
    Unit unit = Unit::Day;
    QDate start = from.isValid() ? from.date() : first;
    if (period == PERIOD_TODAY)
        unit = Unit::Hour;
    else if (period == PERIOD_12_MONTHS || (period == PERIOD_ALL && first.daysTo(today) > 62))
        unit = Unit::Month;
    QStringList bucketLabels;
    int nbBuckets = 0;
    if (unit == Unit::Hour)
    {
        nbBuckets = 24;
        for (int h = 0; h < 24; ++h)
            bucketLabels << QString("%1:00").arg(h, 2, 10, QChar('0'));
    }
    else if (unit == Unit::Day)
    {
        nbBuckets = static_cast<int>(start.daysTo(today)) + 1;
        for (int i = 0; i < nbBuckets; ++i)
            bucketLabels << _locDate(start.addDays(i), "d MMM");
    }
    else
    {
        start = QDate(start.year(), start.month(), 1);
        nbBuckets = (today.year() - start.year()) * 12 + today.month() - start.month() + 1;
        for (int i = 0; i < nbBuckets; ++i)
            bucketLabels << _locDate(start.addMonths(i), "MMM yy");
    }
    auto bucketOf = [&](const QDateTime &d) {
        if (unit == Unit::Hour)
            return d.time().hour();
        if (unit == Unit::Day)
            return static_cast<int>(start.daysTo(d.date()));
        return (d.date().year() - start.year()) * 12 + d.date().month() - start.month();
    };
    QVector<double> data(nbBuckets, 0), ok(nbBuckets, 0), ko(nbBuckets, 0), best1(nbBuckets, qQNaN()),
            sumBytes(nbBuckets, 0), sumTime(nbBuckets, 0);
    QVector<QVector<double>> heat(7, QVector<double>(24, 0));
    QMap<QString, double> groups, servers, kinds, options;
    QMap<QDate, QPair<int, double>> perDay;
    for (const Entry *e : sel)
    {
        int b = bucketOf(e->date);
        if (b >= 0 && b < nbBuckets)
        {
            data[b] += e->bytes;
            (e->status.startsWith("ERRORS") ? ko[b] : ok[b]) += 1;
            if (e->speedBps > 0)
            {
                sumBytes[b] += e->bytes;
                sumTime[b]  += e->bytes / e->speedBps;
                best1[b] = qIsNaN(best1[b]) ? e->speedBps : qMax(best1[b], e->speedBps);
            }
        }
        heat[e->date.date().dayOfWeek() - 1][e->date.time().hour()] += 1;
        for (const QString &g : e->groups.split(',', Qt::SkipEmptyParts))
            groups[g.trimmed()] += 1;
        for (const QString &s : e->servers.split(',', Qt::SkipEmptyParts))
            servers[s.trimmed()] += 1;
        if (!e->kind.isEmpty())
            kinds[e->kind] += 1;
        if (!e->options.isEmpty())
        {
            const QStringList keys = e->options.split('+');
            for (const QString &k : keys)
                options[k.startsWith("par2") ? QString("par2") : k] += 1;
        }
        QPair<int, double> &day = perDay[e->date.date()];
        day.first  += 1;
        day.second += e->bytes;
    }
    QVector<double> avg(nbBuckets, qQNaN());
    for (int i = 0; i < nbBuckets; ++i)
        if (sumTime[i] > 0)
            avg[i] = sumBytes[i] / sumTime[i];

    const ChartWidget::Formatter fmtBytes = [](double v) { return hs(v); };
    const ChartWidget::Formatter fmtSpeed = [](double v) { return hs(v) + "/s"; };
    const ChartWidget::Formatter fmtPosts = [loc](double v) { return loc.toString(static_cast<qint64>(qRound64(v))); };

    _charts[CH_DATA]->setBinaryScale(true);
    _charts[CH_SPEED]->setBinaryScale(true);
    _charts[CH_DATA]->setData(bucketLabels, {{QString(), data, 0}}, fmtBytes);
    _charts[CH_SPEED]->setData(bucketLabels, {{tr("Average"), avg, 0}, {tr("Best"), best1, 1}}, fmtSpeed);
    _charts[CH_RESULTS]->setData(bucketLabels, {{tr("Without errors"), ok, 2}, {tr("With errors"), ko, 3}}, fmtPosts);

    auto ranked = [](const QMap<QString, double> &map, int maxRows, QStringList &labels, QVector<double> &values) {
        QList<QPair<double, QString>> list;
        for (auto it = map.cbegin(); it != map.cend(); ++it)
            list << qMakePair(it.value(), it.key());
        std::sort(list.begin(), list.end(), [](const QPair<double, QString> &a, const QPair<double, QString> &b) {
            return a.first != b.first ? a.first > b.first : a.second < b.second;
        });
        labels.clear();
        values.clear();
        for (int i = 0; i < list.size() && i < maxRows; ++i)
        {
            labels << list.at(i).second;
            values << list.at(i).first;
        }
    };
    QStringList lbl;
    QVector<double> val;
    ranked(groups, 10, lbl, val);
    _charts[CH_GROUPS]->setData(lbl, {{QString(), val, 0}}, fmtPosts);
    ranked(servers, 10, lbl, val);
    _charts[CH_SERVERS]->setData(lbl, {{QString(), val, 1}}, fmtPosts);
    ranked(kinds, 10, lbl, val);
    for (QString &k : lbl)
        k = _kindName(k);
    _charts[CH_KINDS]->setData(lbl, {{QString(), val, 0}}, fmtPosts);
    ranked(options, 10, lbl, val);
    for (QString &k : lbl)
        k = k == "compress" ? tr("Compression") : k == "password" ? tr("Password") : k == "par2" ? tr("par2")
          : k == "keep" ? tr("Archives kept") : k == "none" ? tr("Files posted as they are") : k;
    _charts[CH_OPTIONS]->setData(lbl, {{QString(), val, 1}}, fmtPosts);
    QStringList dayNames;
    for (int d = 1; d <= 7; ++d)
        dayNames << loc.dayName(d, QLocale::ShortFormat);
    _charts[CH_HEATMAP]->setHeatmap(heat, dayNames, [loc](double v) {
        return StatsWidget::tr("%1 post(s)").arg(loc.toString(static_cast<qint64>(qRound64(v))));
    });

    _chartHint[CH_DATA]->setText(unit == Unit::Hour ? tr("By hour") : unit == Unit::Day ? tr("By day") : tr("By month"));
    _chartHint[CH_SPEED]->setText(tr("Average and best speed of the posts"));
    _chartHint[CH_RESULTS]->setText(tr("Number of posts"));
    _chartHint[CH_GROUPS]->setText(tr("Number of posts in each newsgroup (10 most used)"));
    _chartHint[CH_HEATMAP]->setText(tr("Number of posts started at each hour of each day of the week"));
    const QString partial = known < n ? tr("Known for %1 of the %2 post(s): it is recorded since version 1.4.").arg(known).arg(n)
                                      : tr("Number of posts");
    for (int c : {CH_SERVERS, CH_KINDS, CH_OPTIONS})
        _chartHint[c]->setText(partial);

    // ---------- rankings
    QList<const Entry*> big = sel;
    std::sort(big.begin(), big.end(), [](const Entry *a, const Entry *b) { return a->bytes > b->bytes; });
    _biggest->setRowCount(qMin(10, big.size()));
    for (int i = 0; i < _biggest->rowCount(); ++i)
    {
        const Entry *e = big.at(i);
        const QStringList cells = {_locDate(e->date.date(), "d MMM yyyy"), e->name, hs(e->bytes), e->speed};
        for (int c = 0; c < cells.size(); ++c)
        {
            QTableWidgetItem *item = new QTableWidgetItem(cells.at(c));
            if (c >= 2)
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            _biggest->setItem(i, c, item);
        }
    }
    QList<QDate> busiest = perDay.keys();
    std::sort(busiest.begin(), busiest.end(), [&perDay](const QDate &a, const QDate &b) {
        return perDay.value(a).second > perDay.value(b).second;
    });
    _days->setRowCount(qMin(10, busiest.size()));
    for (int i = 0; i < _days->rowCount(); ++i)
    {
        const QDate d = busiest.at(i);
        const QStringList cells = {_locDate(d, "ddd d MMM yyyy"), loc.toString(perDay.value(d).first), hs(perDay.value(d).second)};
        for (int c = 0; c < cells.size(); ++c)
        {
            QTableWidgetItem *item = new QTableWidgetItem(cells.at(c));
            if (c >= 1)
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            _days->setItem(i, c, item);
        }
    }
}
