# Fehlgeschlagene Artikel und erneutes Posten

Bei einem Upload können einige Artikel fehlschlagen (eine Verbindung bricht ab, der Server lehnt sie ab). ngPost versucht jeden Artikel mehrmals. Schlagen einige weiterhin fehl, endet der Post in Orange und das Protokoll sagt, wie viele Artikel fehlgeschlagen sind.

## Fehlende Artikel erneut posten

Wenn Artikel fehlgeschlagen sind, zeigt der Tab eine Schaltfläche **Fehlende Artikel neu posten**. Sie postet **nur diese Artikel** erneut und korrigiert die NZB-Datei. Sie müssen nicht den ganzen Post noch einmal senden.

Nach einem erfolgreichen erneuten Posten wird die Dateiliste aktualisiert und der Post wird grün.

## Tipps

- Wenn viele Artikel fehlschlagen, liegt die Ursache meist an der Verbindung. Verringern Sie die Anzahl der Verbindungen oder prüfen Sie die Limits des Anbieters.
- Wenn immer wieder dieselben Artikel fehlschlagen, prüfen Sie Ihr Konto und die Newsgroups.
- Die Anzahl der Wiederholungsversuche lässt sich mit der Option `--retry` (Standard 5) oder in der Konfigurationsdatei einstellen.

Siehe [Server und Verbindungen](02-servers.md).
