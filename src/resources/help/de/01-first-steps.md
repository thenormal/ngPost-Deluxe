# Erste Schritte

Diese Seite führt Sie von der frischen Installation bis zu Ihrem ersten Post.

## Was Sie brauchen

- Ein Konto bei einem **Usenet-Anbieter** (Hostname, Port, Benutzername und Passwort).
- Die Dateien oder Ordner, die Sie posten möchten.
- Optional die Programme zum Komprimieren (**7-Zip** oder **RAR**) und zum Erstellen von par2-Dateien (**ParPar** oder **par2**). Das Windows-Installationsprogramm enthält 7-Zip und ParPar bereits.

## Ihr erster Post

Solange kein Server und keine Newsgroup eingestellt sind, zeigt Ihnen eine blaue Leiste oben in Quick Post und Auto Posting, was fehlt, mit einer Schaltfläche, die die richtige Seite öffnet. Sie verschwindet von selbst.

1. Öffnen Sie **Server**, fügen Sie Ihren Anbieter hinzu und klicken Sie auf **Verbindung testen**. Siehe [Server und Verbindungen](02-servers.md).
2. Öffnen Sie **Einstellungen**, Kategorie **Posts**, Karte **Newsgroups**, und fügen Sie mindestens eine Newsgroup hinzu. Siehe [Newsgroups](03-newsgroups.md).
3. Öffnen Sie **Quick Post** und klicken Sie auf **Dateien auswählen** (oder ziehen Sie Dateien und Ordner in das Fenster).
4. Wählen Sie die gewünschten Optionen (Komprimierung, par2, Passwort) und klicken Sie auf **Post Dateien**.
5. Verfolgen Sie den Fortschritt am unteren Fensterrand und im Protokoll. Am Ende wird die **NZB-Datei** gespeichert und im Protokoll angezeigt.

Die NZB-Datei ist die „Karte“ Ihres Posts: Ohne sie kann niemand die Artikel finden, besonders wenn Sie Verschleierung verwenden. Bewahren Sie sie gut auf.

## Wo was zu finden ist

- **Geführter Post**: Beantworten Sie ein paar Fragen und der Post wird für Sie vorbereitet. Siehe [Geführter Post](01b-guided-post.md).
- **Quick Post**: Dateien und Ordner sofort posten.
- **Auto Posting**: Ordner durchsuchen oder überwachen.
- **Server**: Ihre Usenet-Anbieter.
- **Einstellungen**: Standardoptionen des Programms, in fünf Kategorien (Allgemein, Posts, NZB-Dateien, Benachrichtigungen, Verlauf und Warteschlange).
- **Verlauf**: alles, was Sie gepostet haben, mit den [Statistiken](09b-statistics.md).
- **Hilfe**: dieser Bereich.

## Neue Versionen

Einmal am Tag, beim Start des Programms, fragt ngPost Deluxe bei GitHub nach, ob es eine neuere Version gibt (GitHub sieht nur Ihre IP-Adresse). Falls ja, erscheint unten in der linken Leiste eine Schaltfläche **Version X available**: Klicken Sie darauf, um die Seite der Version zu öffnen. Nichts wird von selbst heruntergeladen oder installiert, und Release Candidates werden nie angekündigt.

Zum Abschalten entfernen Sie das Häkchen bei **Beim Programmstart nach einer neuen Version suchen** unter **Einstellungen**, Kategorie **Allgemein**, Karte **Oberfläche**. Die Kommandozeilenversion und das Docker-Image prüfen nie.

## Computer wechseln oder eine Konfiguration teilen

Unter **Einstellungen**, Kategorie **Verlauf und Warteschlange**, Karte **Einstellungen exportieren und importieren**, schreibt **Einstellungen exportieren...** eine Textdatei mit den Einstellungen des Programms, den Profilen, den bevorzugten Newsgroups, den gewählten Benachrichtigungen und, wenn Sie möchten, den Servern. **Einstellungen importieren...** liest diese Datei auf einem anderen Computer und zeigt den Inhalt, bevor etwas geändert wird.

Die Datei enthält nie ein Passwort, einen Token, die Adresse einer Benachrichtigung, das feste Archivpasswort oder einen Ordner des Computers. Importierte Server bleiben ausgeschaltet, bis Sie ihr Passwort unter **Server** eingeben. Server, die Sie schon haben, werden nicht verändert, und ein Profil mit gleichem Namen bleibt erhalten, außer Sie wählen das Ersetzen. Die alte Konfiguration bleibt neben der neuen erhalten (`.before-import`), und ngPost startet neu, um die neuen Einstellungen zu verwenden.

Auf der Kommandozeile: `--export_settings Datei` und `--import_settings Datei` (danach beendet sich das Programm).

Weiter: [Server und Verbindungen](02-servers.md).
