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

#include "NfoPreviewDialog.h"
#include "utils/NfoWriter.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#define TR(text) QCoreApplication::translate("NfoPreview", text)

namespace {
//! a monospaced font that has the block and box characters (Consolas on Windows)
QFont previewFont()
{
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    const QStringList families = QFontDatabase().families();
    for (const char *name : {"Consolas", "DejaVu Sans Mono", "Menlo", "Liberation Mono"})
        if (families.contains(QLatin1String(name)))
        {
            f.setFamily(QLatin1String(name));
            break;
        }
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(10);
    return f;
}
}

bool NfoPreviewDialog::run(QWidget *parent, const QString &appName, const QString &appVersion, QString &styleKey, bool &asciiOnly)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(TR("NFO file: choose a style"));
    QVBoxLayout *v = new QVBoxLayout(&dlg);

    QLabel *intro = new QLabel(TR("The NFO file is a text file. Choose its look: the preview shows an example post. "
                                  "If the symbols look wrong in the program you open it with, use \"Only ASCII characters\"."), &dlg);
    intro->setWordWrap(true);
    v->addWidget(intro);

    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(new QLabel(TR("Style:"), &dlg));
    QComboBox *combo = new QComboBox(&dlg);
    for (const QString &key : NfoWriter::styleKeys())
        combo->addItem(NfoWriter::styleName(NfoWriter::styleFromKey(key)), key);
    combo->setCurrentIndex(qMax(0, NfoWriter::styleKeys().indexOf(styleKey)));
    top->addWidget(combo);
    QCheckBox *ascii = new QCheckBox(TR("Only ASCII characters"), &dlg);
    ascii->setToolTip(TR("No blocks and no boxes: for old systems and programs that show wrong symbols."));
    ascii->setChecked(asciiOnly);
    top->addSpacing(16);
    top->addWidget(ascii);
    top->addStretch(1);
    v->addLayout(top);

    QHBoxLayout *what = new QHBoxLayout();
    what->addWidget(new QLabel(TR("The example shows:"), &dlg));
    QCheckBox *media = new QCheckBox(TR("Media information"), &dlg);
    QCheckBox *pass  = new QCheckBox(TR("Password"), &dlg);
    QCheckBox *text  = new QCheckBox(TR("Text at the end"), &dlg);
    QCheckBox *hidden = new QCheckBox(TR("File names hidden"), &dlg);
    for (QCheckBox *cb : {media, pass, text})
        cb->setChecked(true);
    hidden->setToolTip(TR("What the NFO looks like when the names of the files are hidden (obfuscation)."));
    for (QCheckBox *cb : {media, pass, text, hidden})
        what->addWidget(cb);
    what->addStretch(1);
    v->addLayout(what);

    QPlainTextEdit *preview = new QPlainTextEdit(&dlg);
    preview->setReadOnly(true);
    preview->setLineWrapMode(QPlainTextEdit::NoWrap);
    preview->setFont(previewFont());
    QFontMetrics fm(preview->font());
    preview->setMinimumWidth(fm.horizontalAdvance(QString(82, QChar('M'))) + 40);
    preview->setMinimumHeight(fm.lineSpacing() * 34);
    v->addWidget(preview, 1);

    auto refresh = [=](){
        NfoWriter::NfoInfo info = NfoWriter::sample(appName, appVersion);
        info.style = NfoWriter::styleFromKey(combo->currentData().toString());
        info.ascii = ascii->isChecked();
        if (!media->isChecked())
            info.mediaEnabled = false;
        if (!pass->isChecked())
            info.password.clear();
        if (!text->isChecked())
            info.freeText.clear();
        info.hideNames = hidden->isChecked();
        preview->setPlainText(NfoWriter::build(info));
        preview->moveCursor(QTextCursor::Start);
    };
    QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), &dlg, refresh);
    for (QCheckBox *cb : {ascii, media, pass, text, hidden})
        QObject::connect(cb, &QCheckBox::toggled, &dlg, refresh);
    refresh();

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(TR("Use this style"));
    buttons->button(QDialogButtonBox::Cancel)->setText(TR("Cancel"));
    QPushButton *copy = buttons->addButton(TR("Copy the example"), QDialogButtonBox::ActionRole);
    copy->setToolTip(TR("Copies the example to the clipboard, to look at it in Notepad or in another program."));
    QObject::connect(copy, &QPushButton::clicked, &dlg, [=](){ QApplication::clipboard()->setText(preview->toPlainText()); });
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted)
        return false;
    styleKey  = combo->currentData().toString();
    asciiOnly = ascii->isChecked();
    return true;
}
