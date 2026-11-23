# Articles échoués et reposter

Pendant un téléversement, certains articles peuvent échouer (une connexion tombe, le serveur les refuse). ngPost essaie chaque article plusieurs fois. Si certains échouent encore, l'envoi se termine en orange et le journal indique combien d'articles ont échoué.

## Reposter les articles manquants

Quand des articles ont échoué, sélectionnez le post dans la page **Posts** : le bouton **Reposter les articles manquants** reposte **seulement ces articles** et corrige le fichier NZB. Pas besoin de renvoyer tout le post.

Le repost apparaît comme une nouvelle ligne dans la liste ; s'il se termine sans erreur, plus rien ne manque.

## Conseils

- Si beaucoup d'articles échouent, la cause est généralement la connexion. Diminuez le nombre de connexions, ou vérifiez les limites du fournisseur.
- Si les mêmes articles échouent encore et encore, vérifiez votre compte et les newsgroups.
- Le nombre de tentatives se règle avec l'option `--retry` (5 par défaut) ou dans le fichier de configuration.

Voir [Serveurs et connexions](02-servers.md).
