// NfoWriter: the six styles, the ASCII version, the width of the lines, hidden names, long names, the BOM
#include "NfoWriter.h"
#include <QCoreApplication>
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)
#define CHECK_MSG(cond, msg) do { if (!(cond)) { std::printf("FAILED: %s (line %d): %s\n", #cond, __LINE__, qPrintable(msg)); ++failures; } } while (0)

static int maxWidth(const QString &text)
{
    int m = 0;
    for (const QString &l : text.split('\n'))
        m = qMax(m, l.size());
    return m;
}
static bool isAscii(const QString &text)
{
    for (QChar c : text)
        if (c.unicode() > 127)
            return false;
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // the keys
    CHECK(NfoWriter::styleKeys().size() == 6);
    CHECK(NfoWriter::DEFAULT_STYLE == NfoWriter::Scene);
    bool ok = false;
    CHECK(NfoWriter::styleFromKey(" Retro ", &ok) == NfoWriter::Retro && ok);
    NfoWriter::styleFromKey("nothing", &ok);
    CHECK(!ok);
    for (int i = 0; i < 6; ++i)
        CHECK(NfoWriter::styleFromKey(NfoWriter::styleKey(static_cast<NfoWriter::Style>(i))) == static_cast<NfoWriter::Style>(i));

    for (int s = 0; s < 6; ++s)
        for (bool ascii : {false, true})
        {
            const QString tag = QString("style %1 ascii %2").arg(NfoWriter::styleKey(static_cast<NfoWriter::Style>(s))).arg(ascii);
            NfoWriter::NfoInfo info = NfoWriter::sample("ngPost Deluxe", "1.5");
            info.style = static_cast<NfoWriter::Style>(s);
            info.ascii = ascii;

            // the example: everything is shown, nothing is wider than 80 columns
            QString t = NfoWriter::build(info);
            CHECK_MSG(maxWidth(t) <= 80, tag + " width " + QString::number(maxWidth(t)));
            for (const char *needle : {"Summer.Trip.2026.1080p", "Summer.Trip.2026.Part1.mkv", "k3Xq9vLm2Rt8", "Sibillini", "HEVC", "Italian (SRT)"})
                CHECK_MSG(t.contains(needle), tag + " is missing " + needle);
            if (ascii)
                CHECK_MSG(isAscii(t), tag + " has characters out of ASCII");
            CHECK_MSG(NfoWriter::encode(t).startsWith("\xEF\xBB\xBF") == !isAscii(t), tag + " BOM");

            // no password, no text, no media when they are not given
            NfoWriter::NfoInfo plain = info;
            plain.password.clear();
            plain.freeText.clear();
            plain.mediaEnabled = false;
            t = NfoWriter::build(plain);
            CHECK_MSG(!t.contains("k3Xq9vLm2Rt8") && !t.contains("Sibillini") && !t.contains("HEVC"), tag + " shows what is off");
            CHECK_MSG(!t.contains("Password"), tag + " Password line");

            // names hidden (obfuscation): no name of file, no media information, and the notes say so
            NfoWriter::NfoInfo hid = info;
            hid.hideNames = true;
            t = NfoWriter::build(hid);
            CHECK_MSG(!t.contains("Part1") && !t.contains(".srt") && !t.contains("HEVC"), tag + " shows names that are hidden");
            CHECK_MSG(t.contains("hidden") && t.contains("not written"), tag + " does not say that names are hidden");
            CHECK_MSG(t.contains("Summer.Trip.2026.1080p"), tag + " lost the name of the post");

            // very long names and many files: the lines stay in 80 columns, the files are all listed
            NfoWriter::NfoInfo big = info;
            big.postName = QString(150, QChar('p')) + ".final";
            big.files.clear();
            for (int i = 0; i < 40; ++i)
            {
                NfoWriter::File f;
                f.name = QString("folder/%1/").arg(i) + QString(120, QChar('a' + i % 20)) + QString(".part%1.mkv").arg(i);
                f.size = 1000 + i;
                big.files << f;
            }
            big.media.first().name = QString(150, QChar('m')) + ".mkv";
            big.media.first().video << QString(200, QChar('v'));
            big.freeText = QString("word ").repeated(60) + "\n" + QString(200, QChar('x'));
            big.mediaFiles = 30;
            t = NfoWriter::build(big);
            if (s != NfoWriter::Classic) // Classic is the old format: it never cut the lines
                CHECK_MSG(maxWidth(t) <= 80, tag + " long names: width " + QString::number(maxWidth(t)));
            CHECK_MSG(t.contains(".part39.mkv"), tag + " the extension of a long name is lost");
            CHECK_MSG(t.contains("28 more media"), tag + " note of the media files that are not described");

            // timed out
            big.mediaTimedOut = true;
            CHECK_MSG(NfoWriter::build(big).contains("after 20 seconds"), tag + " note of the time out");
        }

    // Classic is the format of the versions up to 1.4
    NfoWriter::NfoInfo c = NfoWriter::sample("ngPost Deluxe", "1.5");
    c.style = NfoWriter::Classic;
    QString t = NfoWriter::build(c);
    CHECK(t.startsWith(QString(70, QChar('=')) + "\n Summer.Trip.2026.1080p\n"));
    CHECK(t.contains(" Content     : 3 file(s), 4.49 GB\n"));
    CHECK(t.contains(" par2        : 10 % redundancy\n"));
    CHECK(t.contains("   Summer.Trip.2026.Part1.mkv - 01:02:14, 2.31 GB\n     Video     : HEVC 1920x1080, 25 fps\n"));
    CHECK(t.contains("     Subtitles : Italian (SRT), English (SRT)\n"));

    if (failures)
    {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("nfo_writer_test: all checks passed\n");
    return 0;
}
