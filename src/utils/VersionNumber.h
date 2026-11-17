//========================================================================
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//========================================================================

#ifndef VERSIONNUMBER_H
#define VERSIONNUMBER_H

#include <QString>
#include <QStringList>

/*!
 * \brief Comparison of the version numbers of the releases ("1.3", "v1.10", "1.3.1").
 * A version with a "-" (v1.4-rc1, a release candidate) is never considered newer: only the real releases are announced.
 */
namespace VersionNumber
{
    //! "v1.3" or "1.3" -> "1.3"; empty when it is not a plain number (like "1.4-rc1" or "abc")
    inline QString clean(QString tag)
    {
        tag = tag.trimmed();
        if (tag.startsWith('v') || tag.startsWith('V'))
            tag.remove(0, 1);
        if (tag.isEmpty())
            return QString();
        for (const QString &part : tag.split('.'))
        {
            bool ok = false;
            part.toUInt(&ok);
            if (!ok)
                return QString();
        }
        return tag;
    }

    //! true when "candidate" is a real release with a higher number than "current"
    inline bool isNewer(const QString &candidate, const QString &current)
    {
        const QString a = clean(candidate), b = clean(current);
        if (a.isEmpty() || b.isEmpty())
            return false;
        const QStringList pa = a.split('.'), pb = b.split('.');
        const int n = qMax(pa.size(), pb.size());
        for (int i = 0; i < n; ++i)
        {
            const uint x = i < pa.size() ? pa[i].toUInt() : 0, y = i < pb.size() ? pb[i].toUInt() : 0;
            if (x != y)
                return x > y;
        }
        return false;
    }
}

#endif // VERSIONNUMBER_H
