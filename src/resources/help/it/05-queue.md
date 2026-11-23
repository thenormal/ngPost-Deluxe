# Coda e ripresa

## La coda

Quando pubblichi più release, aspettano il loro turno nella pagina **Post** e vengono caricate una dopo l'altra. Il pulsante **pausa** in fondo alla finestra ferma il caricamento in corso e lo fa ripartire.

## Ordine, priorità e pausa

Nella pagina **Post** i post in attesa sono mostrati nell'ordine in cui partiranno, con la loro posizione: *In attesa (1)* è il prossimo. Seleziona un post per cambiarlo con i pulsanti in alto o con il tasto destro:

- **Priorità**: **Alta**, **Normale** (predefinita) o **Bassa**. Un post con priorità più alta passa davanti ai post con priorità più bassa; non interrompe il post in caricamento.
- **Frecce**: spostano un post in attesa su, giù, in cima o in fondo alla coda. Puoi anche trascinarlo con il mouse. Un post che sposti resta dove lo metti, qualunque sia la sua priorità.
- **Pausa** / **Riprendi**: un post in attesa messo in pausa resta in coda e i post successivi passano prima. Il post in caricamento smette di inviare e tiene il suo posto: gli altri post aspettano finché non lo riprendi, perché i post vengono caricati uno alla volta. Se è ancora in compressione, va in pausa quando inizia il caricamento.
- **Aggiungi in pausa**, accanto a **Posta i file** in Quick Post e accanto a **Posta la lista** in Post automatico: i post entrano in coda senza partire. Riprendili quando vuoi.

Mentre un post si carica, il successivo può essere già compresso (*In preparazione (1)*, poi *Pronto (1)*). Quel post resta primo: non si può mettere in pausa né spostare, e niente può passargli davanti. Puoi comunque annullarlo.

## Coda salvata

Per impostazione predefinita i post non terminati vengono salvati. Se chiudi ngPost, o il computer si spegne, al successivo avvio una finestra mostra quanti post non erano finiti:

- **Riprendi**: vengono avviati di nuovo. Un post che stava per essere inviato riparte **dall'inizio**.
- **Scarta**: vengono rimossi.
- **Decidi dopo**: restano salvati e verranno riproposti al prossimo avvio.

I post ripresi compaiono nella pagina **Post**.

## Da sapere

- Ordine, priorità e pausa di ogni post vengono salvati con la coda: dopo un riavvio i post tornano come li avevi lasciati. Da riga di comando vengono pubblicati anche i post in pausa.
- La coda salvata contiene anche la password di ogni archivio, in chiaro, come la Cronologia. Puoi disattivarla in **Impostazioni**, categoria **Cronologia e coda**, scheda **Coda dei post**: questo cancella anche la coda salvata.
- Da riga di comando usa `--resume_queue`.
