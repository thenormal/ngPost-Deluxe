# Serveurs et connexions

Un serveur est votre fournisseur Usenet. Vous pouvez en saisir plusieurs : les envois sont téléversés sur tous les serveurs activés.

## Champs

- **Hôte** et **Port** : fournis par votre fournisseur. Les ports habituels sont 563 (ou 443) avec SSL et 119 sans.
- **SSL** : utilisez-le dès que votre fournisseur le propose. Il chiffre la connexion.
- **Nom d'utilisateur** et **Mot de passe** : votre compte.
- **Connexions** : le nombre de connexions ouvertes en parallèle. Plus de connexions peuvent aller plus vite, mais votre fournisseur a une limite (souvent de 20 à 50). La dépasser provoque des erreurs.

## Tester la connexion

Cliquez sur **Tester la connexion** avant de poster. Le test se connecte et s'identifie, et vous dit si l'hôte, le port, le réglage SSL ou les identifiants sont incorrects.

## Erreurs courantes

- *Connexion impossible* : hôte ou port incorrect, pas d'internet, ou un pare-feu ou un VPN bloque la connexion.
- *Échec de l'authentification* : nom d'utilisateur ou mot de passe incorrect. Ne réessayez que quelques fois : les fournisseurs peuvent bloquer temporairement un compte après de nombreuses connexions échouées.
- *Trop de connexions* : diminuez le nombre de connexions.

Voir aussi [Problèmes courants](12-problems.md).
