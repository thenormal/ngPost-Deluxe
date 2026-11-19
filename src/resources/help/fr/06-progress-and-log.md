# Progression et journal

## La barre de progression

La barre en bas montre la phase en cours :

- **Compression** : création de l'archive.
- **par2** : création des fichiers de récupération.
- **Téléversement** : envoi des articles. Vous y voyez aussi la vitesse, la taille, le nombre d'articles et l'**ETA** (temps restant estimé).
- **Terminé**, **En pause** ou **Arrêté**.

La couleur indique l'état : bleu pendant le travail, gris en pause, vert une fois terminé, orange si certains articles ont échoué, rouge si l'envoi s'est arrêté sur une erreur. Le pourcentage s'affiche aussi dans le titre de la fenêtre.

L'ETA est une estimation fondée sur la vitesse récente : elle change quand la vitesse change.

Si vous réglez une limite de vitesse (**Paramètres**, catégorie **Envois**, carte **Upload**), la vitesse est suivie de la limite, par exemple *(max 2.5 Mo/s)*.

## Le journal

Le journal colore les lignes : les phases, les succès, les avertissements et les erreurs se distinguent facilement. À la fin d'un envoi, il affiche un résumé avec la durée de chaque phase (compression, par2, téléversement) et le total.

## Latence

À la fin d'un envoi, le journal affiche aussi la **latence** : le temps médian d'un aller-retour avec le serveur (la commande POST et sa réponse, sans données) et de la connexion. Elle est mesurée uniquement à partir de ce que ngPost fait déjà, rien n'est envoyé pour cela. Une valeur élevée (plus d'une demi-seconde) signifie que le délai vient du réseau, de la distance au serveur ou du serveur lui-même, par exemple un VPN, et non de votre ordinateur.

## Rapport de diagnostic

Si quelque chose ne fonctionne pas, le bouton **Rapport de diagnostic**, sous le journal dans Post Rapide, enregistre un rapport de ce que le programme a fait, avec une section **Latence** pour chaque serveur (connexion, SSL, identification, aller-retour : minimum, médiane, 95e centile et maximum) et une section **Notifications** (ce qui est notifié, si la notification système est activée et disponible, quels canaux sont définis, et le dernier échec ; les adresses et les jetons ne sont jamais écrits). C'est un fichier texte que vous pouvez joindre quand vous demandez de l'aide. Vérifiez-le avant de le partager : il ne doit pas contenir votre mot de passe.

## Notifications

ngPost peut vous prévenir quand vos envois sont terminés, pour que vous n'ayez pas à surveiller la fenêtre. Cela se règle dans **Paramètres**, catégorie **Notifications**, en deux parties : **quoi notifier** (un envoi terminé, un envoi avec erreurs ou échoué, un message à la fin ou un par envoi) et **où envoyer les messages**. Dans le programme avec fenêtre, la notification système est activée dès le départ ; les autres canaux demandent une adresse.

- **Notification système** : un message depuis l'icône de la zone de notification (version bureau uniquement ; activée par défaut, vous pouvez la désactiver). Sous Windows, elle apparaît comme n'importe quelle autre notification et reste dans le centre de notifications ; si vous ne la voyez pas, vérifiez que les notifications et l'assistant de concentration (« Ne pas déranger ») de Windows ne la masquent pas.
- **Discord** : collez l'adresse d'un webhook Discord.
- **Telegram** : le jeton de votre bot et l'identifiant de la discussion qui reçoit les messages.
- **Autre webhook** : toute adresse qui accepte un POST JSON avec un `title` et un `text` (outils de type Slack, ntfy, Home Assistant).

Choisissez ce qui est notifié (terminés, erreurs ou échecs) et comment : **un message quand tous les envois sont terminés** (par défaut, idéal pour Post Auto) ou **un message après chaque envoi**. Le bouton **Envoyer un message de test** montre tout de suite si cela fonctionne.

Le message contient le nom de l'envoi, le résultat, la taille, la durée et la vitesse. Il ne contient jamais le mot de passe ni les noms des fichiers. Les adresses et le jeton sont enregistrés en clair dans le fichier de configuration, comme les autres mots de passe : gardez-le privé. Si un message ne peut pas être envoyé, une ligne du journal l'indique et l'envoi n'est pas affecté.

En ligne de commande, ou dans Docker, utilisez les options `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` et `--notify_when each|all_done`, ou les clés `notify_*` du fichier de configuration.
