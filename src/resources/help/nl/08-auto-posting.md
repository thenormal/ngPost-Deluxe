# Auto Posting

Auto Posting post veel bestanden en mappen zonder dat u elk met de hand voorbereidt. Kies een van de drie knoppen bovenaan de pagina; alleen de instellingen die u nodig hebt worden getoond.

## Post wat ik kies

Voeg de gewenste bestanden en mappen toe (sleep ze naar de lijst, of gebruik **Bestanden kiezen** / **Selecteer een Map**) en klik op **Lijst posten**: elk bestand of elke map wordt een aparte post. Met **Gepauzeerd toevoegen** aangevinkt (ook voor **Eén keer een map scannen**) wachten de posts op de pagina **Posts** tot je ze hervat.

## Een map eenmalig scannen

Kies een map en klik op **Scannen**: wat erin zit wordt weergegeven. Mappen hebben **compressie** nodig. Gebruik **nieuwste bestanden eerst** om op datum te sorteren.

## Een map bewaken en posten wat binnenkomt

Kies de map en klik op **Bewaking starten**: elk nieuw bestand of elke nieuwe map wordt gepost zodra die binnenkomt. Klik op **Bewaking stoppen** om te stoppen. Stoppen terwijl er nog posts wachten vraagt om bevestiging.

Tijdens het bewaken toont een balk onder de drie keuzes altijd de bewaakte map, hoeveel posts er vandaag klaar zijn (en hoeveel met fouten) en hoeveel er bezig zijn of wachten. **Bewaking pauzeren** laat nieuwe bestanden wachten: ze worden gepost als u op **Bewaking hervatten** klikt; posts die al zijn gestart gaan door. Als u de bewaking stopt, worden de bestanden die wachtten vergeten.

## De lijst

Elk item heeft een vakje en zijn grootte. Schakel een item uit om het weg te laten; alleen aangevinkte items worden gepost. Om items uit de lijst te verwijderen, selecteert u ze en drukt u op DEL. De regel onder de lijst toont hoeveel items zijn geselecteerd, hun grootte, de te uploaden grootte en, na een eerste post, de uploadtijd.

## Items die al zijn gepost

Als sommige aangevinkte items al in de geschiedenis staan (zelfde naam en grootte), toont één venster ze met de datum van de vorige post. Vink **Opnieuw posten** aan voor degene die u nog een keer wilt posten; de andere worden niet gepost en blijven in de lijst staan, uitgevinkt en grijs. Bestaat er al een NZB-bestand met dezelfde naam, dan heet het nieuwe `naam_1.nzb` enzovoort. Bij het bewaken van een map worden items die al in de geschiedenis staan overgeslagen, met een regel in het logboek. U kunt de waarschuwing uitzetten bij **Instellingen** (Geschiedenis).

## Voordat het begint

Als u op **Lijst posten** of **Bewaking starten** klikt, controleert het programma de instellingen eenmalig voor de hele groep (een server en een nieuwsgroep, compressie voor mappen, de programma's rar / 7z en par2, de map van de NZB, de tijdelijke map). Problemen worden in gewone woorden getoond met een knop die ze herstelt; waarschuwingen kunnen worden genegeerd met **Toch posten**. Het is hetzelfde venster als in [Quick Post](04-quick-post.md).

## Naam, nieuwsgroepen en servers

De kaart **Naam, nieuwsgroepen en servers** werkt zoals de regels van [Quick Post](04-quick-post.md), met één regel meer: **NZB-map**, waar de NZB-bestanden van deze posts worden geschreven (leeg: de map van de Instellingen). Ze geldt ook voor de bewaakte mappen. Een profiel kan deze instellingen bewaren.

## Tips

- Gebruik compressie, willekeurige namen, willekeurige wachtwoorden en par2: dit is de gebruikelijke opzet voor geautomatiseerd posten. Het profiel **Aanbevolen** (menu **Profiel**) stelt het met één klik in.
- **Na het posten** (de rij boven de knop Posten): niets doen (standaard), de bestanden naar een **map "Posted"** verplaatsen, of ze verwijderen. De standaardnaam `Posted` maakt die map naast elk bestand of elke map aan; u kunt ook een andere map kiezen. Er wordt niets verplaatst of verwijderd tenzij de post zonder fouten is afgerond. Een naam die al in de map bestaat wordt nooit overschreven: het bestand wordt `naam (2)`. Het scannen en het bewaken van mappen negeren de map "Posted". Verwijderen is definitief: gebruik het alleen als u zeker bent.
- Op de opdrachtregel is dezelfde keuze `--move_posted` of `--rm_posted` (zie [Opdrachtregel](10-command-line.md)).
- Het programma onthoudt de laatste knop die u hebt gebruikt.
- Voor een NAS of een server zonder scherm doen de opdrachtregel en de Docker-image hetzelfde werk.

Zie [Opdrachtregel](10-command-line.md).
