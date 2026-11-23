# File d'attente et reprise

## La file d'attente

Quand vous postez plusieurs releases, elles attendent leur tour dans la page **Posts** et sont envoyées l'une après l'autre. Le bouton **pause** arrête les envois et les relance.

## File d'attente enregistrée

Par défaut, les envois non terminés sont enregistrés. Si vous fermez ngPost, ou si l'ordinateur s'arrête, au démarrage suivant une fenêtre indique combien d'envois n'étaient pas terminés :

- **Reprendre** : ils sont relancés. Un envoi qui était en cours repart **du début**.
- **Abandonner** : ils sont supprimés.
- **Décider plus tard** : ils restent enregistrés et seront proposés de nouveau au prochain démarrage.

Les posts repris apparaissent dans la page **Posts**.

## Bon à savoir

- La pause n'est pas mémorisée après un redémarrage.
- La file enregistrée contient aussi le mot de passe de chaque archive, en clair, comme l'Historique. Vous pouvez la désactiver dans **Paramètres**, catégorie **Historique et file d'attente**, carte **File des envois** : cela supprime aussi la file enregistrée.
- En ligne de commande, utilisez `--resume_queue`.
