# Articoli falliti e riposting

Durante un upload alcuni articoli possono fallire (una connessione cade, il server li rifiuta). ngPost prova più volte ogni articolo. Se alcuni falliscono ancora, il post termina in arancione e il log dice quanti articoli sono falliti.

## Riposta gli articoli mancanti

Quando alcuni articoli sono falliti, seleziona il post nella pagina **Post**: il pulsante **Ripubblica gli articoli mancanti** posta di nuovo **solo quegli articoli** e corregge il file NZB. Non serve inviare di nuovo tutto il post.

La ripubblicazione compare come una nuova riga nella lista; se finisce senza errori, non manca più nulla.

## Consigli

- Se falliscono molti articoli, la causa è di solito la connessione. Riduci il numero di connessioni, oppure controlla i limiti del provider.
- Se gli stessi articoli falliscono ancora e ancora, controlla il tuo account e i newsgroup.
- Il numero di tentativi si imposta con l'opzione `--retry` (predefinito 5) o nel file di configurazione.

Vedi [Server e connessioni](02-servers.md).
