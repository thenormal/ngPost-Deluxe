// SettingsTransfer: what is exported (never a secret nor a path), how an import merges into a configuration
#include "SettingsTransfer.h"
#include <QCoreApplication>
#include <cstdio>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAILED: %s (line %d)\n", #cond, __LINE__); ++failures; } } while (0)

static const char *kConfig =
"# ngPost configuration file\n"
"lang = IT\n"
"theme = DARK\n"
"nzbPath  = C:/Users/pino/nzb\n"
"SHUTDOWN_CMD = shutdown /s /f /t 0\n"
"NZB_UPLOAD_URL = ftp://user:topsecretftp@host:21\n"
"PROXY_SOCKS5 = bob:proxypass@192.168.1.1:5555\n"
"NZB_POST_CMD = curl -F api=12345 https://example.com\n"
"notify_ok = ON\n"
"notify_when = each\n"
"notify_discord = https://discord.com/api/webhooks/123/SECRETTOKEN\n"
"notify_telegram_token = 999:telegramsecret\n"
"notify_telegram_chat = 42\n"
"notify_webhook = https://hooks.example.com/abc\n"
"gen_nfo = ON\n"
"nfo_style = retro\n"
"nfo_text = Hello=world\\nsecond line\n"
"max_speed = 12.5\n"
"favorite_groups = alt.binaries.test,alt.binaries.misc\n"
"name_counter = 41\n"
"profile = Film|1|1|1|1|10|0\n"
"profile = Series|1|1|0|1|5|1|alt.binaries.tv\n"
"GROUPS   = alt.binaries.test\n"
"TMP_DIR = C:/Temp\n"
"RAR_PATH = C:/Program Files/7-Zip/7z.exe\n"
"RAR_PASS = fixedarchivepass\n"
"RAR_SIZE = 50\n"
"POST_HISTORY = C:/nzb/history.csv\n"
"mediainfo_path = C:/mi/MediaInfo.exe\n"
"\n"
"##############################################################\n"
"##                   servers section                        ##\n"
"##############################################################\n"
"\n"
"[server]\n"
"host = news.provider.com\n"
"port = 563\n"
"ssl  = true\n"
"user = pino\n"
"pass = serverpassword\n"
"connection = 20\n"
"enabled = true\n"
"nzbCheck = false\n"
"\n"
"[server]\n"
"host = free.example.org\n"
"port = 119\n"
"ssl  = false\n"
"user = \n"
"pass = \n"
"connection = 4\n"
"enabled = true\n"
"nzbCheck = false\n";

