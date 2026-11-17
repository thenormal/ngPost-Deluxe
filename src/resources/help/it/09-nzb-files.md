# File NZB

Un file NZB elenca gli articoli di un post. Chiunque lo abbia può scaricare il post da Usenet.

## Dove viene salvato

La cartella si imposta in **Impostazioni**, categoria **File NZB**. Il file prende il nome del post (il primo file o la prima cartella).

## Come usarlo

- Condividilo insieme alla password dell'archivio, se ne hai impostata una.
- Un programma di download (un newsreader) lo apre e scarica i file.
- Con l'offuscamento l'NZB è l'**unico** modo per trovare il tuo post: conservane una copia.

## Controllare un NZB

Da riga di comando, `--check` verifica che gli articoli di un NZB siano ancora disponibili sul server.

## Cronologia

La pagina **Cronologia** elenca i tuoi upload passati (data, nome, dimensione, velocità, archivio, password, gruppi). Puoi disattivare la cronologia nelle Impostazioni. La password è salvata in chiaro.

### Post già presenti nella cronologia

Prima che un post parta, ngPost cerca nella cronologia un post riuscito con lo stesso nome NZB e la stessa dimensione totale dei file. In **Quick Post** una finestra chiede se postarlo di nuovo (**Pubblica comunque** o **Annulla**). In **Auto Posting** e da riga di comando il post viene saltato e una riga nel log lo segnala. Per postarlo comunque, usa l'opzione `--allow_repost` oppure disattiva l'avviso in **Impostazioni**, categoria **Cronologia e coda** (scheda Cronologia degli upload). Il controllo richiede che la cronologia sia attiva e guarda solo il nome e la dimensione: una cartella rinominata non viene riconosciuta.

## File NFO

Un NFO è un piccolo file di testo che descrive il post: il nome, la data, i file con le loro dimensioni, come è stato preparato (archivio e par2) e un testo tuo. Attivalo nella scheda **File NFO** di Quick Post o di Auto Posting (è la stessa scheda: le impostazioni sono condivise).

- **Scrivi un file NFO accanto all'nzb**: `nome.nfo` viene salvato nella stessa cartella di `nome.nzb`.
- **Posta il file NFO insieme ai file**: viene caricato con il post e compare nell'NZB.
- **Includi le informazioni multimediali (MediaInfo)**: attivo per impostazione predefinita. Per i file video e audio l'NFO elenca la durata, il video (formato, dimensioni, frame rate, HDR), le tracce audio (lingua, formato, canali, bit rate) e i sottotitoli. Vengono descritti al massimo 20 file (gli altri vengono contati) e la lettura si ferma dopo 20 secondi. Usa il programma gratuito MediaInfo: è incluso nell'installer per Windows e nell'immagine Docker; su Linux e macOS installa il pacchetto `mediainfo`, oppure scegli il programma nelle Impostazioni. Senza MediaInfo l'NFO semplicemente non ha la sezione multimediale.
- **Scrivi la password dell'archivio**: disattivato per impostazione predefinita. In un NFO postato la password può essere letta da chiunque scarichi il post.
- **Testo aggiunto alla fine**: note, link, qualsiasi cosa tu voglia dire. Lo stesso testo viene usato per tutti i post.

Quando i nomi dei file sono nascosti (offuscamento degli articoli o dei nomi dei file), l'NFO che viene postato non elenca alcun nome di file né informazioni multimediali, e il file stesso riceve un nome casuale. Da riga di comando usa `--gen_nfo`, `--nfo_in_post`, `--nfo_password` e `--nfo_text file.txt`, più `--no_nfo_media` e `--mediainfo_path percorso`.
