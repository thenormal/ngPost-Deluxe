# Quick Post

Quick Post prepara un post: i file e le cartelle della lista diventano una release. Quando premi **Posta i file**, il post passa nella pagina **Post** e Quick Post torna vuoto, pronto per la release successiva.

## Passaggi

1. Aggiungi file e cartelle con **Seleziona file** o trascinandoli.
2. Scegli le opzioni (qui sotto).
3. Premi **Posta i file**.

## Opzioni

- **Comprimi**: mette i file in un archivio RAR o 7z prima di postare. Puoi dividerlo in volumi.
- **Password**: l'archivio può avere una password, fissa o generata a caso. La password viene salvata con il post (per esempio nei metadati dell'NZB e nella Cronologia).
- **Nome casuale**: dà all'archivio un nome casuale.
- **par2**: crea i file di recupero. Imposta la ridondanza in percentuale: più ridondanza permette al downloader di riparare più articoli mancanti, ma carica più dati.
- **Offuscamento**: nasconde gli oggetti (subject) degli articoli. Senza il file NZB nessuno può trovare il tuo post, nemmeno tu.

## Controlli e riepilogo

Sotto le schede una riga ti dice cosa sta per essere postato: il numero di elementi, la loro dimensione e, con compressione o par2, circa quanto verrà caricato. Il tempo di upload compare dopo il primo post della sessione, calcolato dalla velocità reale di quell'upload (prima, ngPost non tira a indovinare).

Quando premi **Posta i file**, ngPost controlla prima il post. Se qualcosa non va, una finestra lo spiega in parole semplici, con un pulsante che lo risolve:

- **Errori** (il post non può partire): nessun server abilitato, nessun newsgroup valido, cartelle senza compressione, programma rar / 7z o par2 non trovato, una password attivata ma vuota, una cartella NZB non scrivibile.
- **Avvisi** (puoi scegliere **Pubblica comunque**): la cartella temporanea manca o potrebbe essere troppo piccola, MediaInfo non è stato trovato per le informazioni multimediali dell'NFO.

Dopo una correzione i controlli ripartono, così un nuovo problema salta fuori subito invece che a metà del post. Post automatico non ha ancora questa finestra.

## Profili

Il menu **Profilo** sopra le schede imposta tutte le opzioni in una volta.

- **Consigliato**: compressione, nome e password casuali, par2 con ridondanza del 10 %. È la scelta di una nuova installazione.
- **Personalizzato**: compare quando cambi un'opzione a mano.
- **I tuoi profili**: imposta le opzioni, premi **Salva come profilo...** e dagli un nome. **Elimina** rimuove il profilo scelto. I profili sono salvati nel file di configurazione (chiave `profile`) e sono gli stessi in Quick Post e in Post automatico.

Anche la scheda **File NFO** (vedi [File NZB](09-nzb-files.md)) è condivisa: Quick Post e Post automatico mostrano le stesse impostazioni.

## Dopo Posta i file

Il post entra nella coda e il programma mostra la pagina **Post**, dove segui il suo avanzamento e il suo log. Quick Post conserva le tue opzioni e crea un nuovo nome e una nuova password casuali, così puoi preparare subito la release successiva.

Vedi anche [Avanzamento e log](06-progress-and-log.md).
