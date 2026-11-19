# Avanzamento e log

## La barra di avanzamento

La barra in basso mostra la fase corrente:

- **Compressione**: creazione dell'archivio.
- **par2**: creazione dei file di recupero.
- **Upload**: invio degli articoli. Qui vedi anche la velocità, la dimensione, il numero di articoli e l'**ETA** (tempo stimato rimanente).
- **Completato**, **In pausa** o **Fermato**.

Il colore indica lo stato: blu durante il lavoro, grigio in pausa, verde a completamento, arancione se alcuni articoli non sono riusciti, rosso se il post si è fermato con un errore. La percentuale è mostrata anche nel titolo della finestra.

L'ETA è una stima basata sulla velocità recente: cambia quando cambia la velocità.

Se imposti un limite di velocità (**Impostazioni**, categoria **Post**, scheda **Upload**), dopo la velocità compare il limite, per esempio *(max 2.5 MB/s)*.

## Il log

Il log colora le righe: fasi, successi, avvisi ed errori si distinguono facilmente. Alla fine di un post mostra un riepilogo con il tempo di ogni fase (compressione, par2, upload) e il totale.

## Latenza

Alla fine di un post il log mostra anche la **latenza**: il tempo mediano di un giro completo con il server (il comando POST e la sua risposta, senza dati) e della connessione. Viene misurata solo da ciò che ngPost fa già, per questo non viene inviato nulla. Un valore alto (più di mezzo secondo) significa che il ritardo viene dalla rete, dalla distanza dal server o dal server stesso, per esempio una VPN, e non dal tuo computer.

## Report diagnostico

Se qualcosa non funziona, il pulsante **Report diagnostico**, sotto il log in Quick Post, salva un report di ciò che il programma ha fatto, con una sezione **Latenza** per ogni server (connessione, SSL, login, giro completo: minimo, mediana, 95° percentile e massimo) e una sezione **Notifiche** (cosa viene notificato, se la notifica di sistema è attiva e disponibile, quali canali sono impostati e l'ultimo errore; gli indirizzi e i token non vengono mai scritti). È un file di testo che puoi allegare quando chiedi aiuto. Controllalo prima di condividerlo: non dovrebbe contenere la tua password.

## Notifiche

ngPost può avvisarti quando i tuoi post sono finiti, così non devi guardare la finestra. Si imposta in **Impostazioni**, categoria **Notifiche**, in due parti: **cosa notificare** (un post terminato, un post con errori o fallito, un messaggio alla fine o uno per ogni post) e **dove inviare i messaggi**. Nel programma con finestra la notifica di sistema è attiva fin dall'inizio; gli altri canali richiedono un indirizzo.

- **Notifica di sistema**: un messaggio dall'icona nell'area di notifica (solo nella versione desktop; attiva per impostazione predefinita, puoi disattivarla). In Windows appare come qualsiasi altra notifica e resta nel centro notifiche; se non la vedi, controlla che le notifiche e l'assistente notifiche ("Non disturbare") di Windows non la nascondano.
- **Discord**: incolla l'indirizzo di un webhook di Discord.
- **Telegram**: il token del tuo bot e l'id della chat che riceve i messaggi.
- **Altro webhook**: qualsiasi indirizzo che accetti un POST JSON con un `title` e un `text` (strumenti tipo Slack, ntfy, Home Assistant).

Scegli cosa viene notificato (terminati, errori o fallimenti) e come: **un messaggio quando tutti i post sono finiti** (predefinito, ideale per Post automatico) oppure **un messaggio dopo ogni post**. Il pulsante **Invia un messaggio di prova** mostra subito se funziona.

Il messaggio contiene il nome del post, il risultato, la dimensione, il tempo e la velocità. Non contiene mai la password né i nomi dei file. Gli indirizzi e il token sono salvati in chiaro nel file di configurazione, come le altre password: tienilo privato. Se un messaggio non può essere inviato, una riga nel log lo segnala e il post non ne risente.

Da riga di comando, o in Docker, usa le opzioni `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` e `--notify_when each|all_done`, oppure le chiavi `notify_*` del file di configurazione.
