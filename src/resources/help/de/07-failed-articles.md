# Fehlgeschlagene Artikel und erneutes Posten

Bei einem Upload können einige Artikel fehlschlagen (eine Verbindung bricht ab, der Server lehnt sie ab). ngPost versucht jeden Artikel mehrmals. Schlagen einige weiterhin fehl, endet der Post in Orange und das Protokoll sagt, wie viele Artikel fehlgeschlagen sind.

## Fehlende Artikel erneut posten

Wenn Artikel fehlgeschlagen sind, wählen Sie den Post auf der Seite **Posts**: die Schaltfläche **Fehlende Artikel erneut posten** postet **nur diese Artikel** erneut und korrigiert die NZB-Datei. Sie müssen nicht den ganzen Post noch einmal senden.

Das erneute Posten erscheint als neue Zeile in der Liste; endet es ohne Fehler, fehlt nichts mehr.

## Tipps

- Wenn viele Artikel fehlschlagen, liegt die Ursache meist an der Verbindung. Verringern Sie die Anzahl der Verbindungen oder prüfen Sie die Limits des Anbieters.
- Wenn immer wieder dieselben Artikel fehlschlagen, prüfen Sie Ihr Konto und die Newsgroups.
- Die Anzahl der Wiederholungsversuche lässt sich mit der Option `--retry` (Standard 5) oder in der Konfigurationsdatei einstellen.

Siehe [Server und Verbindungen](02-servers.md).
