// text written by the programs we start: UTF-8 is kept, the code page of the console is converted
#include "ProcessText.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

int main()
{
    const QString eGrave = QString(QChar(0x00E8));
    CHECK(ProcessText::decode(QByteArray()).isEmpty());
    CHECK(ProcessText::decode("plain ascii") == "plain ascii");
    // valid UTF-8 is kept (ParPar, a recent 7-Zip with -sccUTF-8)
    CHECK(ProcessText::decode(QByteArray("Il file \xC3\xA8 usato")) == "Il file " + eGrave + " usato");
    // a console code page: 0x8A is "e grave" in code page 850, 0xE8 in Windows-1252 / Latin 1; read as UTF-8 both gave a black diamond
    CHECK(ProcessText::decode(QByteArray("Il file \x8A usato"), "IBM850") == "Il file " + eGrave + " usato");
    CHECK(ProcessText::decode(QByteArray("Il file \xE8 usato"), "ISO-8859-1") == "Il file " + eGrave + " usato");
    // an unknown code page or no code page at all: the text is still given back, one character for each byte
    CHECK(ProcessText::decode(QByteArray("a\x8A" "b"), "no-such-codepage").size() == 3);
    CHECK(ProcessText::decode(QByteArray("a\x8A" "b")).size() == 3);
    if (failures == 0)
        std::printf("OK\n");
    return failures ? 1 : 0;
}
