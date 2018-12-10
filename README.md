# ProjetReseau

Compiler avec make (voir le makefile pour voir les lignes de compilation).
Lancer le serveur -> ./serveur *port*
Lancer le client -> ./client *IpServeur* *port*
La zone de texte correspond au fichier partagé par tous les utilisateurs. 
A gauche de celle-ci il y a la liste de tous les utilisateurs connectés sur le serveur.

La mise à jour du texte est réalisé de manière automatique.
La librairie pour l'interface utilisateur est GTK+.

Chaque client crée un thread qui gère la mise à jour du fichier partagé.

Le serveur créé pour chaque client un processus fils qui contient 3 threads.
Un thread s'occupe de la mise à jour du fichier de son client, un autre de la liste utilisateur de son client
et enfin le dernier s'occupe de lancer les mises à jour nécessaire à tous les clients du serveur (mise à jour 
du fichier et de la liste utilisateur). Pour cela, il alloue de la ressource aux deux autres threads de tous les clients connectés pour lancer la mise à jour.
Il s'occupe aussi de la déconnexion d'un client.

