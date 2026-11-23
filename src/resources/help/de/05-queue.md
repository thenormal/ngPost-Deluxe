# Warteschlange und Fortsetzen

## Die Warteschlange

Wenn Sie mehrere Releases posten, warten sie auf der Seite **Posts** auf ihren Zeitpunkt und werden nacheinander hochgeladen. Die Schaltfläche **Pause** unten im Fenster hält den laufenden Upload an und startet ihn wieder.

## Reihenfolge, Priorität und Pause

Auf der Seite **Posts** stehen die wartenden Posts in der Reihenfolge, in der sie starten, mit ihrem Platz: *Wartet (1)* ist der nächste. Wählen Sie einen Post und ändern Sie ihn mit den Schaltflächen oben oder mit einem Rechtsklick:

- **Priorität**: **Hoch**, **Normal** (Standard) oder **Niedrig**. Ein Post mit höherer Priorität kommt vor den Posts mit niedrigerer; den Post, der gerade hochgeladen wird, unterbricht er nicht.
- **Pfeile**: einen wartenden Post nach oben, nach unten, an den Anfang oder an das Ende der Warteschlange verschieben. Sie können ihn auch mit der Maus ziehen. Ein verschobener Post bleibt dort, wo Sie ihn hinsetzen, egal welche Priorität er hat.
- **Pause** / **Fortsetzen**: ein pausierter wartender Post bleibt in der Warteschlange und die nächsten Posts kommen vor ihm dran. Der Post, der gerade hochgeladen wird, hört auf zu senden und behält seinen Platz: die anderen Posts warten, bis Sie ihn fortsetzen, denn die Posts werden einer nach dem anderen hochgeladen. Wird er noch komprimiert, pausiert er, sobald sein Upload beginnt.
- **Pausiert hinzufügen**, neben **Post Dateien** in Quick Post und neben **Liste posten** in Auto Posting: die Posts kommen in die Warteschlange, ohne zu starten. Setzen Sie sie fort, wann Sie wollen.

Während ein Post hochgeladen wird, kann der nächste schon komprimiert werden (*Wird vorbereitet (1)*, dann *Bereit (1)*). Dieser Post bleibt an erster Stelle: er kann nicht pausiert oder verschoben werden und nichts kann vor ihn. Abbrechen können Sie ihn trotzdem.

## Gespeicherte Warteschlange

Standardmäßig werden Posts, die nicht fertig sind, gespeichert. Wenn Sie ngPost schließen oder der Computer ausgeht, zeigt beim nächsten Start ein Fenster, wie viele Posts nicht fertig waren:

- **Fortsetzen**: Sie werden neu gestartet. Ein Post, der gerade gesendet wurde, beginnt **von vorn**.
- **Verwerfen**: Sie werden entfernt.
- **Später entscheiden**: Sie bleiben gespeichert und werden beim nächsten Start erneut angeboten.

Die fortgesetzten Posts erscheinen auf der Seite **Posts**.

## Gut zu wissen

- Reihenfolge, Priorität und Pause jedes Posts werden mit der Warteschlange gespeichert: nach einem Neustart kommen die Posts so zurück, wie Sie sie verlassen haben. In der Kommandozeile werden auch die pausierten Posts gepostet.
- Die gespeicherte Warteschlange enthält auch das Passwort jedes Archivs im Klartext, wie der Verlauf. Sie können sie unter **Einstellungen**, Kategorie **Verlauf und Warteschlange**, Karte **Warteschlange der Posts** abschalten: Das löscht auch die gespeicherte Warteschlange.
- In der Kommandozeile verwenden Sie `--resume_queue`.
