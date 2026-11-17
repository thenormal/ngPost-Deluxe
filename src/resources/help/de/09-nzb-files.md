# NZB-Dateien

Eine NZB-Datei listet die Artikel eines Posts auf. Jeder, der sie hat, kann den Post aus dem Usenet herunterladen.

## Wo sie gespeichert wird

Der Ordner wird unter **Einstellungen**, Kategorie **NZB-Dateien** festgelegt. Die Datei erhält den Namen des Posts (die erste Datei oder der erste Ordner).

## Verwendung

- Geben Sie sie zusammen mit dem Passwort des Archivs weiter, falls Sie eines festgelegt haben.
- Ein Download-Programm (ein Newsreader) öffnet sie und lädt die Dateien herunter.
- Mit Verschleierung ist die NZB der **einzige** Weg, Ihren Post zu finden: Bewahren Sie eine Kopie auf.

## Eine NZB prüfen

In der Kommandozeile prüft `--check`, ob die Artikel einer NZB noch auf dem Server verfügbar sind.

## Verlauf

Die Seite **Verlauf** listet Ihre früheren Uploads auf (Datum, Name, Größe, Geschwindigkeit, Archiv, Passwort, Gruppen). Sie können den Verlauf in den Einstellungen abschalten. Das Passwort wird im Klartext gespeichert.

### Posts, die schon im Verlauf stehen

Bevor ein Post startet, sucht ngPost im Verlauf nach einem erfolgreichen Post mit demselben NZB-Namen und derselben Gesamtgröße der Dateien. In **Quick Post** fragt ein Fenster, ob er erneut gepostet werden soll (**Trotzdem posten** oder **Abbrechen**). In **Auto Posting** und in der Kommandozeile wird der Post übersprungen und eine Zeile im Protokoll sagt das. Um ihn dennoch erneut zu posten, verwenden Sie die Option `--allow_repost` oder schalten Sie die Warnung unter **Einstellungen**, Kategorie **Verlauf und Warteschlange** (Karte Verlauf der Uploads) ab. Die Prüfung setzt voraus, dass der Verlauf aktiv ist, und sie betrachtet nur Name und Größe: Ein umbenannter Ordner wird nicht erkannt.

## NFO-Datei

Eine NFO ist eine kleine Textdatei, die den Post beschreibt: seinen Namen, das Datum, die Dateien mit ihren Größen, wie er vorbereitet wurde (Archiv und par2) und einen Text von Ihnen. Aktivieren Sie sie in der Karte **NFO-Datei** von Quick Post oder Auto Posting (es ist dieselbe Karte: Die Einstellungen werden gemeinsam genutzt).

- **NFO-Datei neben die NZB schreiben**: `name.nfo` wird im selben Ordner wie `name.nzb` gespeichert.
- **NFO-Datei auch mit den Dateien posten**: Sie wird mit dem Post hochgeladen und erscheint in der NZB.
- **Medieninformationen einbeziehen (MediaInfo)**: standardmäßig aktiv. Für Video- und Audiodateien listet die NFO die Dauer, das Video (Format, Größe, Bildrate, HDR), die Audiospuren (Sprache, Format, Kanäle, Bitrate) und die Untertitel auf. Höchstens 20 Dateien werden beschrieben (die übrigen werden gezählt) und das Lesen stoppt nach 20 Sekunden. Es nutzt das kostenlose Programm MediaInfo: Es ist im Windows-Installationsprogramm und im Docker-Image enthalten; unter Linux und macOS installieren Sie das Paket `mediainfo` oder wählen das Programm in den Einstellungen. Ohne MediaInfo hat die NFO einfach keinen Medienabschnitt.
- **Passwort des Archivs schreiben**: standardmäßig aus. In einer geposteten NFO kann das Passwort von jedem gelesen werden, der den Post herunterlädt.
- **Am Ende angefügter Text**: Notizen, Links, alles, was Sie sagen möchten. Derselbe Text wird für alle Posts verwendet.

Wenn die Dateinamen verborgen sind (Verschleierung der Artikel oder der Dateinamen), listet die geposte NFO keine Dateinamen und keine Medieninformationen auf, und die Datei selbst erhält einen zufälligen Namen. In der Kommandozeile verwenden Sie `--gen_nfo`, `--nfo_in_post`, `--nfo_password` und `--nfo_text file.txt`, dazu `--no_nfo_media` und `--mediainfo_path Pfad`.
