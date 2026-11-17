# Fichiers NZB

Un fichier NZB liste les articles d'un envoi. Toute personne qui l'a peut télécharger l'envoi depuis Usenet.

## Où il est enregistré

Le dossier se définit dans **Paramètres**, catégorie **Fichiers NZB**. Le fichier prend le nom de l'envoi (le premier fichier ou dossier).

## L'utiliser

- Partagez-le avec le mot de passe de l'archive, si vous en avez défini un.
- Un programme de téléchargement (un lecteur de news) l'ouvre et télécharge les fichiers.
- Avec l'obfuscation, le NZB est le **seul** moyen de retrouver votre envoi : gardez-en une copie.

## Vérifier un NZB

En ligne de commande, `--check` vérifie que les articles d'un NZB sont toujours disponibles sur le serveur.

## Historique

La page **Historique** liste vos téléversements passés (date, nom, taille, vitesse, archive, mot de passe, groupes). Vous pouvez désactiver l'historique dans les Paramètres. Le mot de passe est enregistré en clair.

### Envois déjà dans l'historique

Avant qu'un envoi ne démarre, ngPost cherche dans l'historique un envoi réussi ayant le même nom de NZB et la même taille totale de fichiers. Dans **Post Rapide**, une fenêtre demande s'il faut le reposter (**Poster quand même** ou **Annuler**). Dans **Post Auto** et en ligne de commande, l'envoi est ignoré et une ligne du journal l'indique. Pour le poster malgré tout, utilisez l'option `--allow_repost` ou désactivez l'avertissement dans **Paramètres**, catégorie **Historique et file d'attente** (carte Historique des téléversements). La vérification demande que l'historique soit activé, et elle ne regarde que le nom et la taille : un dossier renommé n'est pas reconnu.

## Fichier NFO

Un NFO est un petit fichier texte qui décrit l'envoi : son nom, la date, les fichiers avec leur taille, la façon dont il a été préparé (archive et par2) et un texte de votre choix. Activez-le dans la carte **Fichier NFO** de Post Rapide ou de Post Auto (c'est la même carte : les réglages sont partagés).

- **Écrire un fichier NFO à côté du nzb** : `nom.nfo` est enregistré dans le même dossier que `nom.nzb`.
- **Poster aussi le fichier NFO avec les fichiers** : il est téléversé avec l'envoi et apparaît dans le NZB.
- **Inclure les informations médias (MediaInfo)** : activé par défaut. Pour les fichiers vidéo et audio, le NFO liste la durée, la vidéo (format, taille, fréquence d'images, HDR), les pistes audio (langue, format, canaux, débit) et les sous-titres. Au plus 20 fichiers sont décrits (les autres sont comptés) et la lecture s'arrête après 20 secondes. Il utilise le programme gratuit MediaInfo : il est fourni avec l'installateur Windows et l'image Docker ; sous Linux et macOS, installez le paquet `mediainfo`, ou choisissez le programme dans les Paramètres. Sans MediaInfo, le NFO n'a simplement pas de section médias.
- **Écrire le mot de passe de l'archive** : désactivé par défaut. Dans un NFO posté, le mot de passe peut être lu par toute personne qui télécharge l'envoi.
- **Texte ajouté à la fin** : notes, liens, tout ce que vous voulez dire. Le même texte est utilisé pour tous les envois.

Quand les noms des fichiers sont masqués (obfuscation des articles ou des noms de fichiers), le NFO posté ne liste aucun nom de fichier ni information média, et le fichier lui-même reçoit un nom aléatoire. En ligne de commande, utilisez `--gen_nfo`, `--nfo_in_post`, `--nfo_password` et `--nfo_text file.txt`, ainsi que `--no_nfo_media` et `--mediainfo_path chemin`.
