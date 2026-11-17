# Opdrachtregel

Het programma kan ook zonder venster draaien, voor een server, een NAS of een script. De opdrachtregelversie is hetzelfde programma zonder het venster gebouwd (of de Docker-image).

## Voorbeelden

Twee items posten met compressie, willekeurige naam en wachtwoord, en par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Elk item van een map posten, elk als aparte post:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Een map bewaken en de bestanden na het posten naar een map "Posted" verplaatsen:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` accepteert een naam zoals `Posted` (de map wordt naast elk bestand of elke map aangemaakt) of een volledig pad. Gebruik in plaats daarvan `--rm_posted` om de bestanden na het posten te verwijderen. Beide gelden alleen als alles zonder fouten is gepost.

## Nuttige opties

- `-c` configuratiebestand (anders `$HOME/.ngPost`)
- `-g` nieuwsgroepen, gescheiden door komma's
- `-o` NZB-uitvoerbestand
- `-S` server in één tekenreeks
- `--disp_progress` BAR of FILES
- `--resume_queue` posts die niet klaar waren opnieuw posten
- `--allow_repost` bestanden die al in de geschiedenis staan opnieuw posten (standaard worden ze overgeslagen)
- `--gen_nfo` een NFO-bestand naast de nzb schrijven; `--nfo_in_post` het ook posten; `--nfo_password` het wachtwoord van het archief erin schrijven; `--nfo_text file.txt` een tekst aan het eind toevoegen; `--no_nfo_media` de media-informatie (MediaInfo) weglaten; `--mediainfo_path pad` het MediaInfo-programma kiezen
- `--notify_discord URL`, `--notify_telegram_token T` met `--notify_telegram_chat ID`, `--notify_webhook URL` en `--notify_when each|all_done`: meldingen aan het einde van de posts (zie [Voortgang en logboek](06-progress-and-log.md))
- `--help` de volledige lijst

Zie [Auto Posting](08-auto-posting.md).
