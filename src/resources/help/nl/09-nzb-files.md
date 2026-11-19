# NZB-bestanden

Een NZB-bestand somt de artikelen van een post op. Iedereen die het heeft kan de post van Usenet downloaden.

## Waar het wordt opgeslagen

De map wordt ingesteld bij **Instellingen**, categorie **NZB-bestanden**. Het bestand krijgt de naam van de post (het eerste bestand of de eerste map).

## Gebruik

- Deel het samen met het wachtwoord van het archief, als u er een hebt ingesteld.
- Een downloadprogramma (een newsreader) opent het en downloadt de bestanden.
- Met versluiering is de NZB de **enige** manier om uw post te vinden: bewaar een kopie.

## Een NZB controleren

Op de opdrachtregel controleert `--check` of de artikelen van een NZB nog beschikbaar zijn op de server.

## Geschiedenis

De pagina **Geschiedenis** somt uw eerdere uploads op (datum, naam, grootte, snelheid, archief, wachtwoord, groepen). U kunt de geschiedenis uitzetten in de Instellingen. Het wachtwoord wordt in leesbare tekst opgeslagen.

De weergave **Statistieken** van deze pagina (de schakelaar naast **Lijst**) maakt van de geschiedenis totalen en grafieken: zie [Statistieken](09b-statistics.md).

### Posts die al in de geschiedenis staan

Voordat een post start, zoekt ngPost in de geschiedenis naar een geslaagde post met dezelfde NZB-naam en dezelfde totale grootte van de bestanden. In **Quick Post** vraagt een venster of u hem opnieuw wilt posten (**Toch posten** of **Annuleren**). In **Auto Posting** en op de opdrachtregel wordt de post overgeslagen en vermeldt een regel in het logboek dat. Om hem toch opnieuw te posten, gebruikt u de optie `--allow_repost` of zet u de waarschuwing uit bij **Instellingen**, categorie **Geschiedenis en wachtrij** (kaart Geschiedenis van de uploads). De controle vereist dat de geschiedenis aan staat en kijkt alleen naar de naam en de grootte: een hernoemde map wordt niet herkend.

## NFO-bestand

Een NFO is een klein tekstbestand dat de post beschrijft: de naam, de datum, de bestanden met hun grootte, hoe hij is voorbereid (archief en par2) en een tekst van uzelf. Schakel het in bij de kaart **NFO-bestand** van Quick Post of Auto Posting (het is dezelfde kaart: de instellingen worden gedeeld).

- **Een NFO-bestand naast de nzb schrijven**: `naam.nfo` wordt opgeslagen in dezelfde map als `naam.nzb`.
- **Het NFO-bestand ook met de bestanden posten**: het wordt met de post geüpload en verschijnt in de NZB.
- **Media-informatie opnemen (MediaInfo)**: standaard aan. Voor video- en audiobestanden somt de NFO de duur op, de video (formaat, afmetingen, framesnelheid, HDR), de audiosporen (taal, formaat, kanalen, bitsnelheid) en de ondertitels. Hoogstens 20 bestanden worden beschreven (de andere worden geteld) en het lezen stopt na 20 seconden. Het gebruikt het gratis programma MediaInfo: het zit in het Windows-installatieprogramma en de Docker-image; installeer in Linux en macOS het pakket `mediainfo`, of kies het programma in de Instellingen. Zonder MediaInfo heeft de NFO gewoon geen mediasectie.
- **Het wachtwoord van het archief schrijven**: standaard uit. In een geposte NFO kan het wachtwoord worden gelezen door iedereen die de post downloadt.
- **Tekst die aan het eind wordt toegevoegd**: notities, links, alles wat u wilt zeggen. Dezelfde tekst wordt voor alle posts gebruikt.

Als de namen van de bestanden verborgen zijn (versluiering van de artikelen of van de bestandsnamen), bevat de geposte NFO geen bestandsnamen of media-informatie, en krijgt het bestand zelf een willekeurige naam. Gebruik op de opdrachtregel `--gen_nfo`, `--nfo_in_post`, `--nfo_password` en `--nfo_text file.txt`, plus `--no_nfo_media` en `--mediainfo_path pad`.
