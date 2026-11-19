# Primi passi

Questa pagina ti accompagna dall'installazione al tuo primo post.

## Cosa ti serve

- Un account presso un **provider Usenet** (nome host, porta, nome utente e password).
- I file o le cartelle che vuoi postare.
- Facoltativamente, i programmi per comprimere (**7-Zip** o **RAR**) e per creare i file par2 (**ParPar** o **par2**). L'installer per Windows include già 7-Zip e ParPar.

## Il tuo primo post

Finché non sono impostati un server e un newsgroup, una barra blu in alto in Quick Post e in Post automatico ti dice cosa manca, con un pulsante che apre la pagina giusta. Scompare da sola.

1. Apri **Server**, aggiungi il tuo provider e premi **Prova connessione**. Vedi [Server e connessioni](02-servers.md).
2. Apri **Impostazioni**, categoria **Post**, scheda **Newsgroup**, e aggiungi almeno un newsgroup. Vedi [Newsgroup](03-newsgroups.md).
3. Apri **Quick Post**, premi **Seleziona file** (oppure trascina file e cartelle nella finestra).
4. Scegli le opzioni che vuoi (compressione, par2, password) e premi **Posta i file**.
5. Segui l'avanzamento in basso nella finestra e nel log. Alla fine il **file NZB** viene salvato e mostrato nel log.

Il file NZB è la "mappa" del tuo post: senza di esso nessuno può trovare gli articoli, soprattutto se usi l'offuscamento. Conservalo con cura.

## Dove si trovano le cose

- **Post guidato**: rispondi ad alcune domande e il post viene preparato per te. Vedi [Post guidato](01b-guided-post.md).
- **Quick Post**: posta subito file e cartelle.
- **Post automatico**: analizza o monitora le cartelle.
- **Server**: i tuoi provider Usenet.
- **Impostazioni**: le opzioni predefinite del programma, in cinque categorie (Generale, Post, File NZB, Notifiche, Cronologia e coda).
- **Cronologia**: tutto ciò che hai postato, con le sue [statistiche](09b-statistics.md).
- **Guida**: questa sezione.

## Nuove versioni

Una volta al giorno, all'avvio del programma, ngPost Deluxe chiede a GitHub se esiste una versione più recente (GitHub vede solo il tuo indirizzo IP). Se c'è, in fondo alla barra a sinistra compare il pulsante **Version X available**: cliccalo per aprire la pagina della release. Niente viene scaricato o installato da solo, e le release candidate non vengono mai segnalate.

Per disattivarlo, togli la spunta a **Cerca una nuova versione all'avvio del programma** in **Impostazioni**, categoria **Generale**, scheda **Interfaccia**. La versione da riga di comando e l'immagine Docker non fanno mai questo controllo.

Avanti: [Server e connessioni](02-servers.md).
