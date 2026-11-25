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

#ifndef POSTSWIDGET_H
#define POSTSWIDGET_H

#include <QWidget>
#include <QHash>
#include "utils/PostList.h"

class NgPost;
class MainWindow;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QPlainTextEdit;
class QLineEdit;
class QCheckBox;
class ChartWidget;
class QSplitter;
class QMenu;

//! page Posts: one list of all the posts of the session (waiting, running, finished),
//! with the details and the log of the selected post
class PostsWidget : public QWidget
{
    Q_OBJECT

public:
    enum Column {COL_NAME = 0, COL_STATE, COL_PRIORITY, COL_PROGRESS, COL_SIZE, COL_SPEED, COL_ETA, COL_ORIGIN, NB_COLUMNS};
    static constexpr int sIdRole       = Qt::UserRole + 1; //!< id of the post (first column)
    static constexpr int sPercentRole  = Qt::UserRole + 2; //!< progress (column Progress)
    static constexpr int sStateRole    = Qt::UserRole + 3; //!< PostEntry::State (column Progress)

    PostsWidget(NgPost *ngPost, MainWindow *hmi, QWidget *parent = nullptr);

    void selectPost(int id);   //!< selects a post (and scrolls to it)
    void retranslate();
    int  selectedId() const;   //!< -1: nothing selected

    static QString stateText(PostEntry::State state);
    static QString originText(const QString &origin);
    static QString priorityText(int priority);

private slots:
    void onPostAdded(int id);
    void onPostChanged(int id);
    void onPostRemoved(int id);
    void onLogAdded(int id, const PostEntry::LogLine &line, bool continuesLine);
    void onSelectionChanged();

    void onStop();
    void onRepost();
    void onRemove();
    void onRemoveFinished();
    void onOpenFolder();
    void onCopyPassword();
    void onPauseResume();
    void onOrderChanged();
    void onContextMenu(const QPoint &pos);

private:
    void _fillRow(int row, const PostEntry &e);
    int  _rowOf(int id) const;
    void _showDetails();
    void _showLog();
    void _showSpeed();                         //!< graph of the speed of the selected post
    bool _logVisible(const PostEntry::LogLine &line) const; //!< filters of the log
    bool _logFiltered() const;                //!< some lines are hidden by the filters
    void _refreshLog();                       //!< the filters changed: the log is shown again
    void _updateButtons();
    void _move(int move);                     //!< QueueOrder::Move of the selected post
    void _setPriority(int priority);
    void _makeIcons();                        //!< arrows drawn in the color of the theme
    void _dropped(int id, int beforeRow);     //!< drag and drop in the table
    bool _canDrag(int id) const;
    int  _rowId(int row) const;
protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    void _updateEmpty();
    QString _logHtml(const PostEntry::LogLine &line) const;

    NgPost       *_ngPost;
    MainWindow   *_hmi;
    PostList     *_list;

    QLabel       *_title;
    QLabel       *_subtitle;
    QLabel       *_summary;
    QPushButton  *_stopButton;
    QPushButton  *_pauseButton;
    QPushButton  *_priorityButton;
    QPushButton  *_topButton;
    QPushButton  *_upButton;
    QPushButton  *_downButton;
    QPushButton  *_bottomButton;
    QMenu        *_priorityMenu;
    QPushButton  *_repostButton;
    QPushButton  *_folderButton;
    QPushButton  *_passButton;
    QPushButton  *_removeButton;
    QPushButton  *_clearButton;
    QTableWidget *_table;
    QLabel       *_emptyLbl;
    QSplitter    *_splitter;
    QLabel       *_detailsTitle;
    QLabel       *_details;
    QLabel       *_logTitle;
    QPlainTextEdit *_log;
    int           _logOf = -1; //!< post whose log is shown
    ChartWidget  *_speedChart;
    QLabel       *_speedTitle;
    QLabel       *_speedInfo;
    QLineEdit    *_logSearch;
    QCheckBox    *_logLevels[6]; //!< PostEntry::LogLevel: info, phase, ok, warn, debug, error
    QPushButton  *_logCopy;
    QLabel       *_logCount;

};

#endif // POSTSWIDGET_H
