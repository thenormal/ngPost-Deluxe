# Eerste stappen

Deze pagina brengt u van een verse installatie naar uw eerste post.

## Wat u nodig hebt

- Een account bij een **Usenet-provider** (hostnaam, poort, gebruikersnaam en wachtwoord).
- De bestanden of mappen die u wilt posten.
- Optioneel de programma's om te comprimeren (**7-Zip** of **RAR**) en om par2-bestanden te maken (**ParPar** of **par2**). Het Windows-installatieprogramma bevat 7-Zip en ParPar al.

## Uw eerste post

Zolang er geen server en geen nieuwsgroep zijn ingesteld, laat een blauwe balk bovenaan Quick Post en Auto Posting zien wat er ontbreekt, met een knop die de juiste pagina opent. De balk verdwijnt vanzelf.

1. Open **Servers**, voeg uw provider toe en klik op **Verbinding testen**. Zie [Servers en verbindingen](02-servers.md).
2. Open **Instellingen**, categorie **Posts**, kaart **Nieuwsgroepen**, en voeg minstens één nieuwsgroep toe. Zie [Nieuwsgroepen](03-newsgroups.md).
3. Open **Quick Post** en klik op **Bestanden kiezen** (of sleep bestanden en mappen naar het venster).
4. Kies de gewenste opties (compressie, par2, wachtwoord) en klik op **Post Bestanden**.
5. Volg de voortgang onderaan het venster en in het logboek. Aan het einde wordt het **NZB-bestand** opgeslagen en in het logboek getoond.

Het NZB-bestand is de "kaart" van uw post: zonder dat bestand kan niemand de artikelen vinden, vooral als u versluiering gebruikt. Bewaar het goed.

## Waar vindt u wat

- **Begeleide post**: beantwoord een paar vragen en de post wordt voor u voorbereid. Zie [Begeleide post](01b-guided-post.md).
- **Quick Post**: bestanden en mappen meteen posten.
- **Auto Posting**: mappen scannen of bewaken.
- **Servers**: uw Usenet-providers.
- **Instellingen**: standaardopties van het programma, in vijf categorieën (Algemeen, Posts, NZB-bestanden, Meldingen, Geschiedenis en wachtrij).
- **Geschiedenis**: alles wat u hebt gepost, met de [statistieken](09b-statistics.md).
- **Help**: dit onderdeel.

## Nieuwe versies

Eén keer per dag, bij het starten van het programma, vraagt ngPost Deluxe aan GitHub of er een nieuwere versie bestaat (GitHub ziet alleen uw IP-adres). Als die er is, verschijnt onderaan de linkerbalk een knop **Version X available**: klik erop om de pagina van de release te openen. Er wordt niets vanzelf gedownload of geïnstalleerd, en release candidates worden nooit gemeld.

Om dit uit te zetten, schakelt u **Bij het starten naar een nieuwe versie zoeken** uit bij **Instellingen**, categorie **Algemeen**, kaart **Interface**. De opdrachtregelversie en de Docker-image controleren nooit.

## Van computer wisselen of een configuratie delen

Bij **Instellingen**, categorie **Geschiedenis en wachtrij**, kaart **Instellingen exporteren en importeren**, schrijft **Instellingen exporteren...** een tekstbestand met de instellingen van het programma, de profielen, de favoriete newsgroups, de gekozen meldingen en, als u wilt, de servers. **Instellingen importeren...** leest dit bestand op een andere computer en toont de inhoud voordat er iets verandert.

Het bestand bevat nooit een wachtwoord, token, meldingsadres, het vaste archiefwachtwoord of een map van de computer. Geïmporteerde servers blijven uit tot u hun wachtwoord invoert bij **Servers**. Servers die u al hebt, worden niet aangeraakt, en een profiel met dezelfde naam blijft behouden tenzij u vervangen kiest. De oude configuratie wordt naast de nieuwe bewaard (`.before-import`) en ngPost start opnieuw op om de nieuwe instellingen te gebruiken.

Vanaf de opdrachtregel: `--export_settings bestand` en `--import_settings bestand` (daarna stopt het programma).

Volgende: [Servers en verbindingen](02-servers.md).
