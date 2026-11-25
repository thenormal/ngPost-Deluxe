# Post automatico

Post automatico posta tanti file e cartelle senza preparare ciascuno a mano. Scegli uno dei tre pulsanti in alto nella pagina; vengono mostrate solo le impostazioni che servono.

## Posta ciò che scelgo

Aggiungi i file e le cartelle che vuoi (trascinali nella lista, oppure usa **Seleziona file** / **Seleziona una cartella**), poi premi **Posta la lista**: ogni file o cartella diventa un post separato. Con **Aggiungi in pausa** spuntato (anche per **Scansiona una cartella una volta**), i post aspettano nella pagina **Post** finché non li riprendi.

## Analizza una cartella una volta

Scegli una cartella e premi **Analizza**: ciò che contiene viene elencato. Le cartelle richiedono la **compressione**. Usa **prima i file più recenti** per ordinare per data.

## Controlla una cartella e posta ciò che arriva

Scegli la cartella e premi **Avvia il controllo**: ogni nuovo file o cartella viene postato quando arriva. Premi **Ferma il controllo** per fermarti. Fermarsi con dei post ancora in attesa chiede una conferma.

Durante il controllo, una barra sotto le tre scelte mostra sempre la cartella controllata, quanti post sono finiti oggi (e quanti con errori) e quanti sono in corso o in attesa. **Metti in pausa** fa aspettare i nuovi file: vengono postati quando premi **Riprendi il monitoraggio**; i post già avviati continuano. Fermare il controllo dimentica i file che erano in attesa.

Le cartelle controllate vengono ricordate. Se il programma è stato chiuso durante il controllo, al successivo avvio chiede se **Controllarle di nuovo** (con le opzioni della pagina), **Dimenticarle** o **Decidere dopo**; le cartelle che non esistono più vengono escluse. L'elenco non viene esportato con le impostazioni, perché le cartelle appartengono al computer.

## La lista

Ogni elemento ha una casella e la sua dimensione. Togli la spunta a un elemento per escluderlo; vengono postati solo gli elementi selezionati. Per rimuovere elementi dalla lista, selezionali e premi CANC. La riga sotto la lista mostra quanti elementi sono selezionati, la loro dimensione, la dimensione da caricare e, dopo un primo post, il tempo di upload.

## Elementi già postati

Se alcuni elementi selezionati sono già nella cronologia (stesso nome e stessa dimensione), una finestra li elenca con la data del post precedente. Spunta **Posta di nuovo** per quelli che vuoi postare ancora una volta; gli altri non vengono postati e restano nella lista, senza spunta e in grigio. Se esiste già un file NZB con lo stesso nome, il nuovo si chiama `nome_1.nzb` e così via. Quando controlli una cartella, gli elementi già nella cronologia vengono saltati con una riga nel log. Puoi disattivare l'avviso in **Impostazioni** (Cronologia).

## Prima di cominciare

Quando premi **Posta la lista** o **Avvia il controllo**, il programma controlla le impostazioni una sola volta per tutto il gruppo (un server e un newsgroup, la compressione per le cartelle, i programmi rar / 7z e par2, la cartella dell'NZB, la cartella temporanea). I problemi sono mostrati in parole semplici con un pulsante che li risolve; gli avvisi si possono ignorare con **Pubblica comunque**. È la stessa finestra di [Quick Post](04-quick-post.md).

## Nome, newsgroup e server

La scheda **Nome, newsgroup e server** funziona come le righe di [Quick Post](04-quick-post.md), con una riga in più: **Cartella NZB**, dove vengono scritti i file NZB di questi post (vuota: la cartella delle Impostazioni). Vale anche per le cartelle monitorate. Un profilo può ricordare queste impostazioni.

## Consigli

- Usa compressione, nomi casuali, password casuali e par2: è la configurazione tipica per la pubblicazione automatica. Il profilo **Consigliato** (menu **Profilo**) la imposta con un clic.
- **Dopo il post** (la riga sopra il pulsante Posta): non fare nulla (predefinito), spostare i file in una **cartella "Posted"**, oppure eliminarli. Il nome predefinito `Posted` crea quella cartella accanto a ogni file o cartella; puoi anche scegliere un'altra cartella. Niente viene spostato o eliminato se il post non è finito senza errori. Un nome già usato nella cartella non viene mai sovrascritto: il file diventa `nome (2)`. L'analisi e il controllo delle cartelle ignorano la cartella "Posted". L'eliminazione è definitiva: usala solo quando sei sicuro.
- Da riga di comando la stessa scelta è `--move_posted` o `--rm_posted` (vedi [Riga di comando](10-command-line.md)).
- Il programma ricorda l'ultimo pulsante usato.
- Per un NAS o un server senza schermo, la riga di comando e l'immagine Docker fanno lo stesso lavoro.

Vedi [Riga di comando](10-command-line.md).
