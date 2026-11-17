# Quick Post

Quick Post verstuurt de bestanden en mappen die u kiest. Elk tabblad is één post, zodat u meerdere posts kunt voorbereiden en in een wachtrij kunt uitvoeren.

## Stappen

1. Voeg bestanden en mappen toe met **Bestanden kiezen** of door ze te slepen.
2. Kies de opties (hieronder).
3. Klik op **Post Bestanden**.

## Opties

- **Comprimeren**: stopt de bestanden vóór het posten in een RAR- of 7z-archief. U kunt het in volumes splitsen.
- **Wachtwoord**: het archief kan een wachtwoord hebben, vast of willekeurig gegenereerd. Het wachtwoord wordt bij de post opgeslagen (bijvoorbeeld in de NZB-metadata en in de Geschiedenis).
- **Willekeurige naam**: geeft het archief een willekeurige naam.
- **par2**: maakt herstelbestanden. Stel de redundantie in procenten in: meer redundantie laat de downloader meer ontbrekende artikelen herstellen, maar uploadt meer gegevens.
- **Versluiering**: verbergt de onderwerpen van de artikelen. Zonder het NZB-bestand kan niemand uw post vinden, zelfs u niet.

## Controles en samenvatting

Onder de kaarten laat een regel zien wat er gepost gaat worden: het aantal items, hun grootte en, met compressie of par2, ongeveer hoeveel er wordt geüpload. De uploadtijd verschijnt na de eerste post van de sessie, op basis van de werkelijke snelheid van die upload (daarvoor gist ngPost niet).

Als u op **Post Bestanden** klikt, controleert ngPost eerst de post. Als er iets mis is, legt een venster dat in gewone woorden uit, met een knop die het herstelt:

- **Fouten** (de post kan niet starten): geen ingeschakelde server, geen geldige nieuwsgroep, mappen zonder compressie, rar- / 7z- of par2-programma niet gevonden, een ingeschakeld maar leeg wachtwoord, een NZB-map waarin niet kan worden geschreven.
- **Waarschuwingen** (u kunt **Toch posten** kiezen): de tijdelijke map ontbreekt of is mogelijk te klein, MediaInfo is niet gevonden voor de media-informatie van de NFO.

Na een herstelactie worden de controles opnieuw uitgevoerd, zodat een nieuw probleem meteen opduikt in plaats van midden in de post. Auto Posting heeft dit venster nog niet.

## Profielen

Het menu **Profiel** boven de kaarten stelt alle opties in één keer in.

- **Aanbevolen**: compressie, willekeurige naam en wachtwoord, par2 met 10 % redundantie. Dit is de keuze van een nieuwe installatie.
- **Aangepast**: verschijnt als u een optie met de hand wijzigt.
- **Uw eigen profielen**: stel de opties in, klik op **Opslaan als profiel...** en geef het een naam. **Verwijderen** verwijdert het gekozen profiel. De profielen worden in het configuratiebestand opgeslagen (sleutel `profile`) en zijn in Quick Post en Auto Posting dezelfde.

De kaart **NFO-bestand** (zie [NZB-bestanden](09-nzb-files.md)) wordt ook gedeeld: Quick Post en Auto Posting tonen dezelfde instellingen.

## Tabbladen

Het tabblad **+** opent een nieuwe post. Een tabblad toont zijn status met een pictogram: wachtend, wordt geüpload, klaar of fout.

Zie ook [Voortgang en logboek](06-progress-and-log.md).
