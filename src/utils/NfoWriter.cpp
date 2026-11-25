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

#include "NfoWriter.h"
#include "NfoBanners.h"
#include <QStringList>

namespace {
const int W = 78; //!< width of the new styles (a viewer may add a margin: 80 columns are never passed)

QString rep(QChar c, int n) { return QString(qMax(0, n), c); }
QString rule(int n)         { return rep(QChar(0x2500), n); } // thin line

//! at most \a max characters: a long text loses its middle ("name...ext")
QString elide(const QString &s, int max)
{
    if (s.size() <= max)
        return s;
    if (max <= 3)
        return s.left(max);
    int keep = max - 3, right = qMin(12, keep / 2);
    return s.left(keep - right) + "..." + s.right(right);
}
QString pad(const QString &s, int w)  { QString e = elide(s, w); return e + rep(' ', w - e.size()); }
QString padL(const QString &s, int w) { QString e = elide(s, w); return rep(' ', w - e.size()) + e; }
QString center(const QString &s, int w = W)
{
    QString e = elide(s, w);
    return rep(' ', (w - e.size()) / 2) + e;
}
QStringList centerBlock(const QStringList &lines, int w = W)
{
    int m = 0;
    for (const QString &l : lines)
        m = qMax(m, l.size());
    QStringList out;
    for (const QString &l : lines)
        out << rep(' ', qMax(0, (w - m) / 2)) + l;
    return out;
}
QString spaced(const QString &s)
{
    QString out;
    for (QChar c : s)
        out += QString(c) + ' ';
    return out.trimmed();
}

//! free text of the user: lines cut at \a width on the spaces
QStringList wrap(const QString &text, int width)
{
    QStringList out;
    for (QString line : text.trimmed().replace("\r", "").split('\n'))
    {
        line = line.trimmed();
        while (line.size() > width)
        {
            int cut = line.lastIndexOf(' ', width);
            if (cut <= 0)
                cut = width;
            out << line.left(cut).trimmed();
            line = line.mid(cut).trimmed();
        }
        out << line;
    }
    return out;
}

QString plural(int n, const char *one, const char *many) { return QString("%1 %2").arg(n).arg(n == 1 ? one : many); }

//! the characters that are not ASCII in the decorations, replaced by ASCII ones (same width)
QString toAscii(const QString &text)
{
    static const QString from = QString::fromUtf16(u"█▓▒░▀▄▌▐═║╔╗╚╝╠╣─■◆·»«");
    static const QString to   = "#%+.=_||=|++++++-**.><";
    QString out;
    out.reserve(text.size());
    for (QChar c : text)
    {
        int i = from.indexOf(c);
        if (i >= 0)
            out += to[i];
        else if (c.unicode() >= 0x2596 && c.unicode() <= 0x259f) // quarter blocks (small banner)
            out += '#';
        else
            out += c;
    }
    return out;
}

//! everything the styles show, prepared once
struct Content {
    QString name, footer;
    QList<QPair<QString, QString>> rows;     //!< label, value (the part "Release")
    bool    showBar = false;                 //!< the size of the upload is compared with the size of the content
    double  barRatio = 1.;
    bool    filesHidden = false;
    QList<QPair<QString, QString>> files;    //!< name, size
    QString total;
    bool    mediaHidden = false;
    QList<NfoWriter::MediaEntry> media;
    QString mediaNote;
    QStringList notes;
};

Content prepare(const NfoWriter::NfoInfo &info)
{
    Content c;
    c.name = info.postName;
    c.footer = QString("Posted with %1 · github.com/thenormal/ngPost-Deluxe").arg(info.appName);
    qint64 total = 0;
    for (const NfoWriter::File &f : info.files)
    {
        total += f.size;
        c.files << qMakePair(f.name, NfoWriter::humanSize(f.size));
    }
    c.total = NfoWriter::humanSize(total);
    c.filesHidden = info.hideNames;
    c.rows << qMakePair(QString("Posted with"), QString("%1 %2").arg(info.appName, info.appVersion))
           << qMakePair(QString("Date"), info.date.toString("yyyy-MM-dd hh:mm"))
           << qMakePair(QString("Content"), QString("%1, %2").arg(plural(info.files.size(), "file", "files"), c.total));
    if (info.compressed)
        c.rows << qMakePair(QString("Archive"), QString("%1, %2").arg(info.archiveFormat, plural(info.nbVolumes, "volume", "volumes")));
    if (info.par2)
        c.rows << qMakePair(QString("par2"), QString("%1 % redundancy").arg(info.par2Pct));
    if (info.compressed || info.par2)
        c.rows << qMakePair(QString("Uploaded"), NfoWriter::humanSize(info.uploadedSize));
    if (info.par2 && total > 0 && info.uploadedSize >= total)
    {
        c.showBar = true;
        c.barRatio = static_cast<double>(total) / static_cast<double>(info.uploadedSize);
    }
    if (!info.password.isEmpty())
        c.rows << qMakePair(QString("Password"), info.password);

    c.mediaHidden = info.mediaEnabled && info.hideNames;
    if (info.mediaEnabled && !info.hideNames && !info.media.isEmpty())
    {
        c.media = info.media;
        if (info.mediaTimedOut)
            c.mediaNote = "(reading stopped after 20 seconds: the other files are not described)";
        else if (info.mediaFiles > info.mediaDescribed)
            c.mediaNote = QString("... and %1 more media file(s)").arg(info.mediaFiles - info.mediaDescribed);
    }
    if (!info.freeText.trimmed().isEmpty())
        c.notes = wrap(info.freeText, W - 10);
    return c;
}

bool hasMedia(const Content &c) { return c.mediaHidden || !c.media.isEmpty(); }
const char *kFilesHidden = "The names of the files are hidden (obfuscation is on).";
const char *kMediaHidden = "The media information is not written (obfuscation is on).";

// ---------------------------------------------------------------- Classic (the format of the versions up to 1.4)

QStringList classic(const NfoWriter::NfoInfo &info)
{
    const QString line(70, QChar('='));
    QStringList o;
    qint64 total = 0;
    for (const NfoWriter::File &f : info.files)
        total += f.size;
    o << line << " " + elide(info.postName, 68) << line << "";
    o << QString(" Posted with : %1 %2").arg(info.appName, info.appVersion);
    o << QString(" Date        : %1").arg(info.date.toString("yyyy-MM-dd hh:mm"));
    o << QString(" Content     : %1 file(s), %2").arg(info.files.size()).arg(NfoWriter::humanSize(total));
    if (info.compressed)
        o << QString(" Archive     : %1, %2 volume(s)").arg(info.archiveFormat).arg(info.nbVolumes);
    if (info.par2)
        o << QString(" par2        : %1 % redundancy").arg(info.par2Pct);
    if (info.compressed || info.par2)
        o << QString(" Uploaded    : %1").arg(NfoWriter::humanSize(info.uploadedSize));
    if (!info.password.isEmpty())
        o << QString(" Password    : %1").arg(info.password);
    o << "" << " Files" << " -----";
    if (info.hideNames)
        o << QString(" ") + kFilesHidden;
    else
    {
        int width = 0;
        for (const NfoWriter::File &f : info.files)
            width = qMax(width, f.name.size());
        width = qMin(width, 52);
        for (const NfoWriter::File &f : info.files)
            o << QString("   %1  %2").arg(f.name, -width).arg(NfoWriter::humanSize(f.size), 10);
    }
    if (info.mediaEnabled && info.hideNames)
        o << "" << " Media information" << " -----------------" << QString(" ") + kMediaHidden;
    else if (info.mediaEnabled && !info.media.isEmpty())
    {
        o << "" << " Media information" << " -----------------";
        for (const NfoWriter::MediaEntry &m : info.media)
        {
            o << QString("   %1%2").arg(m.name, m.head.isEmpty() ? QString() : " - " + m.head);
            for (const QString &v : m.video)
                o << QString("     Video     : %1").arg(v);
            for (const QString &a : m.audio)
                o << QString("     Audio     : %1").arg(a);
            if (!m.subtitles.isEmpty())
                o << QString("     Subtitles : %1").arg(m.subtitles.join(", "));
        }
        if (info.mediaTimedOut)
            o << "   (reading stopped after 20 seconds: the other files are not described)";
        else if (info.mediaFiles > info.mediaDescribed)
            o << QString("   ... and %1 more media file(s)").arg(info.mediaFiles - info.mediaDescribed);
    }
    if (!info.freeText.trimmed().isEmpty())
    {
        o << "" << QString(70, QChar('-'));
        o << info.freeText.trimmed().replace("\r", "").split('\n');
    }
    o << "" << line;
    return o;
}

// ---------------------------------------------------------------- the new styles

QString mediaLine(const QString &key, const QString &value, int indent, int keyW, const QString &sep, int width)
{
    return rep(' ', indent) + pad(key, keyW) + sep + elide(value, width - indent - keyW - sep.size());
}

//! the lines of a media entry: name, head, then Video / Audio / Subtitles. \a prefix goes before each line
QStringList mediaLines(const NfoWriter::MediaEntry &m, const QString &prefix, const QString &mark, const QString &sep, int width)
{
    QStringList o;
    int w = width - prefix.size();
    o << prefix + mark + elide(m.name, w - mark.size());
    int ind = mark.size() ? mark.size() : 2;
    if (!m.head.isEmpty())
        o << prefix + rep(' ', ind) + elide(m.head, w - ind);
    auto add = [&](const QString &k, const QString &v){ o << prefix + mediaLine(k, v, ind, 10, sep, w); };
    for (const QString &v : m.video)
        add("Video", v);
    for (const QString &a : m.audio)
        add("Audio", a);
    if (!m.subtitles.isEmpty())
        add("Subtitles", m.subtitles.join(", "));
    return o;
}

QStringList clean(const NfoWriter::NfoInfo &info)
{
    Content c = prepare(info);
    const QChar full(0x2500);
    QStringList o;
    o << "";
    o += centerBlock(info.ascii ? NfoBanners::kBox() : NfoBanners::kClean());
    o << "" << center("U S E N E T   P O S T E R") << "" << rep(full, W) << "" << center(c.name) << "" << rep(full, W) << "";
    for (const auto &r : c.rows)
        o << "   " + padL(r.first, 12) + "   " + elide(r.second, W - 18);
    const QString ind("   "), sub = ind + rule(W - 6);
    o << "" << ind + "FILES" << sub;
    if (c.filesHidden)
        o << ind + kFilesHidden;
    else
    {
        for (const auto &f : c.files)
            o << ind + pad(f.first, 56) + padL(f.second, 16);
        o << sub << ind + pad("Total", 56) + padL(c.total, 16);
    }
    if (hasMedia(c))
    {
        o << "" << ind + "MEDIA" << sub;
        if (c.mediaHidden)
            o << ind + kMediaHidden;
        for (const NfoWriter::MediaEntry &m : c.media)
        {
            o << ind + elide(m.name, W - 6);
            if (!m.head.isEmpty())
                o << ind + "  " + elide(m.head, W - 8);
            for (const QString &v : m.video)
                o << ind + mediaLine("Video", v, 2, 10, " ", W - 3);
            for (const QString &a : m.audio)
                o << ind + mediaLine("Audio", a, 2, 10, " ", W - 3);
            if (!m.subtitles.isEmpty())
                o << ind + mediaLine("Subtitles", m.subtitles.join(", "), 2, 10, " ", W - 3);
            o << "";
        }
        if (!c.mediaNote.isEmpty())
            o << ind + c.mediaNote;
        else if (!c.media.isEmpty())
            o.removeLast(); // the empty line after the last file
    }
    if (!c.notes.isEmpty())
    {
        o << "" << ind + "NOTES" << sub;
        for (const QString &n : c.notes)
            o << ind + n;
    }
    o << "" << rep(full, W) << center(c.footer) << rep(full, W);
    return o;
}

QStringList box(const NfoWriter::NfoInfo &info)
{
    Content c = prepare(info);
    const int I = W - 4;
    const QChar dbl(0x2550), vert(0x2551);
    auto row = [&](const QString &s = QString()){ return QString(vert) + ' ' + pad(s, I) + ' ' + vert; };
    auto sep = [&](const QString &t = QString()){
        if (t.isEmpty())
            return QString(QChar(0x2560)) + rep(dbl, W - 2) + QChar(0x2563);
        QString label = "[ " + t + " ]";
        return QString(QChar(0x2560)) + rep(dbl, 3) + label + rep(dbl, W - 5 - label.size()) + QChar(0x2563);
    };
    QStringList o;
    o << QString(QChar(0x2554)) + rep(dbl, W - 2) + QChar(0x2557) << row();
    for (const QString &l : centerBlock(NfoBanners::kBox(), I))
        o << row(l);
    o << row() << sep() << row() << row(center(c.name, I)) << row() << sep("RELEASE") << row();
    for (const auto &r : c.rows)
        o << row("  " + pad(r.first, 12) + " " + elide(r.second, I - 15));
    o << row() << sep("FILES") << row();
    if (c.filesHidden)
        o << row("  " + QString(kFilesHidden));
    else
    {
        for (const auto &f : c.files)
            o << row("  " + pad(f.first, 54) + padL(f.second, 14));
        o << row("  " + rep(QChar(0x00b7), I - 4)) << row("  " + pad("Total", 54) + padL(c.total, 14));
    }
    o << row();
    if (hasMedia(c))
    {
        o << sep("MEDIA") << row();
        if (c.mediaHidden)
            o << row("  " + QString(kMediaHidden)) << row();
        for (const NfoWriter::MediaEntry &m : c.media)
        {
            for (const QString &l : mediaLines(m, "  ", "", " ", I))
                o << row(l);
            o << row();
        }
        if (!c.mediaNote.isEmpty())
            o << row("  " + c.mediaNote) << row();
    }
    if (!c.notes.isEmpty())
    {
        o << sep("NOTES") << row();
        for (const QString &n : c.notes)
            o << row("  " + n);
        o << row();
    }
    o << QString(QChar(0x255a)) + rep(dbl, W - 2) + QChar(0x255d) << center(c.footer);
    return o;
}

QStringList scene(const NfoWriter::NfoInfo &info)
{
    Content c = prepare(info);
    const QChar up(0x2580), low(0x2584), blk(0x2588), mid(0x2592);
    auto hdr = [&](const QString &t){
        QString label = " " + t + " ";
        int side = (W - label.size() - 8) / 2;
        return QString::fromUtf16(u"░▒▓█") + rep(up, side) + label + rep(up, W - 8 - side - label.size())
             + QString::fromUtf16(u"█▓▒░");
    };
    const QString pre("      ");
    QStringList o;
    o << rep(low, W) << "";
    if (info.ascii)
        o += centerBlock(NfoBanners::kAsciiBig());
    else
    {
        o += centerBlock(NfoBanners::kSceneNg());
        o << "";
        o += centerBlock(NfoBanners::kSceneDl());
    }
    o << "" << center(QString::fromUtf16(u"░▒▓█  U S E N E T   P O S T E R  █▓▒░")) << ""
      << rep(up, W) << "" << center(QString::fromUtf16(u"»  ") + c.name + QString::fromUtf16(u"  «")) << ""
      << hdr("RELEASE") << "";
    const QString bar(QChar(0x2590));
    for (const auto &r : c.rows)
        o << pre + padL(r.first, 12) + " " + bar + " " + elide(r.second, W - 6 - 12 - 3);
    if (c.showBar)
    {
        int len = 40, filled = qBound(1, qRound(len * c.barRatio), len);
        o << "" << pre + padL("Size", 12) + " " + bar + " " + rep(blk, filled) + rep(mid, len - filled) + "  data + par2";
    }
    o << "" << hdr("FILES") << "";
    const QString sq(QChar(0x25a0));
    if (c.filesHidden)
        o << pre + kFilesHidden;
    else
        for (const auto &f : c.files)
            o << pre + sq + " " + pad(f.first, 50) + padL(f.second, 14);
    if (hasMedia(c))
    {
        o << "" << hdr("MEDIA") << "";
        if (c.mediaHidden)
            o << pre + kMediaHidden;
        for (const NfoWriter::MediaEntry &m : c.media)
        {
            o << pre + sq + " " + elide(m.name, W - 8);
            if (!m.head.isEmpty())
                o << pre + "  " + elide(m.head, W - 8);
            auto add = [&](const QString &k, const QString &v){ o << pre + "  " + pad(k, 10) + " " + bar + " " + elide(v, W - 6 - 2 - 10 - 3); };
            for (const QString &v : m.video)
                add("Video", v);
            for (const QString &a : m.audio)
                add("Audio", a);
            if (!m.subtitles.isEmpty())
                add("Subtitles", m.subtitles.join(", "));
            o << "";
        }
        if (!c.mediaNote.isEmpty())
            o << pre + c.mediaNote << "";
    }
    if (!c.notes.isEmpty())
    {
        o << hdr("NOTES") << "";
        for (const QString &n : c.notes)
            o << pre + n;
    }
    o << "" << rep(low, W) << center(c.footer) << rep(up, W);
    return o;
}

QStringList elegant(const NfoWriter::NfoInfo &info)
{
    Content c = prepare(info);
    const QString dash = rule(11);
    auto orn = [&](const QString &t){ return center(dash + "  " + spaced(t) + "  " + dash); };
    const QString diamond(QChar(0x25c6));
    QStringList o;
    o << "";
    o += centerBlock(NfoBanners::kElegant());
    o << "" << center(diamond) << "" << center(c.name) << center(info.date.toString("yyyy-MM-dd hh:mm")) << "" << orn("RELEASE") << "";
    for (const auto &r : c.rows)
        if (r.first != "Date")
            o << center(padL(r.first, 12) + "   " + pad(r.second, 26) , W);
    o << "" << orn("FILES") << "";
    if (c.filesHidden)
        o << center(kFilesHidden);
    else
        for (const auto &f : c.files)
            o << center(pad(f.first, 34) + padL(f.second, 12));
    if (hasMedia(c))
    {
        o << "" << orn("MEDIA") << "";
        if (c.mediaHidden)
            o << center(kMediaHidden);
        for (const NfoWriter::MediaEntry &m : c.media)
        {
            QStringList l;
            l << m.name;
            if (!m.head.isEmpty())
                l << "  " + m.head;
            auto add = [&](const QString &k, const QString &v){ l << "  " + pad(k, 10) + " " + QChar(0x00b7) + " " + v; };
            for (const QString &v : m.video)
                add("Video", v);
            for (const QString &a : m.audio)
                add("Audio", a);
            if (!m.subtitles.isEmpty())
                add("Subtitles", m.subtitles.join(", "));
            for (QString &x : l)
                x = elide(x, W - 12);
            o += centerBlock(l);
            o << "";
        }
        if (!c.mediaNote.isEmpty())
            o << center(c.mediaNote) << "";
    }
    if (!c.notes.isEmpty())
    {
        o << orn("NOTES") << "";
        for (const QString &n : c.notes)
            o << center(n);
        o << "";
    }
    o << center(diamond) << "" << center(QString("posted with %1 %2").arg(info.appName, info.appVersion));
    return o;
}

QStringList retro(const NfoWriter::NfoInfo &info)
{
    Content c = prepare(info);
    const QChar lite(0x2591), med(0x2592), dark(0x2593), bar(0x258c);
    QStringList o;
    o << rep(lite, W) << rep(med, W) << rep(dark, W) << "";
    o += centerBlock(info.ascii ? NfoBanners::kAsciiBig() : NfoBanners::kRetro());
    o << "" << rep(dark, W) << rep(med, W) << rep(lite, W) << "";
    auto sec = [&](const QString &t){ return QStringList() << "" << QString(bar) + " " + t << QString(bar) + rep(QChar(0x2500), W - 1); };
    const QString b(bar);
    o << b + " " + elide(c.name, W - 2) << "";
    o += sec("RELEASE");
    for (const auto &r : c.rows)
        o << b + "  " + pad(r.first, 12) + " " + elide(r.second, W - 17);
    o += sec("FILES");
    if (c.filesHidden)
        o << b + "  " + kFilesHidden;
    else
        for (const auto &f : c.files)
            o << b + "  " + pad(f.first, 56) + padL(f.second, 12);
    if (hasMedia(c))
    {
        o += sec("MEDIA");
        if (c.mediaHidden)
            o << b + "  " + kMediaHidden;
        for (const NfoWriter::MediaEntry &m : c.media)
            for (const QString &l : mediaLines(m, b + "  ", "", " ", W))
                o << l;
        if (!c.mediaNote.isEmpty())
            o << b + "  " + c.mediaNote;
    }
    if (!c.notes.isEmpty())
    {
        o += sec("NOTES");
        for (const QString &n : c.notes)
            o << b + "  " + n;
    }
    o << "" << rep(lite, W) << center(c.footer);
    return o;
}
} // namespace

