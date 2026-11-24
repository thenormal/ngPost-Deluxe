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

#ifndef NFOPREVIEWDIALOG_H
#define NFOPREVIEWDIALOG_H

#include <QString>
class QWidget;

namespace NfoPreviewDialog
{
/*!
 * \brief Window to choose the look of the NFO file: the list of the styles, the option "only ASCII characters"
 * and a preview of an example post that follows the choices at once. The example can be copied to look at it in another program.
 * \param style  in: the current style (key); out: the chosen one
 * \param ascii  in / out: only ASCII characters
 * \return true if the user chose a style (\a style and \a ascii are changed)
 */
bool run(QWidget *parent, const QString &appName, const QString &appVersion, QString &style, bool &ascii);
}

#endif // NFOPREVIEWDIALOG_H
