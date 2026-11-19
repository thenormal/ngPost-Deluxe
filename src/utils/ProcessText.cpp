#include "ProcessText.h"

#include <QTextCodec>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QString ProcessText::decode(const QByteArray &bytes, const char *fallbackCodec)
{
    if (bytes.isEmpty())
        return QString();

    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    if (utf8)
    {
        QTextCodec::ConverterState state;
        const QString text = utf8->toUnicode(bytes.constData(), bytes.size(), &state);
        if (state.invalidChars == 0)
            return text;
    }

    if (fallbackCodec)
    {
        QTextCodec *codec = QTextCodec::codecForName(fallbackCodec);
        if (codec)
            return codec->toUnicode(bytes);
    }
#ifdef Q_OS_WIN
    const int len = MultiByteToWideChar(CP_OEMCP, 0, bytes.constData(), bytes.size(), nullptr, 0);
    if (len > 0)
    {
        QString out(len, QChar());
        MultiByteToWideChar(CP_OEMCP, 0, bytes.constData(), bytes.size(), reinterpret_cast<wchar_t *>(out.data()), len);
        return out;
    }
#endif
    return QString::fromLocal8Bit(bytes);
}