// ---------------------------------------------------------------- public

QString NfoWriter::humanSize(qint64 bytes)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double v = static_cast<double>(bytes);
    int u = 0;
    while (v >= 1024. && u < 4)
    {
        v /= 1024.;
        ++u;
    }
    return u == 0 ? QString("%1 B").arg(bytes) : QString("%1 %2").arg(v, 0, 'f', 2).arg(units[u]);
}

QStringList NfoWriter::styleKeys()
{
    return {"classic", "clean", "box", "scene", "elegant", "retro"};
}

QString NfoWriter::styleKey(Style style)
{
    return styleKeys().value(static_cast<int>(style));
}

QString NfoWriter::styleName(Style style)
{
    QString k = styleKey(style);
    return k.left(1).toUpper() + k.mid(1);
}

NfoWriter::Style NfoWriter::styleFromKey(const QString &key, bool *ok)
{
    int i = styleKeys().indexOf(key.trimmed().toLower());
    if (ok)
        *ok = i >= 0;
    return i >= 0 ? static_cast<Style>(i) : DEFAULT_STYLE;
}

QString NfoWriter::build(const NfoInfo &info)
{
    QStringList lines;
    switch (info.style)
    {
    case Classic: lines = classic(info); break;
    case Clean:   lines = clean(info);   break;
    case Box:     lines = box(info);     break;
    case Scene:   lines = scene(info);   break;
    case Elegant: lines = elegant(info); break;
    case Retro:   lines = retro(info);   break;
    }
    QString out = lines.join("\n") + "\n";
    return info.ascii ? toAscii(out) : out;
}

