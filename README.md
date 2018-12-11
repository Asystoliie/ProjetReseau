# ProjetReseau

Note : Le projet utilise la bibliothèque graphique GTK version 3.0, il est donc nécessaire d'avoir (si possible) la dernière version de cette librairie d'installée pour compiler le programme. (Version déjà installée sur les ordinateurs de la facultée des sciences)

Compiler avec make (voir le makefile pour plus d'informations sur la compilation de GTK).

Lancer le serveur -> ./serveur *port*
Lancer le client -> ./client *IpServeur* *port*


Manuel d'utilisation:

Lors du lancement du programme, il vous sera demander de vous identifier. Dans le cas où votre serait vide ou qu'il contiendrait des espaces, un pseudo par defaut vous sera attribuer.
Ensuite, vous pourrez aperçevoir deux zones. La première, celle de gauche, correspond à la liste des utilisateurs connectés au serveur. Elle est mise à jour en temps réél lorsqu'un nouvel utilisateur rejoint le fichier
partagé ou qu'il le quitte.
La seconde zone, à droite, est une zone de texte correspondant au fichier partagé. Bien qu'il soit mis à jour toutes les 4 secondes environ, il prend en compte tous les caractères tapés dés leur saisie et les envoie côté serveur, il n'est donc
pas nécessaire de valider ou d'enregistrer le fichier pour que les modifications soient prise en compte par tous les autres utilisateurs. Malheureusement, la librairie utilisée n'étant pas plainement optimisée pour du traitement réseau, il est possible que le dernier caractère saisie soit effacé lors de l'écriture à cause de la mise à jour régulière de l'affichage. Malgré tout, pas d'inquiètude, le fichier reste intacte à l'arrivée et n'est pas pris en compte par les autres utilisateurs.
Finalement, il est possible de quitter le logiciel grâce au bouton "quitter" en bas à gauche de l'écran, ou tout simplement en fermant la fenêtre.

Fonctionnement logiciel:

	Client :
		Chaque client initialise avant toute connection au server son interface graphique (boutons, fenêtre, zone de saisie, etc...). La connection au serveur ne s'effectue que lorsque l'utilisateur valide son pseudo et qu'il est vérifié par le logiciel. Ensuite, le client initialise un thread qui tournera en boucle tant que le client restera connecté. Il aura pour but de reçevoir les modifications côté serveur tel que les mises à jours du fichier partagé ou encore l'arrivée de nouveaux utilisateurs. Cette gestion est réalisée par des flags distinctifs basé sur le code suivant : flag 0 = deconnexion du client, flag 1 = maj fichier, flag 2 = nouvel utilisateur.

	Serveur :
		A l'initialisation, le serveur met en place la mémoire partagée puis se met en attente d'une connexion utilisateur grâce à un listen(). Lorsqu'un nouveau client se connecte, un processus fils est créé puis il crée à son tour 3 threads.
		Un thread s'occupe de la mise à jour du fichier de son client, un autre de la liste utilisateur de son client et enfin le dernier s'occupe de lancer les mises à jour nécessaire à tous les clients du serveur (mise à jour du fichier et de la liste utilisateur). Pour cela, il alloue de la ressource aux deux autres threads clients bloqués par le biais de semaphores avant de lancer la mise à jour. C'est d'ailleurs aussi lui qui s'occupe de la deconnection d'une client 
		(fameux flag 0). A noter que la liaison à la mémoire partagée se fait lors du lancement de chaque thread, chacun des thread y à donc accés.

ATTENTION !
A priori, selon la distribution de linux utilisée, il est possible de rencontrer des problèmes lors de la compilation voir des problèmes lors de l'execution du programme (divers plantages par exemple), il est donc très recommandée d'avoir une version à jour de GTK ainsi que toutes les bibliothèques dépendantes pour de bonne conditions d'utilisations du programme.
Malgré tout, le programme a été testé en quasi totalité sur les ordinateurs de la faculté des sciences. Autrement dit, aucun problème lié à l'utilisation de GTK ne devrait être rencontré.

