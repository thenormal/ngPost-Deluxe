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

#ifndef STATSWIDGET_H
#define STATSWIDGET_H

#include <QWidget>
#include <QVector>
#include <QImage>
#include <QRectF>
#include <functional>
#include "HistoryWidget.h"

class NgPost;
class QLabel;
class QComboBox;
class QTableWidget;
class QGridLayout;
class QFrame;
class QPainter;
class QPushButton;

//! a simple chart drawn with QPainter (no extra library): vertical bars (stacked), lines, horizontal bars or a week/hour heat map
class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Type {Bars, Line, HBars, Heatmap};
    struct Series {
        QString         name;
        QVector<double> values;  //!< NaN: no value (a line is not drawn there)
        int             role = 0; //!< colour: 0 accent, 1 second colour, 2 success, 3 error
    };
    using Formatter = std::function<QString(double)>;

    ChartWidget(Type type, NgPost *ngPost, QWidget *parent = nullptr);

    void setData(const QStringList &labels, const QVector<Series> &series, const Formatter &fmt);
    //! grid[row][col]: rows are the days of the week, columns the hours
    void setHeatmap(const QVector<QVector<double>> &grid, const QStringList &rowLabels, const Formatter &fmt);
    bool isEmpty() const;
    void setBinaryScale(bool binary) { _binary = binary; } //!< steps of the scale in KB, MB, GB (sizes and speeds)

    //! draws the chart in a document or an image (light colours, no hover)
    void paintTo(QPainter &p, const QRectF &target);
    QImage toImage(const QSize &size, qreal scale = 2);
    Type type() const { return _type; }
    const QStringList &labels() const { return _labels; }
    const QVector<Series> &series() const { return _series; }
    const QVector<QVector<double>> &grid() const { return _grid; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void   _draw(QPainter &p, bool print);
    QRectF _plotRect() const;
    int    _hit(const QPoint &pos, int *col = nullptr) const;
    double _maxValue() const;
    QString _tooltip(int index, int col) const;

    Type              _type;
    NgPost           *_ngPost;
    QStringList       _labels;
    QVector<Series>   _series;
    QVector<QVector<double>> _grid;
    Formatter         _fmt;
    int               _hover    = -1;
    int               _hoverCol = -1;
    bool              _binary   = false;
    QRectF            _area;      //!< where the chart is drawn when it is exported (null: the widget)
};

//! Statistics of the History page: totals, charts and rankings computed from the history file
class StatsWidget : public QWidget
{
    Q_OBJECT
public:
    StatsWidget(NgPost *ngPost, QWidget *parent = nullptr);

    void setEntries(const QList<HistoryWidget::Entry> &entries, bool historyOn);
    void retranslate();

    enum Period {PERIOD_TODAY = 0, PERIOD_7_DAYS, PERIOD_30_DAYS, PERIOD_12_MONTHS, PERIOD_ALL, NB_PERIODS};

private:
    void _refresh();
    QFrame *_chartCard(QLabel *&title, ChartWidget *chart, QLabel *&hint);
    QString _kindName(const QString &key) const;
    QString _locDate(const QDate &date, const QString &format) const;
    QString _chartUnit(int chart) const;   //!< unit of the values of a chart in the exports
    QString _savePath(const QString &title, const QString &ext, const QString &filter);
    QString _reportTitle() const;          //!< "Statistics of ngPost Deluxe - Last 30 days"
    void _exportCsv();
    void _exportPdf();
    void _exportHtml();
    void _exportPng();

    NgPost      *_ngPost;
    QList<HistoryWidget::Entry> _entries;
    bool         _historyOn = true;

    QComboBox   *_period;
    QPushButton *_exportButton;
    QLabel      *_note;
    QLabel      *_empty;
    QWidget     *_content;
    QLabel      *_tileValue[8];
    QLabel      *_tileLabel[8];

    enum Chart {CH_DATA = 0, CH_SPEED, CH_RESULTS, CH_GROUPS, CH_SERVERS, CH_KINDS, CH_OPTIONS, CH_HEATMAP, NB_CHARTS};
    ChartWidget *_charts[NB_CHARTS];
    QLabel      *_chartTitle[NB_CHARTS];
    QLabel      *_chartHint[NB_CHARTS];

    QLabel       *_biggestTitle;
    QLabel       *_daysTitle;
    QTableWidget *_biggest;
    QTableWidget *_days;
};

#endif // STATSWIDGET_H
