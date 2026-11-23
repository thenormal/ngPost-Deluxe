#ifndef GROUPSEDIT_H
#define GROUPSEDIT_H

#include <QFrame>
#include <QStringList>

class QLineEdit;
class QCompleter;
class QToolButton;
class QStringListModel;
class FlowLayout;

//! the newsgroups marked with a star: shared by all the newsgroups boxes (and saved by the main window)
class FavoriteGroups : public QObject
{
    Q_OBJECT
public:
    static FavoriteGroups *instance();
    const QStringList &groups() const { return _groups; }
    void setGroups(const QStringList &groups); //!< emits changed() if the list is different
    void toggle(const QString &group);
    bool contains(const QString &group) const { return _groups.contains(group); }
signals:
    void changed();
private:
    QStringList _groups;
};

//! Newsgroups box: each group is a "chip" (with a cross to remove it), new groups are typed (comma, space,
//! Enter or Tab ends one) or chosen in the suggestions. A name that does not look like a newsgroup is red.
class GroupsEdit : public QFrame
{
    Q_OBJECT

public:
    explicit GroupsEdit(QWidget *parent = nullptr);

    QStringList groups() const;          //!< all the groups, in order (the invalid ones too)
    QStringList invalidGroups() const;
    void setGroups(const QStringList &groups); //!< does not emit changed()
    void setEmptyHint(const QString &hint);   //!< text shown when there is no group (empty: the default one)
    void retranslate();

    static bool isValidGroup(const QString &group);
    static QStringList suggestions(); //!< the most used groups

signals:
    void changed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTextEdited(const QString &text);
    void commitText();

private:
    bool _addGroup(const QString &group);
    void _removeChip(QFrame *chip);
    void _updatePlaceholder();
    void _setFocused(bool focused);
    QFrame *_findChip(const QString &group) const;
    void _refreshFavorites();      //!< stars of the chips and order of the suggestions
    void _showFavoritesMenu();

    FlowLayout *_flow;
    QLineEdit  *_input;
    QCompleter *_completer;
    QStringListModel *_model;
    QToolButton *_favButton;
    QString     _emptyHint;
    QList<QFrame *> _chips;
    bool _loading;
};

#endif // GROUPSEDIT_H
