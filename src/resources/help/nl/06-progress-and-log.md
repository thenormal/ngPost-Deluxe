# Voortgang en logboek

## De voortgangsbalk

De balk onderaan toont de huidige fase:

- **Compressie**: het archief wordt gemaakt.
- **par2**: de herstelbestanden worden gemaakt.
- **Upload**: de artikelen worden verstuurd. Hier ziet u ook de snelheid, de grootte, het aantal artikelen en de **ETA** (geschatte resterende tijd).
- **Voltooid**, **Gepauzeerd** of **Gestopt**.

De kleur geeft de status aan: blauw tijdens het werk, grijs bij pauze, groen als het klaar is, oranje als sommige artikelen zijn mislukt, rood als de post met een fout is gestopt. Het percentage staat ook in de titel van het venster.

De ETA is een schatting op basis van de recente snelheid: hij verandert als de snelheid verandert.

Als u een snelheidslimiet instelt (**Instellingen**, categorie **Posts**, kaart **Upload**), staat achter de snelheid de limiet, bijvoorbeeld *(max. 2.5 MB/s)*.

## Het logboek

Het logboek kleurt de regels: fasen, successen, waarschuwingen en fouten zijn gemakkelijk te onderscheiden. Aan het einde van een post toont het een samenvatting met de tijd van elke fase (compressie, par2, upload) en het totaal.

## Latentie

Aan het einde van een post toont het logboek ook de **latentie**: de mediane tijd van een heen-en-terugreis met de server (het commando POST en het antwoord, zonder gegevens) en van de verbinding. Ze wordt alleen gemeten aan de hand van wat ngPost al doet, er wordt er niets voor verstuurd. Een hoge waarde (meer dan een halve seconde) betekent dat de vertraging van het netwerk komt, van de afstand tot de server of van de server zelf, bijvoorbeeld door een VPN, en niet van uw computer.

## Diagnoserapport

Als iets niet werkt, slaat de knop **Diagnoserapport**, onder het logboek in Quick Post, een rapport op van wat het programma heeft gedaan, met een sectie **Latentie** voor elke server (verbinding, SSL, aanmelden, heen en terug: minimum, mediaan, 95e percentiel en maximum) en een sectie **Meldingen** (wat er wordt gemeld, of de systeemmelding aan staat en beschikbaar is, welke kanalen zijn ingesteld en de laatste mislukking; de adressen en tokens worden nooit geschreven). Het is een tekstbestand dat u kunt bijvoegen als u om hulp vraagt. Controleer het voordat u het deelt: het mag uw wachtwoord niet bevatten.

## Meldingen

ngPost kan u laten weten wanneer uw posts klaar zijn, zodat u het venster niet hoeft te bekijken. Dit wordt ingesteld bij **Instellingen**, categorie **Meldingen**, in twee delen: **wat er wordt gemeld** (een post is klaar, een post met fouten of mislukt, één bericht aan het einde of één per post) en **waarheen de berichten gaan**. In het programma met venster staat de systeemmelding vanaf het begin aan; de andere kanalen hebben een adres nodig.

- **Systeemmelding**: een bericht vanaf het pictogram in het systeemvak (alleen de desktopversie; standaard aan, u kunt het uitzetten). In Windows verschijnt het als elke andere melding en blijft het in het actiecentrum; als u het niet ziet, controleer dan of de meldingen en de focushulp ("Niet storen") van Windows het niet verbergen.
- **Discord**: plak het adres van een Discord-webhook.
- **Telegram**: het token van uw bot en de id van de chat die de berichten ontvangt.
- **Andere webhook**: elk adres dat een JSON-POST met een `title` en een `text` accepteert (Slack-achtige hulpmiddelen, ntfy, Home Assistant).

Kies wat er wordt gemeld (klaar, fouten of mislukkingen) en hoe: **één bericht als alle posts klaar zijn** (standaard, het best voor Auto Posting) of **een bericht na elke post**. De knop **Testbericht verzenden** laat meteen zien of het werkt.

Het bericht bevat de naam van de post, het resultaat, de grootte, de tijd en de snelheid. Het bevat nooit het wachtwoord of de namen van de bestanden. De adressen en het token worden net als de andere wachtwoorden in leesbare tekst in het configuratiebestand opgeslagen: houd het privé. Als een bericht niet kan worden verstuurd, meldt een regel in het logboek dat en heeft de post er geen last van.

Gebruik op de opdrachtregel, of in Docker, de opties `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` en `--notify_when each|all_done`, of de sleutels `notify_*` van het configuratiebestand.
