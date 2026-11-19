# Premiers pas

Cette page vous accompagne de l'installation jusqu'à votre premier envoi.

## Ce qu'il vous faut

- Un compte chez un **fournisseur Usenet** (nom d'hôte, port, nom d'utilisateur et mot de passe).
- Les fichiers ou dossiers que vous voulez poster.
- En option, les outils pour compresser (**7-Zip** ou **RAR**) et pour créer les fichiers par2 (**ParPar** ou **par2**). L'installateur Windows inclut déjà 7-Zip et ParPar.

## Votre premier envoi

Tant qu'un serveur et un newsgroup ne sont pas définis, une barre bleue en haut de Post Rapide et de Post Auto vous dit ce qui manque, avec un bouton qui ouvre la bonne page. Elle disparaît toute seule.

1. Ouvrez **Serveurs**, ajoutez votre fournisseur et cliquez sur **Tester la connexion**. Voir [Serveurs et connexions](02-servers.md).
2. Ouvrez **Paramètres**, catégorie **Envois**, carte **Newsgroups**, et ajoutez au moins un newsgroup. Voir [Newsgroups](03-newsgroups.md).
3. Ouvrez **Post Rapide**, cliquez sur **Choisir des fichiers** (ou glissez des fichiers et des dossiers dans la fenêtre).
4. Choisissez les options voulues (compression, par2, mot de passe) et cliquez sur **Poster Fichiers**.
5. Suivez la progression en bas de la fenêtre et dans le journal. À la fin, le **fichier NZB** est enregistré et indiqué dans le journal.

Le fichier NZB est la « carte » de votre envoi : sans lui, personne ne peut retrouver les articles, surtout si vous utilisez l'obfuscation. Gardez-le précieusement.

## Où se trouvent les choses

- **Post guidé** : répondez à quelques questions et le post est préparé pour vous. Voir [Post guidé](01b-guided-post.md).
- **Post Rapide** : poster tout de suite des fichiers et des dossiers.
- **Post Auto** : analyser ou surveiller des dossiers.
- **Serveurs** : vos fournisseurs Usenet.
- **Paramètres** : les options par défaut du programme, en cinq catégories (Général, Envois, Fichiers NZB, Notifications, Historique et file d'attente).
- **Historique** : tout ce que vous avez posté, avec ses [statistiques](09b-statistics.md).
- **Aide** : cette section.

## Nouvelles versions

Une fois par jour, au démarrage du programme, ngPost Deluxe demande à GitHub s'il existe une version plus récente (GitHub ne voit que votre adresse IP). S'il y en a une, un bouton **Version X available** apparaît en bas de la barre de gauche : cliquez dessus pour ouvrir la page de la version. Rien n'est téléchargé ni installé automatiquement, et les versions candidates ne sont jamais signalées.

Pour désactiver cette vérification, décochez **Chercher une nouvelle version au démarrage du programme** dans **Paramètres**, catégorie **Général**, carte **Interface**. La version en ligne de commande et l'image Docker ne vérifient jamais.

Suite : [Serveurs et connexions](02-servers.md).
