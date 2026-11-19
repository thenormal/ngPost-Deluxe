# Riga di comando

Il programma può funzionare anche senza finestra, per un server, un NAS o uno script. La versione da riga di comando è lo stesso programma compilato senza la finestra (oppure l'immagine Docker).

## Esempi

Posta due elementi con compressione, nome e password casuali e par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Posta ogni elemento di una cartella, un post ciascuno:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Controlla una cartella e sposta i file in una cartella "Posted" una volta postati:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` accetta un nome come `Posted` (la cartella viene creata accanto a ogni file o cartella) oppure un percorso completo. Usa invece `--rm_posted` per eliminare i file una volta postati. Entrambe si applicano solo quando tutto è stato postato senza errori.

## Opzioni utili

- `-c` file di configurazione (altrimenti `$HOME/.ngPost`)
- `-g` newsgroup, separati da virgole
- `-o` file NZB di output
- `-S` server in una sola stringa
- `--disp_progress` BAR o FILES
- `--resume_queue` posta di nuovo i post non terminati
- `--allow_repost` posta di nuovo i file che sono già nella cronologia (per impostazione predefinita vengono saltati)
- `--max_speed MB/s` limita la velocità di upload, per tutte le connessioni insieme (0: nessun limite); equivale alle chiavi `limit_speed` e `max_speed` del file di configurazione
- `--gen_nfo` scrive un file NFO accanto all'nzb; `--nfo_in_post` lo posta anche; `--nfo_password` vi scrive la password dell'archivio; `--nfo_text file.txt` aggiunge un testo alla fine; `--no_nfo_media` esclude le informazioni multimediali (MediaInfo); `--mediainfo_path percorso` sceglie il programma MediaInfo
- `--notify_discord URL`, `--notify_telegram_token T` con `--notify_telegram_chat ID`, `--notify_webhook URL` e `--notify_when each|all_done`: notifiche alla fine dei post (vedi [Avanzamento e log](06-progress-and-log.md))
- `--help` l'elenco completo

Vedi [Post automatico](08-auto-posting.md).
