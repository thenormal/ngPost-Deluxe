# File d'attente et reprise

## La file d'attente

Quand vous postez plusieurs releases, elles attendent leur tour dans la page **Posts** et sont envoyées l'une après l'autre. Le bouton **pause** en bas de la fenêtre arrête l'envoi en cours et le relance.

## Ordre, priorité et pause

Dans la page **Posts**, les posts en attente sont affichés dans l'ordre où ils démarreront, avec leur place : *En attente (1)* est le prochain. Sélectionnez un post pour le modifier avec les boutons du haut ou avec un clic droit :

- **Priorité** : **Haute**, **Normale** (par défaut) ou **Basse**. Un post de priorité plus haute passe avant les posts de priorité plus basse ; il n'interrompt pas le post en cours d'envoi.
- **Flèches** : déplacent un post en attente vers le haut, vers le bas, en tête ou en fin de file. Vous pouvez aussi le glisser avec la souris. Un post déplacé reste là où vous le mettez, quelle que soit sa priorité.
- **Pause** / **Reprendre** : un post en attente mis en pause reste dans la file et les posts suivants passent avant lui. Le post en cours d'envoi arrête d'envoyer et garde sa place : les autres posts attendent que vous le repreniez, car les posts sont envoyés un par un. S'il est encore en compression, il se met en pause quand son envoi commence.
- **Ajouter en pause**, à côté de **Poster Fichiers** dans Quick Post et de **Poster la liste** dans Post Auto : les posts entrent dans la file sans démarrer. Reprenez-les quand vous voulez.

Pendant qu'un post est envoyé, le suivant peut déjà être compressé (*En préparation (1)*, puis *Prêt (1)*). Ce post reste premier : il ne peut pas être mis en pause ni déplacé, et rien ne peut passer avant lui. Vous pouvez quand même l'annuler.

## File d'attente enregistrée

Par défaut, les envois non terminés sont enregistrés. Si vous fermez ngPost, ou si l'ordinateur s'arrête, au démarrage suivant une fenêtre indique combien d'envois n'étaient pas terminés :

- **Reprendre** : ils sont relancés. Un envoi qui était en cours repart **du début**.
- **Abandonner** : ils sont supprimés.
- **Décider plus tard** : ils restent enregistrés et seront proposés de nouveau au prochain démarrage.

Les posts repris apparaissent dans la page **Posts**.

## Bon à savoir

- L'ordre, la priorité et la pause de chaque post sont enregistrés avec la file : après un redémarrage, les posts reviennent comme vous les avez laissés. En ligne de commande, les posts en pause sont aussi postés.
- La file enregistrée contient aussi le mot de passe de chaque archive, en clair, comme l'Historique. Vous pouvez la désactiver dans **Paramètres**, catégorie **Historique et file d'attente**, carte **File des envois** : cela supprime aussi la file enregistrée.
- En ligne de commande, utilisez `--resume_queue`.
