# Post Auto

Post Auto poste de nombreux fichiers et dossiers sans préparer chacun à la main. Choisissez l'un des trois boutons en haut de la page ; seuls les réglages utiles sont affichés.

## Poster ce que je choisis

Ajoutez les fichiers et dossiers voulus (glissez-les dans la liste, ou utilisez **Choisir des fichiers** / **Sélectionner un dossier**), puis cliquez sur **Poster la liste** : chaque fichier ou dossier devient un envoi distinct.

## Analyser un dossier une fois

Choisissez un dossier et cliquez sur **Analyser** : son contenu est listé. Les dossiers nécessitent la **compression**. Utilisez **fichiers les plus récents d'abord** pour trier par date.

## Surveiller un dossier et poster ce qui arrive

Choisissez le dossier et cliquez sur **Commencer la surveillance** : chaque nouveau fichier ou dossier est posté dès son arrivée. Cliquez sur **Arrêter la surveillance** pour arrêter. Arrêter alors que des envois attendent encore demande une confirmation.

Pendant la surveillance, une barre sous les trois choix affiche toujours le dossier surveillé, combien d'envois sont terminés aujourd'hui (et combien avec des erreurs) et combien sont en cours ou en attente. **Mettre en pause** fait attendre les nouveaux fichiers : ils sont postés quand vous cliquez sur **Reprendre la surveillance** ; les envois déjà commencés continuent. Arrêter la surveillance oublie les fichiers qui attendaient.

## La liste

Chaque élément a une case et sa taille. Décochez un élément pour l'écarter ; seuls les éléments cochés sont postés. Pour retirer des éléments de la liste, sélectionnez-les et appuyez sur SUPPR. La ligne sous la liste indique combien d'éléments sont sélectionnés, leur taille, la taille à téléverser et, après un premier envoi, la durée du téléversement.

## Éléments déjà postés

Si certains éléments cochés sont déjà dans l'historique (même nom et même taille), une fenêtre les liste avec la date de l'envoi précédent. Cochez **Reposter** pour ceux que vous voulez poster une fois de plus ; les autres ne sont pas postés et restent dans la liste, décochés et grisés. Si un fichier NZB du même nom existe déjà, le nouveau s'appelle `nom_1.nzb` et ainsi de suite. Pendant la surveillance d'un dossier, les éléments déjà dans l'historique sont ignorés, avec une ligne dans le journal. Vous pouvez désactiver l'avertissement dans **Paramètres** (Historique).

## Avant le départ

Quand vous cliquez sur **Poster la liste** ou **Commencer la surveillance**, le programme vérifie les réglages une seule fois pour tout le groupe (un serveur et un newsgroup, la compression pour les dossiers, les programmes rar / 7z et par2, le dossier du NZB, le dossier temporaire). Les problèmes sont affichés en termes simples avec un bouton qui les corrige ; les avertissements peuvent être ignorés avec **Poster quand même**. C'est la même fenêtre que dans [Post Rapide](04-quick-post.md).

## Conseils

- Utilisez la compression, les noms aléatoires, les mots de passe aléatoires et par2 : c'est la configuration habituelle pour l'envoi automatisé. Le profil **Recommandé** (menu **Profil**) la règle en un clic.
- **Après le post** (la ligne au-dessus du bouton Poster) : ne rien faire (par défaut), déplacer les fichiers dans un **dossier « Posted »**, ou les supprimer. Le nom par défaut `Posted` crée ce dossier à côté de chaque fichier ou dossier ; vous pouvez aussi choisir un autre dossier. Rien n'est déplacé ni supprimé si l'envoi ne s'est pas terminé sans erreur. Un nom déjà utilisé dans le dossier n'est jamais écrasé : le fichier devient `nom (2)`. L'analyse et la surveillance des dossiers ignorent le dossier « Posted ». La suppression est définitive : ne l'utilisez que si vous êtes sûr.
- En ligne de commande, le même choix est `--move_posted` ou `--rm_posted` (voir [Ligne de commande](10-command-line.md)).
- Le programme se souvient du dernier bouton utilisé.
- Pour un NAS ou un serveur sans écran, la ligne de commande et l'image Docker font le même travail.

Voir [Ligne de commande](10-command-line.md).
