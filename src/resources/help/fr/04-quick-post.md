# Post Rapide

Quick Post prépare un post : les fichiers et dossiers de la liste forment une release. Quand vous cliquez sur **Poster Fichiers**, le post passe dans la page **Posts** et Quick Post est de nouveau vide, prêt pour la release suivante.

## Étapes

1. Ajoutez des fichiers et des dossiers avec **Choisir des fichiers** ou en les glissant.
2. Choisissez les options (ci-dessous).
3. Cliquez sur **Poster Fichiers**.

## Options

- **Compresser** : place les fichiers dans une archive RAR ou 7z avant l'envoi. Vous pouvez la découper en volumes.
- **Mot de passe** : l'archive peut avoir un mot de passe, fixe ou généré au hasard. Le mot de passe est enregistré avec l'envoi (par exemple dans les métadonnées du NZB et dans l'Historique).
- **Nom aléatoire** : donne à l'archive un nom aléatoire.
- **par2** : crée des fichiers de récupération. Réglez la redondance en pourcentage : plus de redondance permet au téléchargeur de réparer plus d'articles manquants, mais envoie plus de données.
- **Obfuscation** : masque les sujets des articles. Sans le fichier NZB, personne ne peut retrouver votre envoi, pas même vous.

## Vérifications et résumé

Sous les cartes, une ligne indique ce qui va être posté : le nombre d'éléments, leur taille et, avec la compression ou par2, la quantité approximative à téléverser. La durée du téléversement apparaît après le premier envoi de la session, d'après la vitesse réelle de ce téléversement (avant, ngPost ne devine pas).

Quand vous cliquez sur **Poster Fichiers**, ngPost vérifie d'abord l'envoi. Si quelque chose ne va pas, une fenêtre l'explique en termes simples, avec un bouton pour le corriger :

- **Erreurs** (l'envoi ne peut pas démarrer) : aucun serveur activé, aucun newsgroup valide, dossiers sans compression, programme rar / 7z ou par2 introuvable, mot de passe activé mais vide, dossier NZB non accessible en écriture.
- **Avertissements** (vous pouvez choisir **Poster quand même**) : le dossier temporaire est absent ou peut être trop petit, MediaInfo est introuvable pour les informations médias du NFO.

Après une correction, les vérifications sont refaites : un nouveau problème apparaît tout de suite au lieu de surgir au milieu de l'envoi. Post Auto n'a pas encore cette fenêtre.

## Nom, newsgroups et serveurs

La carte **NZB** a trois lignes de plus. Vide, chacune utilise ce qui est défini dans les Paramètres.

- **Nom** : un modèle pour le nom de l'envoi (le fichier NZB et, sans nom aléatoire, l'archive) : `{name}` est le nom du fichier ou du dossier, `{date}` la date (2026-11-23), `{time}` l'heure (15-04) et `{n}` un numéro qui augmente de un à chaque envoi. Exemple : `{name} [{date}]`.
- **Newsgroups** : les groupes de cet envoi, avec les favoris (voir [Newsgroups](03-newsgroups.md)).
- **Serveurs** : tous les serveurs activés (par défaut) ou seulement ceux cochés dans le menu. Un envoi garde ses serveurs dans la file, même après un redémarrage.

Ces lignes sont mémorisées à la fermeture du programme.

## Profils

Le menu **Profil** au-dessus des cartes règle toutes les options d'un coup.

- **Recommandé** : compression, nom et mot de passe aléatoires, par2 avec 10 % de redondance. C'est le choix d'une nouvelle installation.
- **Personnalisé** : affiché quand vous changez une option à la main.
- **Vos propres profils** : réglez les options, cliquez sur **Enregistrer comme profil...** et donnez-lui un nom. **Supprimer** retire le profil choisi. Les profils sont enregistrés dans le fichier de configuration (clé `profile`) et sont les mêmes dans Post Rapide et Post Auto.

La carte **Fichier NFO** (voir [Fichiers NZB](09-nzb-files.md)) est aussi partagée : Post Rapide et Post Auto affichent les mêmes réglages.

En enregistrant un profil, vous pouvez aussi enregistrer le modèle de nom, les newsgroups, les serveurs et le dossier NZB (cochez ce que vous voulez dans la fenêtre). Une partie non enregistrée n'est pas changée quand vous choisissez le profil. Passez la souris sur un profil du menu pour voir ce qu'il contient.

## Après Poster Fichiers

Le post entre dans la file et le programme affiche la page **Posts**, où vous suivez sa progression et son journal. Quick Post garde vos options et crée un nouveau nom et un nouveau mot de passe aléatoires, pour préparer tout de suite la release suivante. Avec **Ajouter en pause** coché, le post attend dans la file jusqu'à ce que vous le repreniez.

Voir aussi [Progression et journal](06-progress-and-log.md).
