# Ligne de commande

Le programme peut aussi fonctionner sans fenêtre, pour un serveur, un NAS ou un script. La version en ligne de commande est le même programme compilé sans la fenêtre (ou l'image Docker).

## Exemples

Poster deux éléments avec compression, nom et mot de passe aléatoires, et par2 :

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Poster chaque élément d'un dossier, un envoi chacun :

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Surveiller un dossier et déplacer les fichiers dans un dossier « Posted » une fois postés :

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` accepte un nom comme `Posted` (le dossier est créé à côté de chaque fichier ou dossier) ou un chemin complet. Utilisez plutôt `--rm_posted` pour supprimer les fichiers une fois postés. Les deux ne s'appliquent que lorsque tout a été posté sans erreur.

## Options utiles

- `-c` fichier de configuration (sinon `$HOME/.ngPost`)
- `-g` newsgroups, séparés par des virgules
- `-o` fichier NZB de sortie
- `-S` serveur en une seule chaîne
- `--disp_progress` BAR ou FILES
- `--resume_queue` reposter les envois non terminés
- `--allow_repost` reposter les fichiers déjà présents dans l'historique (par défaut ils sont ignorés)
- `--max_speed Mo/s` limite la vitesse d'envoi, pour toutes les connexions ensemble (0 : pas de limite) ; équivaut aux clés `limit_speed` et `max_speed` du fichier de configuration
- `--gen_nfo` écrire un fichier NFO à côté du nzb ; `--nfo_in_post` le poster aussi ; `--nfo_password` y écrire le mot de passe de l'archive ; `--nfo_text file.txt` ajouter un texte à la fin ; `--no_nfo_media` omettre les informations médias (MediaInfo) ; `--mediainfo_path chemin` choisir le programme MediaInfo; `--nfo_style nom` choisir l'aspect (classic, clean, box, scene, elegant, retro) ; `--nfo_ascii` seulement des caractères ASCII
- `--notify_discord URL`, `--notify_telegram_token T` avec `--notify_telegram_chat ID`, `--notify_webhook URL` et `--notify_when each|all_done` : notifications à la fin des envois (voir [Progression et journal](06-progress-and-log.md))
- `--notify_slack URL`, `--notify_ntfy URL`, `--notify_gotify_url URL` avec `--notify_gotify_token T`, `--notify_pushover_token T` avec `--notify_pushover_user K`, `--notify_email_host`, `--notify_email_port`, `--notify_email_security starttls|ssl|none`, `--notify_email_user`, `--notify_email_pass`, `--notify_email_from`, `--notify_email_to` : les autres canaux
- `--export_settings fichier` écrit les paramètres, profils, notifications et serveurs (jamais les mots de passe) dans un fichier ; `--import_settings fichier` ajoute ce fichier à la configuration (l'ancienne reste sous `.before-import`) ; les deux quittent sans poster
- `--help` la liste complète

Voir [Post Auto](08-auto-posting.md).
