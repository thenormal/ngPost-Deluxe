# Newsgroups

Les newsgroups sont les endroits d'Usenet où vos articles sont postés. Ils se définissent dans **Paramètres**, catégorie **Envois**, carte **Newsgroups**.

## La zone des groupes

Chaque newsgroup est une petite « pastille » :

- Tapez un nom et appuyez sur **Entrée**, **Tab**, la virgule ou l'espace pour l'ajouter.
- Pendant la saisie, une courte liste de groupes courants est proposée.
- Cliquez sur la **×** d'une pastille, ou appuyez sur **Retour arrière** dans la zone vide, pour la supprimer.
- Un nom non valide (par exemple sans point) s'affiche en rouge.

Un nom valide contient des lettres minuscules, des chiffres, `+`, `_` ou `-`, séparés par des points, par exemple `alt.binaries.test`.

## Favoris

Cliquez sur l'étoile d'un groupe pour en faire un favori (un autre clic le retire). Les favoris viennent en premier dans les suggestions, dans toutes les zones de groupes. Le bouton étoile à droite de la zone les liste : cliquez sur un groupe pour l'ajouter à la zone ou le retirer. Ils sont enregistrés dans le fichier de configuration (clé `favorite_groups`).

## Les groupes d'un seul envoi

Quick Post et l'envoi automatique ont leur propre zone **Newsgroups**. Vide, elle affiche les groupes des Paramètres, qui sont alors utilisés. Remplissez-la pour poster dans d'autres groupes sans changer les Paramètres ; un profil peut garder ces groupes. Le menu "Où vont les envois" des Paramètres s'applique aussi à eux.

## Vérifications avant l'envoi

ngPost ne lance pas un envoi s'il n'y a aucun newsgroup, ou si l'un d'eux n'est pas valide. Le message vous invite à les corriger dans les Paramètres.

## Où vont les envois

Quand vous avez plusieurs groupes, le menu à côté de la zone choisit où va chaque article :

- **Tous les groupes dans chaque envoi** (par défaut) : chaque article est posté dans tous les groupes.
- **Un groupe au hasard pour chaque envoi** : chaque envoi va dans un groupe de la liste, choisi au hasard.
- **Un groupe au hasard pour chaque fichier** : chaque fichier va dans un groupe choisi au hasard. Cela demande l'option d'**obfuscation des articles** (dans Confidentialité).

Utilisez un groupe de test comme `alt.binaries.test` pendant votre apprentissage. Pour les vrais envois, utilisez les groupes qu'attendent votre fournisseur et la communauté.
