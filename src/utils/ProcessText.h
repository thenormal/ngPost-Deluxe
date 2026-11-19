#ifndef PROCESSTEXT_H
#define PROCESSTEXT_H

#include <QByteArray>
#include <QString>

/*!
 * \brief Text written by the programs we start (7-Zip, rar, par2...).
 * On Windows they write in the code page of the console (OEM, for example 850 in Italy or Germany), not in UTF-8:
 * read as UTF-8 an "e grave" becomes a "?" in a black diamond. Text that is valid UTF-8 is kept as it is.
 */
namespace ProcessText
{
//! \a fallbackCodec: name of the code page used when the bytes are not UTF-8 (default: the OEM code page on Windows, else the system one)
QString decode(const QByteArray &bytes, const char *fallbackCodec = nullptr);
}

#endif // PROCESSTEXT_H
