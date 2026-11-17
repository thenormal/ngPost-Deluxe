# Mislukte artikelen en opnieuw posten

Tijdens een upload kunnen sommige artikelen mislukken (een verbinding valt weg, de server weigert ze). ngPost probeert elk artikel meerdere keren. Als sommige blijven mislukken, eindigt de post in oranje en vermeldt het logboek hoeveel artikelen zijn mislukt.

## Ontbrekende artikelen opnieuw posten

Als er artikelen zijn mislukt, toont het tabblad een knop **Ontbrekende artikelen opnieuw**. Die post **alleen die artikelen** opnieuw en herstelt het NZB-bestand. U hoeft niet de hele post opnieuw te versturen.

Na een geslaagde herpost wordt de bestandslijst bijgewerkt en wordt de post groen.

## Tips

- Als veel artikelen mislukken, is de oorzaak meestal de verbinding. Verlaag het aantal verbindingen, of controleer de limieten van de provider.
- Als dezelfde artikelen steeds opnieuw mislukken, controleer dan uw account en de nieuwsgroepen.
- Het aantal pogingen kan worden ingesteld met de optie `--retry` (standaard 5) of in het configuratiebestand.

Zie [Servers en verbindingen](02-servers.md).
