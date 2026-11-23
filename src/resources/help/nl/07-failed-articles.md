# Mislukte artikelen en opnieuw posten

Tijdens een upload kunnen sommige artikelen mislukken (een verbinding valt weg, de server weigert ze). ngPost probeert elk artikel meerdere keren. Als sommige blijven mislukken, eindigt de post in oranje en vermeldt het logboek hoeveel artikelen zijn mislukt.

## Ontbrekende artikelen opnieuw posten

Als er artikelen zijn mislukt, selecteer je de post op de pagina **Posts**: de knop **Ontbrekende artikelen opnieuw posten** post **alleen die artikelen** opnieuw en herstelt het NZB-bestand. Je hoeft niet de hele post opnieuw te versturen.

Het opnieuw posten verschijnt als een nieuwe regel in de lijst; eindigt het zonder fouten, dan ontbreekt er niets meer.

## Tips

- Als veel artikelen mislukken, is de oorzaak meestal de verbinding. Verlaag het aantal verbindingen, of controleer de limieten van de provider.
- Als dezelfde artikelen steeds opnieuw mislukken, controleer dan uw account en de nieuwsgroepen.
- Het aantal pogingen kan worden ingesteld met de optie `--retry` (standaard 5) of in het configuratiebestand.

Zie [Servers en verbindingen](02-servers.md).
