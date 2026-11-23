# Auto Posting

Auto Posting postet viele Dateien und Ordner, ohne jeden einzeln von Hand vorzubereiten. Wählen Sie eine der drei Schaltflächen oben auf der Seite; es werden nur die Einstellungen angezeigt, die Sie brauchen.

## Posten, was ich auswähle

Fügen Sie die gewünschten Dateien und Ordner hinzu (ziehen Sie sie in die Liste oder verwenden Sie **Dateien auswählen** / **Ordner auswählen**) und klicken Sie auf **Liste posten**: Jede Datei und jeder Ordner wird ein eigener Post. Mit **Pausiert hinzufügen** (auch für **Ordner einmal scannen**) warten die Posts auf der Seite **Posts**, bis Sie sie fortsetzen.

## Einen Ordner einmal durchsuchen

Wählen Sie einen Ordner und klicken Sie auf **Durchsuchen**: Sein Inhalt wird aufgelistet. Ordner brauchen **Komprimierung**. Mit **neueste Dateien zuerst** sortieren Sie nach Datum.

## Einen Ordner überwachen und posten, was ankommt

Wählen Sie den Ordner und klicken Sie auf **Überwachung starten**: Jede neue Datei und jeder neue Ordner wird gepostet, sobald sie ankommen. Klicken Sie auf **Überwachung beenden**, um zu stoppen. Beenden bei noch wartenden Posts fragt nach einer Bestätigung.

Während der Überwachung zeigt eine Leiste unter den drei Auswahlmöglichkeiten immer den überwachten Ordner, wie viele Posts heute fertig wurden (und wie viele mit Fehlern) und wie viele laufen oder warten. **Überwachung pausieren** lässt neue Dateien warten: Sie werden gepostet, wenn Sie auf **Überwachung fortsetzen** klicken; bereits gestartete Posts laufen weiter. Beim Beenden der Überwachung werden die wartenden Dateien vergessen.

## Die Liste

Jedes Element hat ein Kästchen und seine Größe. Entfernen Sie das Häkchen eines Elements, um es auszulassen; nur markierte Elemente werden gepostet. Um Elemente aus der Liste zu entfernen, wählen Sie sie aus und drücken Sie ENTF. Die Zeile unter der Liste zeigt, wie viele Elemente ausgewählt sind, ihre Größe, die hochzuladende Größe und, nach einem ersten Post, die Upload-Dauer.

## Bereits geposte Elemente

Wenn einige markierte Elemente schon im Verlauf stehen (gleicher Name und gleiche Größe), listet sie ein Fenster mit dem Datum des früheren Posts auf. Setzen Sie **Erneut posten** bei denen, die Sie noch einmal posten möchten; die anderen werden nicht gepostet und bleiben in der Liste, unmarkiert und grau. Existiert bereits eine NZB-Datei mit demselben Namen, heißt die neue `name_1.nzb` und so weiter. Bei der Überwachung eines Ordners werden Elemente, die schon im Verlauf stehen, mit einer Zeile im Protokoll übersprungen. Sie können die Warnung unter **Einstellungen** (Verlauf) abschalten.

## Bevor es losgeht

Wenn Sie auf **Liste posten** oder **Überwachung starten** klicken, prüft das Programm die Einstellungen einmal für die ganze Gruppe (ein Server und eine Newsgroup, Komprimierung für Ordner, die Programme rar / 7z und par2, der NZB-Ordner, der temporäre Ordner). Probleme werden in einfachen Worten mit einer Schaltfläche angezeigt, die sie behebt; Warnungen können mit **Trotzdem posten** ignoriert werden. Es ist dasselbe Fenster wie in [Quick Post](04-quick-post.md).

## Tipps

- Verwenden Sie Komprimierung, zufällige Namen, zufällige Passwörter und par2: Das ist die übliche Einrichtung für automatisches Posten. Das Profil **Empfohlen** (Menü **Profil**) stellt sie mit einem Klick ein.
- **Nach dem Posten** (die Zeile über der Schaltfläche Posten): nichts tun (Standard), die Dateien in einen **Ordner „Posted“** verschieben oder löschen. Der Standardname `Posted` legt diesen Ordner neben jeder Datei oder jedem Ordner an; Sie können auch einen anderen Ordner wählen. Es wird nichts verschoben oder gelöscht, wenn der Post nicht fehlerfrei beendet wurde. Ein im Ordner bereits verwendeter Name wird nie überschrieben: Die Datei wird zu `name (2)`. Das Durchsuchen und die Ordnerüberwachung ignorieren den Ordner „Posted“. Löschen ist endgültig: Verwenden Sie es nur, wenn Sie sicher sind.
- In der Kommandozeile lautet dieselbe Wahl `--move_posted` oder `--rm_posted` (siehe [Kommandozeile](10-command-line.md)).
- Das Programm merkt sich die zuletzt benutzte Schaltfläche.
- Für ein NAS oder einen Server ohne Bildschirm erledigen die Kommandozeile und das Docker-Image dieselbe Arbeit.

Siehe [Kommandozeile](10-command-line.md).