NfoWriter::NfoInfo NfoWriter::sample(const QString &appName, const QString &appVersion)
{
    NfoInfo i;
    i.postName   = "Summer.Trip.2026.1080p";
    i.appName    = appName;
    i.appVersion = appVersion;
    i.date       = QDateTime::currentDateTime();
    auto file = [](const char *name, qint64 size){ File f; f.name = name; f.size = size; return f; };
    i.files << file("Summer.Trip.2026.Part1.mkv", 2480000000LL) << file("Summer.Trip.2026.Part2.mkv", 2340000000LL)
            << file("Summer.Trip.2026.srt", 49350);
    i.compressed = true;
    i.archiveFormat = "7z";
    i.nbVolumes  = 46;
    i.par2       = true;
    i.par2Pct    = 10;
    i.uploadedSize = 5290000000LL;
    i.password   = "k3Xq9vLm2Rt8";
    i.freeText   = "Home videos of the summer trip in the Sibillini mountains.\nShot with a small drone and a phone. Enjoy!";
    i.mediaEnabled = true;
    i.mediaFiles = i.mediaDescribed = 2;
    MediaEntry one, two;
    one.name = "Summer.Trip.2026.Part1.mkv";
    one.head = "01:02:14, 2.31 GB";
    one.video << "HEVC 1920x1080, 25 fps";
    one.audio << "Italian, AAC, 2.0, 192 kb/s";
    one.subtitles << "Italian (SRT)" << "English (SRT)";
    two.name = "Summer.Trip.2026.Part2.mkv";
    two.head = "00:58:40, 2.18 GB";
    two.video << "HEVC 1920x1080, 25 fps";
    two.audio << "Italian, AAC, 2.0, 192 kb/s";
    i.media << one << two;
    return i;
}

QByteArray NfoWriter::encode(const QString &text)
{
    bool plain = true;
    for (QChar c : text)
        if (c.unicode() > 127)
        {
            plain = false;
            break;
        }
    return (plain ? QByteArray() : QByteArray("\xEF\xBB\xBF")) + text.toUtf8();
}
