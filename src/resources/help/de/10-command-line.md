# Kommandozeile

Das Programm kann auch ohne Fenster laufen, für einen Server, ein NAS oder ein Skript. Die Kommandozeilenversion ist dasselbe Programm ohne das Fenster gebaut (oder das Docker-Image).

## Beispiele

Zwei Elemente posten, mit Komprimierung, zufälligem Namen und Passwort und par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Jedes Element eines Ordners posten, je ein Post:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Einen Ordner überwachen und die Dateien nach dem Posten in einen Ordner „Posted“ verschieben:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` akzeptiert einen Namen wie `Posted` (der Ordner wird neben jeder Datei oder jedem Ordner angelegt) oder einen vollständigen Pfad. Verwenden Sie stattdessen `--rm_posted`, um die Dateien nach dem Posten zu löschen. Beide gelten nur, wenn alles fehlerfrei gepostet wurde.

## Nützliche Optionen

- `-c` Konfigurationsdatei (sonst `$HOME/.ngPost`)
- `-g` Newsgroups, durch Kommas getrennt
- `-o` NZB-Ausgabedatei
- `-S` Server in einer Zeichenkette
- `--disp_progress` BAR oder FILES
- `--resume_queue` nicht beendete Posts erneut posten
- `--allow_repost` Dateien, die schon im Verlauf stehen, erneut posten (standardmäßig werden sie übersprungen)
- `--gen_nfo` eine NFO-Datei neben die NZB schreiben; `--nfo_in_post` sie auch posten; `--nfo_password` das Passwort des Archivs hineinschreiben; `--nfo_text file.txt` einen Text am Ende anfügen; `--no_nfo_media` die Medieninformationen (MediaInfo) weglassen; `--mediainfo_path Pfad` das MediaInfo-Programm wählen
- `--notify_discord URL`, `--notify_telegram_token T` mit `--notify_telegram_chat ID`, `--notify_webhook URL` und `--notify_when each|all_done`: Benachrichtigungen am Ende der Posts (siehe [Fortschritt und Protokoll](06-progress-and-log.md))
- `--help` die vollständige Liste

Siehe [Auto Posting](08-auto-posting.md).
