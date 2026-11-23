# Quick Post

Quick Post bereitet einen Post vor: die Dateien und Ordner der Liste werden zu einem Release. Wenn Sie auf **Post Dateien** klicken, kommt der Post auf die Seite **Posts** und Quick Post ist wieder leer, bereit für das nächste Release.

## Schritte

1. Fügen Sie Dateien und Ordner mit **Dateien auswählen** oder per Ziehen hinzu.
2. Wählen Sie die Optionen (unten).
3. Klicken Sie auf **Post Dateien**.

## Optionen

- **Komprimieren**: packt die Dateien vor dem Posten in ein RAR- oder 7z-Archiv. Sie können es in Teile (Volumes) aufteilen.
- **Passwort**: Das Archiv kann ein Passwort haben, fest oder zufällig erzeugt. Das Passwort wird mit dem Post gespeichert (zum Beispiel in den NZB-Metadaten und im Verlauf).
- **Zufälliger Name**: gibt dem Archiv einen zufälligen Namen.
- **par2**: erstellt Wiederherstellungsdateien. Stellen Sie die Redundanz in Prozent ein: Mehr Redundanz lässt den Downloader mehr fehlende Artikel reparieren, lädt aber mehr Daten hoch.
- **Verschleierung**: versteckt die Betreffzeilen der Artikel. Ohne die NZB-Datei kann niemand Ihren Post finden, nicht einmal Sie.

## Prüfungen und Zusammenfassung

Unter den Karten zeigt eine Zeile, was gepostet wird: die Anzahl der Elemente, ihre Größe und, mit Komprimierung oder par2, ungefähr wie viel hochgeladen wird. Die Upload-Dauer erscheint nach dem ersten Post der Sitzung, berechnet aus der tatsächlichen Geschwindigkeit dieses Uploads (vorher rät ngPost nicht).

Wenn Sie auf **Post Dateien** klicken, prüft ngPost den Post zuerst. Stimmt etwas nicht, erklärt ein Fenster es in einfachen Worten, mit einer Schaltfläche, die das Problem behebt:

- **Fehler** (der Post kann nicht starten): kein aktivierter Server, keine gültige Newsgroup, Ordner ohne Komprimierung, rar- / 7z- oder par2-Programm nicht gefunden, ein aktiviertes, aber leeres Passwort, ein nicht beschreibbarer NZB-Ordner.
- **Warnungen** (Sie können **Trotzdem posten** wählen): Der temporäre Ordner fehlt oder ist möglicherweise zu klein, MediaInfo für die Medieninformationen der NFO wurde nicht gefunden.

Nach einer Korrektur werden die Prüfungen erneut ausgeführt, sodass ein neues Problem sofort auftaucht statt mitten im Post. Auto Posting hat dieses Fenster noch nicht.

## Profile

Das Menü **Profil** über den Karten stellt alle Optionen auf einmal ein.

- **Empfohlen**: Komprimierung, zufälliger Name und Passwort, par2 mit 10 % Redundanz. Das ist die Wahl einer Neuinstallation.
- **Benutzerdefiniert**: wird angezeigt, wenn Sie eine Option von Hand ändern.
- **Ihre eigenen Profile**: Stellen Sie die Optionen ein, klicken Sie auf **Als Profil speichern...** und geben Sie einen Namen ein. **Löschen** entfernt das gewählte Profil. Die Profile werden in der Konfigurationsdatei gespeichert (Schlüssel `profile`) und sind in Quick Post und Auto Posting dieselben.

Auch die Karte **NFO-Datei** (siehe [NZB-Dateien](09-nzb-files.md)) wird gemeinsam genutzt: Quick Post und Auto Posting zeigen dieselben Einstellungen.

## Nach Post Dateien

Der Post kommt in die Warteschlange und das Programm zeigt die Seite **Posts**, wo Sie seinen Fortschritt und sein Protokoll verfolgen. Quick Post behält Ihre Optionen und erzeugt einen neuen zufälligen Namen und ein neues Passwort, sodass Sie sofort das nächste Release vorbereiten können.

Siehe auch [Fortschritt und Protokoll](06-progress-and-log.md).
