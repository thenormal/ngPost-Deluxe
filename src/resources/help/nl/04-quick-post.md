# Quick Post

Quick Post bereidt één post voor: de bestanden en mappen van de lijst worden één release. Als je op **Post Bestanden** drukt, gaat de post naar de pagina **Posts** en is Quick Post weer leeg, klaar voor de volgende release.

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

## Naam, nieuwsgroepen en servers

De kaart **NZB** heeft drie regels meer. Leeg gebruikt elke regel wat in de Instellingen staat.

- **Naam**: een sjabloon voor de naam van de post (het NZB-bestand en, zonder willekeurige naam, het archief): `{name}` is de naam van het bestand of de map, `{date}` de datum (2026-11-23), `{time}` de tijd (15-04) en `{n}` een nummer dat bij elke post met één stijgt. Voorbeeld: `{name} [{date}]`.
- **Nieuwsgroepen**: de groepen van deze post, met de favorieten (zie [Nieuwsgroepen](03-newsgroups.md)).
- **Servers**: alle ingeschakelde servers (standaard) of alleen de servers die je in het menu aanvinkt. Een post houdt zijn servers in de wachtrij, ook na een herstart.

Deze regels worden onthouden als het programma wordt gesloten.

## Profielen

Het menu **Profiel** boven de kaarten stelt alle opties in één keer in.

- **Aanbevolen**: compressie, willekeurige naam en wachtwoord, par2 met 10 % redundantie. Dit is de keuze van een nieuwe installatie.
- **Aangepast**: verschijnt als u een optie met de hand wijzigt.
- **Uw eigen profielen**: stel de opties in, klik op **Opslaan als profiel...** en geef het een naam. **Verwijderen** verwijdert het gekozen profiel. De profielen worden in het configuratiebestand opgeslagen (sleutel `profile`) en zijn in Quick Post en Auto Posting dezelfde.

De kaart **NFO-bestand** (zie [NZB-bestanden](09-nzb-files.md)) wordt ook gedeeld: Quick Post en Auto Posting tonen dezelfde instellingen.

Als je een profiel opslaat, kun je ook het naamsjabloon, de nieuwsgroepen, de servers en de NZB-map opslaan (vink in het venster aan wat je wilt). Een onderdeel dat niet is opgeslagen, verandert niet als je het profiel kiest. Beweeg de muis over een profiel in het menu om te zien wat erin staat.

## Na Post Bestanden

De post komt in de wachtrij en het programma toont de pagina **Posts**, waar je de voortgang en het logboek volgt. Quick Post houdt je opties en maakt een nieuwe willekeurige naam en een nieuw wachtwoord, zodat je meteen de volgende release kunt voorbereiden. Met **Gepauzeerd toevoegen** aangevinkt wacht de post in de wachtrij tot je hem hervat.

Zie ook [Voortgang en logboek](06-progress-and-log.md).
