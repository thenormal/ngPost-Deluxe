# Fortschritt und Protokoll

## Die Fortschrittsleiste

Die Leiste unten zeigt die aktuelle Phase:

- **Komprimierung**: Das Archiv wird erstellt.
- **par2**: Die Wiederherstellungsdateien werden erstellt.
- **Upload**: Die Artikel werden gesendet. Hier sehen Sie auch die Geschwindigkeit, die Größe, die Anzahl der Artikel und die **ETA** (geschätzte Restzeit).
- **Abgeschlossen**, **Pausiert** oder **Gestoppt**.

Die Farbe zeigt den Zustand: blau während der Arbeit, grau bei Pause, grün bei Abschluss, orange, wenn einige Artikel fehlgeschlagen sind, rot, wenn der Post mit einem Fehler gestoppt wurde. Der Prozentwert steht auch im Fenstertitel.

Die ETA ist eine Schätzung auf Basis der letzten Geschwindigkeit: Sie ändert sich, wenn sich die Geschwindigkeit ändert.

Wenn Sie eine Geschwindigkeitsbegrenzung einstellen (**Einstellungen**, Kategorie **Posts**, Karte **Upload**), steht hinter der Geschwindigkeit die Grenze, zum Beispiel *(max. 2.5 MB/s)*.

## Das Protokoll

Das Protokoll färbt die Zeilen: Phasen, Erfolge, Warnungen und Fehler lassen sich leicht unterscheiden. Am Ende eines Posts zeigt es eine Zusammenfassung mit der Zeit jeder Phase (Komprimierung, par2, Upload) und der Gesamtzeit.

Jeder Post hat außerdem sein eigenes Protokoll, mit der Uhrzeit jeder Zeile: wählen Sie den Post auf der Seite **Posts**. Diese Seite listet alle Posts der Sitzung mit Fortschritt, Größe, Geschwindigkeit und verbleibender Zeit; dort können Sie einen Post stoppen, einen wartenden abbrechen, den Ordner der NZB-Datei öffnen oder das Passwort kopieren.

Auf der Seite **Posts** lässt sich das Log filtern: Geben Sie ein Wort in das Suchfeld ein, um nur die Zeilen mit diesem Wort zu sehen, und setzen oder entfernen Sie die Häkchen der Kategorien (Info, Phasen, OK, Warnungen, Debug, Fehler). Die Debug-Zeilen sind zunächst ausgeblendet. Die Schaltfläche **Kopieren** kopiert die angezeigten Zeilen. Unter den Details des Posts zeigt ein Diagramm die **Upload-Geschwindigkeit** Sekunde für Sekunde (Pausen werden nicht gezählt), mit der letzten, der besten und der durchschnittlichen Geschwindigkeit. Das Diagramm bleibt nur bis zum Schließen des Programms erhalten; ein Post einer früheren Sitzung hat daher nur seine Durchschnittsgeschwindigkeit.

## Latenz

Am Ende eines Posts zeigt das Protokoll auch die **Latenz**: die mittlere Zeit eines Hin und Zurück mit dem Server (der Befehl POST und seine Antwort, ohne Daten) und der Verbindung. Sie wird nur aus dem gemessen, was ngPost ohnehin tut, dafür wird nichts gesendet. Ein hoher Wert (mehr als eine halbe Sekunde) bedeutet, dass die Verzögerung vom Netzwerk, von der Entfernung zum Server oder vom Server selbst kommt, zum Beispiel durch ein VPN, und nicht von Ihrem Computer.

## Diagnosebericht

Wenn etwas nicht funktioniert, speichert die Schaltfläche **Diagnosebericht** unter dem Protokoll in Quick Post einen Bericht darüber, was das Programm getan hat, mit einem Abschnitt **Latenz** für jeden Server (Verbindung, SSL, Anmeldung, Hin und Zurück: Minimum, Median, 95. Perzentil und Maximum) und einem Abschnitt **Benachrichtigungen** (was gemeldet wird, ob die Systembenachrichtigung aktiv und verfügbar ist, welche Kanäle eingestellt sind und der letzte Fehler; die Adressen und Tokens werden nie geschrieben). Es ist eine Textdatei, die Sie anhängen können, wenn Sie um Hilfe bitten. Prüfen Sie sie vor dem Weitergeben: Sie sollte Ihr Passwort nicht enthalten.

## Benachrichtigungen

ngPost kann Ihnen mitteilen, wenn Ihre Posts fertig sind, damit Sie das Fenster nicht beobachten müssen. Das wird unter **Einstellungen**, Kategorie **Benachrichtigungen**, in zwei Teilen eingestellt: **was gemeldet wird** (ein Post ist fertig, ein Post mit Fehlern oder fehlgeschlagen, eine Nachricht am Ende oder eine pro Post) und **wohin die Nachrichten gesendet werden**. Im Programm mit Fenster ist die Systembenachrichtigung von Anfang an aktiv; die anderen Kanäle brauchen eine Adresse.

- **Systembenachrichtigung**: eine Nachricht vom Symbol im Infobereich (nur Desktop-Version; standardmäßig aktiv, Sie können sie abschalten). Unter Windows erscheint sie wie jede andere Benachrichtigung und bleibt im Benachrichtigungscenter; wenn Sie sie nicht sehen, prüfen Sie, dass die Benachrichtigungen und der Fokusassistent („Nicht stören“) von Windows sie nicht verbergen.
- **Discord**: Fügen Sie die Adresse eines Discord-Webhooks ein.
- **Telegram**: das Token Ihres Bots und die ID des Chats, der die Nachrichten erhält.
- **Anderer Webhook**: jede Adresse, die einen JSON-POST mit `title` und `text` akzeptiert (Slack-ähnliche Werkzeuge, ntfy, Home Assistant).

Wählen Sie, was gemeldet wird (fertig, Fehler oder Fehlschläge) und wie: **eine Nachricht, wenn alle Posts fertig sind** (Standard, am besten für Auto Posting) oder **eine Nachricht nach jedem Post**. Die Schaltfläche **Testnachricht senden** zeigt sofort, ob es funktioniert.

Die Nachricht enthält den Namen des Posts, das Ergebnis, die Größe, die Zeit und die Geschwindigkeit. Sie enthält nie das Passwort oder die Dateinamen. Die Adressen und das Token werden wie die anderen Passwörter im Klartext in der Konfigurationsdatei gespeichert: Halten Sie sie privat. Kann eine Nachricht nicht gesendet werden, sagt das eine Zeile im Protokoll, und der Post ist nicht betroffen.

In der Kommandozeile oder in Docker verwenden Sie die Optionen `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` und `--notify_when each|all_done` oder die Schlüssel `notify_*` der Konfigurationsdatei.