static bool has(const QString &t, const char *s) { return t.contains(QLatin1String(s)); }

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QString config = QString::fromLatin1(kConfig);
    const QDateTime when(QDate(2026, 11, 24), QTime(15, 30));

    // 1. the export has no secret, no path, no command, nothing local
    SettingsTransfer::ExportOptions all;
    all.servers = true;
    QString out = SettingsTransfer::exportText(config, all, "ngPost Deluxe", "1.5", when);
    CHECK(out.startsWith("# ngPost Deluxe settings (format 1)\n"));
    for (const char *secret : {"topsecretftp", "proxypass", "SECRETTOKEN", "telegramsecret", "hooks.example.com", "fixedarchivepass", "serverpassword",
                               "curl", "C:/", "shutdown", "history.csv", "name_counter", "999:", "notify_discord", "notify_webhook"})
        CHECK(!has(out, secret));
    CHECK(!has(out, "pass ="));
    for (const char *wanted : {"lang = IT", "theme = DARK", "gen_nfo = ON", "nfo_style = retro", "max_speed = 12.5", "rar_size = 50",
                               "notify_ok = ON", "notify_when = each", "profile = Film|1|1|1|1|10|0", "favorite_groups = alt.binaries.test,alt.binaries.misc",
                               "host = news.provider.com", "user = pino", "nfo_text = Hello=world\\nsecond line"})
        CHECK(has(out, wanted));

    // 2. the parts can be left out
    SettingsTransfer::ExportOptions none;
    none.general = none.notifications = none.profiles = none.servers = false;
    QString empty = SettingsTransfer::exportText(config, none, "ngPost Deluxe", "1.5", when);
    CHECK(!has(empty, "lang =") && !has(empty, "profile =") && !has(empty, "[server]") && !has(empty, "notify_ok"));
    SettingsTransfer::ExportOptions def; // default: no servers
    CHECK(!has(SettingsTransfer::exportText(config, def, "ngPost Deluxe", "1.5", when), "[server]"));

    // 3. a file of settings is read back; the secrets and the paths of a file edited by hand are ignored
    QString hand = out + "nzbPath = C:/evil\nRAR_PASS = leaked\nnotify_discord = https://x\nunknown_key = 1\n[server]\nhost = h\nport = 1\npass = sneaky\nuser = u\n";
    SettingsTransfer::Parsed p = SettingsTransfer::parse(hand);
    CHECK(p.valid);
    CHECK(p.profiles.size() == 2 && p.favoriteGroups == "alt.binaries.test,alt.binaries.misc");
    CHECK(p.servers.size() == 3);
    CHECK(p.ignored == 4);
    CHECK(!SettingsTransfer::parse("lang = FR\n").valid);
    CHECK(!SettingsTransfer::parse("").valid);
    bool nfoTextOk = false;
    for (const auto &kv : p.settings)
        if (kv.first == "nfo_text" && kv.second == "Hello=world\\nsecond line")
            nfoTextOk = true;
    CHECK(nfoTextOk);

    // 4. import on another computer (a fresh configuration): the settings, the profiles, the servers without password, switched off
    const QString fresh = "# ngPost configuration file\nlang = EN\nnzbPath = /home/other/nzb\nGROUPS = alt.binaries.test2\n\n"
                          "##############################################################\n"
                          "##                   servers section                        ##\n"
                          "##############################################################\n\n";
    SettingsTransfer::ImportOptions opt;
    SettingsTransfer::Summary sum;
    QString merged = SettingsTransfer::merge(fresh, SettingsTransfer::parse(out), opt, &sum);
    CHECK(has(merged, "lang = IT") && !has(merged, "lang = EN"));         // replaced in place
    CHECK(has(merged, "nzbPath = /home/other/nzb"));                       // the local settings stay
    CHECK(has(merged, "GROUPS = alt.binaries.test"));                       // replaced (key kept as written)
    CHECK(has(merged, "profile = Film|1|1|1|1|10|0") && has(merged, "profile = Series|"));
    CHECK(sum.profilesNew == 2 && sum.profilesSame == 0);
    CHECK(sum.serversNew == 2 && sum.serversNeedPassword == 1);
    CHECK(!has(merged, "serverpassword"));
    CHECK(merged.indexOf("nfo_style = retro") < merged.indexOf("servers section")); // new settings before the servers
    CHECK(merged.indexOf("[server]") > merged.indexOf("servers section"));
    SettingsTransfer::Parsed back = SettingsTransfer::parse(SettingsTransfer::exportText(merged, all, "ngPost Deluxe", "1.5", when));
    CHECK(back.servers.size() == 2 && back.servers.at(0).enabled == "false" && back.servers.at(1).enabled == "true");
    CHECK(back.servers.at(0).user == "pino");

    // 5. import in a configuration that has already some: same profile kept or replaced, same server not added twice, favorites merged
    const QString current = "lang = FR\nprofile = Film|0|0|0|0|0|0\nprofile = Mine|1|0|0|1|8|0\nfavorite_groups = alt.binaries.new,alt.binaries.test\n\n"
                            "[server]\nhost = NEWS.provider.com\nport = 563\nssl = true\nuser = Pino\npass = keepme\nconnection = 8\nenabled = true\n";
    SettingsTransfer::Parsed in = SettingsTransfer::parse(out);
    merged = SettingsTransfer::merge(current, in, opt, &sum);
    CHECK(has(merged, "profile = Film|0|0|0|0|0|0") && !has(merged, "profile = Film|1|1|1|1|10|0"));     // kept
    CHECK(has(merged, "profile = Mine|1|0|0|1|8|0") && has(merged, "profile = Series|"));
    CHECK(sum.profilesNew == 1 && sum.profilesSame == 1);
    CHECK(sum.serversPresent == 1 && sum.serversNew == 1);
    CHECK(merged.count("[server]") == 2 && has(merged, "pass = keepme") && has(merged, "connection = 8"));
    CHECK(has(merged, "favorite_groups = alt.binaries.new,alt.binaries.test,alt.binaries.misc"));
    opt.replaceProfiles = true;
    merged = SettingsTransfer::merge(current, in, opt, &sum);
    CHECK(has(merged, "profile = Film|1|1|1|1|10|0") && !has(merged, "profile = Film|0|0|0|0|0|0"));
    CHECK(has(merged, "profile = Mine|1|0|0|1|8|0"));

    // 6. parts left out of the import; summarize changes nothing
    SettingsTransfer::ImportOptions few;
    few.general = few.notifications = false;
    few.servers = false;
    merged = SettingsTransfer::merge(current, in, few, &sum);
    CHECK(has(merged, "lang = FR") && !has(merged, "theme") && !has(merged, "notify_ok") && merged.count("[server]") == 1);
    CHECK(sum.settings == 0 && sum.serversNew == 0 && sum.profilesNew == 1);
    sum = SettingsTransfer::summarize(in, current, SettingsTransfer::ImportOptions());
    CHECK(sum.serversNew == 1 && sum.settings > 5);

    // 7. an import twice is the same as once
    QString once = SettingsTransfer::merge(fresh, in, SettingsTransfer::ImportOptions(), &sum);
    QString twice = SettingsTransfer::merge(once, in, SettingsTransfer::ImportOptions(), &sum);
    CHECK(once == twice);
    CHECK(sum.profilesNew == 0 && sum.serversNew == 0);

    if (failures)
    {
        std::printf("%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("settings_transfer_test: all checks passed\n");
    return 0;
}
