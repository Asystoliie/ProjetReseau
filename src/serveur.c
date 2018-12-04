#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <math.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include <gtk/gtk.h>

#include "structs.c"
#include "function.c"

void init_infoClient(struct InfoClient* infoClient, int* socketClientArray, int pos, int id_mem, int numClient){
	infoClient->socketClientArray = socketClientArray;
	infoClient->position = pos;
	infoClient->id_mem = id_mem;
	infoClient->numClient = numClient;
}

/*void init_sharedStruct(struct SharedStruct* sharedStruct){
	
	sharedStruct->listPseudo = malloc(MAX *sizeof(char*));
	for(int i = 0; i < MAX; i++)
		sharedStruct->listPseudo[i] = malloc(20 * sizeof(char));
}*/

/**
*	Cherche une position libre dans le tableau de socket
*	@param socketClientArray : le tableau de sockets
*	@return renvoi l'index de la position libre
**/
int findPos(int* socketClientArray){
	for(int i = 0; i < MAX; i++){
		if (socketClientArray[i] == -1){
			return i;
		}
	}
}

void* update(void * tmp){

	InfoClient* infoClient = tmp;
	int position = infoClient->position;
	int* socketClientArray = infoClient->socketClientArray;

	struct sembuf opp;

	/* Liaison à la mémoire partagée */
	struct SharedStruct* sharedStruct = NULL;
	sharedStruct = (SharedStruct*) shmat(infoClient->id_mem, NULL, 0);
	if(sharedStruct == NULL){
		perror("Erreur lors de la liaison à la mémoire partagée");
		exit(EXIT_FAILURE);
	}

	key_t key,maj,file;
	if((file = ftok("./file.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé key.\n");
		perror("");
		exit(EXIT_FAILURE);
	}
	int semIDFile =semget(file,0,0666);
		if(semIDFile==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	/* Recuperation du semaphore*/

	if((key = ftok("./key.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé key.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	int semID =semget(key,position,0666);
		if(semID==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	if((maj = ftok("./maj.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé maj.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	//semaphore pour la maj uti
	int semIDMaj = semget(maj,MAX,0666);
	if(semIDMaj==-1){perror("");printf("Erreur sémaphore \n");exit(EXIT_FAILURE);}


	do{
		// il a bien reçus qu'il peut update !
		opp.sem_num=position;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semID,&opp,1);

		printf("ON DOIT UPDATE \n");
		opp.sem_num=position;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semIDFile,&opp,1);


		for (int i = 0; i < MAX; ++i) //
		{
			if(sharedStruct->socketClientArray[i]!=-1 && i!=position){
				printf("client %d\n", sharedStruct->socketClientArray[i]);
				opp.sem_num=i;
				opp.sem_op=1;
				opp.sem_flg=0;
				semop(semIDMaj,&opp,1);
			}
		}

		opp.sem_num=position;
		opp.sem_op=1;
		opp.sem_flg=0;
		semop(semIDFile,&opp,1);


	}while(1);

}
//attend un update des autres clients
//mise à jour client
void* majAffichageUti(void * tmp){
	InfoClient* infoClient = tmp;

	int position = infoClient->position;
	int* socketClientArray = infoClient->socketClientArray;

	/* Liaison à la mémoire partagée */
	struct SharedStruct* sharedStruct = NULL;
	sharedStruct = (SharedStruct*) shmat(infoClient->id_mem, NULL, 0);
	if(sharedStruct == NULL){
		perror("Erreur lors de la liaison à la mémoire partagée");
		exit(EXIT_FAILURE);
	}


	key_t file,maj;

	if((maj = ftok("./maj.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé maj.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	//semaphore pour la maj uti


	int semIDMaj = semget(maj,MAX,0666);
	if(semIDMaj==-1){perror("");printf("Erreur sémaphore \n");exit(EXIT_FAILURE);}

	if((file = ftok("./file.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé key.\n");
		perror("");
		exit(EXIT_FAILURE);
	}
	int semIDFile =semget(file,0,0666);
		if(semIDFile==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	struct sembuf opp;

	do{

		opp.sem_num=position;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semIDMaj,&opp,1);

		opp.sem_num=0;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semIDFile,&opp,1);

		printf("je passe ici ?\n");

		int flag = 1; //envoie d'une update fichier
		if(envoi_tcp(socketClientArray[position], &flag, sizeof(int)) ==-1){
			perror("Erreur envoie verification");
			exit(EXIT_FAILURE);
		}

		if(envoi_tcp(socketClientArray[position],sharedStruct->fichier,sizeof(sharedStruct->fichier))!=0){
			perror("Erreur reception flag");
			exit(EXIT_FAILURE);
		}

		opp.sem_num=0;
		opp.sem_op=1;
		opp.sem_flg=0;
		semop(semIDFile,&opp,1);

	}while(1);

}

void* gestionClient(void* tmp){
	InfoClient* infoClient = tmp;
	printf("Gestion client numéro %i\n", infoClient->numClient);

	int position = infoClient->position;
	int* socketClientArray = infoClient->socketClientArray;

	/* Liaison à la mémoire partagée */
	struct SharedStruct* sharedStruct = NULL;
	sharedStruct = (SharedStruct*) shmat(infoClient->id_mem, NULL, 0);
	if(sharedStruct == NULL){
		perror("Erreur lors de la liaison à la mémoire partagée");
		exit(EXIT_FAILURE);
	}

	/* Recuperation du semaphore pour l'update */
	key_t key = ftok("./key.txt", 42);
	int semID =semget(key,position,0666);
		if(semID==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	/* Recuperation du semaphore pour la mémoire partagé */
	key_t file = ftok("./file.txt", 42);
	int semIDFile =semget(file,0,0666);
		if(semID==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	/* Réception du pseudo */
	char pseudo[30];
	if(recv(socketClientArray[position], pseudo, sizeof(pseudo), 0) == -1)
	{
		perror("Erreur à la reception du pseudo client");
		exit(EXIT_FAILURE);
	}
	struct sembuf opp;
	printf("%s\n", pseudo);
	//semaphore
	opp.sem_num=0;
	opp.sem_op=-1;
	opp.sem_flg=0;
	semop(semIDFile,&opp,1);

	strcpy(sharedStruct->listPseudo[position], pseudo);


	//envoi liste pseudo 
	if(envoi_tcp(socketClientArray[position],sharedStruct->listPseudo,sizeof(char)*10*30)!=0){
			perror("Erreur envoi liste pseudo");
			exit(EXIT_FAILURE);
	}

    //envoi du fichier à la 1er connexion
    if(envoi_tcp(socketClientArray[position],sharedStruct->fichier,sizeof(sharedStruct->fichier))!=0){
		perror("Erreur reception flag");
		exit(EXIT_FAILURE);
	}

	int flag_autre = 2 ; 
	for (int i = 0; i < MAX; ++i)
	{
		if(i!=position && sharedStruct->socketClientArray[i]!=-1){
			if(envoi_tcp(sharedStruct->socketClientArray[i],&flag_autre,sizeof(int))!=0){
				perror("Erreur envoi flag_autre 2");
				exit(EXIT_FAILURE);
			}

			if(envoi_tcp(sharedStruct->socketClientArray[i],sharedStruct->listPseudo,sizeof(char)*10*30)!=0){
				perror("Erreur envoi liste pseudo");
				exit(EXIT_FAILURE);
			}
		}
	}



	opp.sem_num=0;
	opp.sem_op=1;
	opp.sem_flg=0;
	semop(semIDFile,&opp,1);

	int verif = 0;
	if(reception_tcp(socketClientArray[position],&verif,sizeof(verif))!=0){
		perror("Erreur reception verification");
		exit(EXIT_FAILURE);
	}

	if(verif == 0){
		perror("verification failed.");
		exit(EXIT_FAILURE);
	}
	printf("Success verification = %d\n", verif);

	//semaphore

	int flag; //flag pour savoir si le client quitte l'application
	do{
		if(reception_tcp(socketClientArray[position],&flag,sizeof(int))!=0){
			perror("Erreur reception flag");
			exit(EXIT_FAILURE);
		}
		printf("flag = %i\n", flag);
		if(flag==0){ //déconnexion !
			printf("deco !\n");
			opp.sem_num=0;
			opp.sem_op=-1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);

			sharedStruct->nbClients--;
			sharedStruct->socketClientArray[position]=-1;
			memset(sharedStruct->listPseudo[position], 0, sizeof (30));

			opp.sem_num=0;
			opp.sem_op=1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);
		}
		else{
			char fichier[5000];
			if(reception_tcp(socketClientArray[position],fichier,sizeof(fichier))!=0){
				perror("Erreur reception fichier");
				exit(EXIT_FAILURE);
			}

			// semaphore pour modifier le fichier
			opp.sem_num=0;
			opp.sem_op=-1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);

			strcpy(sharedStruct->fichier, fichier);

	        opp.sem_num=0;
			opp.sem_op=1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);

			//semaphore pour dire aux autres de s'update
			//On donne une ressource à l'update
			opp.sem_num=position;
			opp.sem_op=1;
			opp.sem_flg=0;
			semop(semID,&opp,1);

			//On attend la fin de l'update
			opp.sem_num=position;
			opp.sem_op=0;
			opp.sem_flg=0;
			semop(semID,&opp,1);
			printf("%s\n", fichier);
		}

	}while(flag==1);

}

int main(int argc, char* argv[]){
	
	/* Conditions */
	int port;
	if(argc != 2)
    {
    	printf("Invalid syntax.\n");
        printf("./serveur port\n");
   		exit(EXIT_FAILURE); 
    }
    else
    {
        port = atoi(argv[1]);
    }

	/* Init shared memory*/
	key_t key, file, maj;
	int id_mem;

	//clef pour le sémaphore du fichier
	if((file = ftok("./file.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	/* --------------------------------------------------------------------------------------------------------- */

	//clef pour la sémaphore qui dit aux autres de s'update 
	if((key = ftok("./key.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	if((id_mem = shmget(file, sizeof(struct SharedStruct), IPC_CREAT|0666)) == -1){
		fprintf(stderr, "Erreur lors de la création de la mémoire.(Mémoire possiblement déjà existante)\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	struct SharedStruct* sharedStruct = NULL;
	sharedStruct = (SharedStruct*) shmat(id_mem, NULL, 0);
	if(sharedStruct == NULL){
		perror("Erreur lors de la liaison à la mémoire partagée");
		exit(EXIT_FAILURE);
	}

	//semaphore pour l'update 
	int semID = semget(key,MAX,IPC_CREAT|0666);
	if(semID==-1){perror("");printf("Erreur création sémaphore \n");return -1;}


	union semun egCtrl ;
	egCtrl.val=0; //nb de personne ayant la ressource en même temps

	for (int i = 0; i < MAX; ++i) //
	{
		int t = semctl(semID,i,SETVAL, egCtrl);
		if(t==-1){printf("Erreur initialisation \n");return -1;}
	}

	/* --------------------------------------------------------------------------------------------------------- */

	//semaphore pour le file donc des données partagées
	int semIDFile = semget(file,1,IPC_CREAT|0666);
	if(semIDFile==-1){perror("");printf("Erreur création sémaphore \n");return -1;}

	egCtrl.val=1;

	int t = semctl(semIDFile,0,SETVAL, egCtrl);
	if(t==-1){printf("Erreur initialisation \n");return -1;}

	/* --------------------------------------------------------------------------------------------------------- */


	//clef pour le semaphore pour la maj uti
	if((maj = ftok("./maj.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	//semaphore pour la maj uti
	int semIDMaj = semget(maj,MAX,IPC_CREAT|0666);
	if(semIDMaj==-1){perror("");printf("Erreur création sémaphore \n");return -1;}


	egCtrl.val=0; //nb de personne ayant la ressource en même temps

	for (int i = 0; i < MAX; ++i) //
	{
		int t = semctl(semIDMaj,i,SETVAL, egCtrl);
		if(t==-1){printf("Erreur initialisation \n");return -1;}
	}

	/* --------------------------------------------------------------------------------------------------------- */

	//Initialisation de la mémoire partagée ( pas besoin de sémaphore ici )
	sharedStruct->nbClients = 0;
	strcpy(sharedStruct->fichier, "Votre message ici");

	/* --------------------------------------------------------------------------------------------------------- */

	/* Init sockets */
	int sock;
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sai;
    socklen_t socklen = sizeof(struct sockaddr_in);
    sai.sin_family = AF_INET;
    sai.sin_addr.s_addr = INADDR_ANY;
    sai.sin_port = htons(port);
    if(bind(sock, (struct sockaddr*)&sai, socklen) == -1)
    {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    int nbClients = 0; // limiter à 10
    int* socketClientArray = malloc(MAX * sizeof(int));

    for (int i = 0; i < MAX; ++i)
    {
    	memset(sharedStruct->listPseudo[i],0,sizeof(30));
    	socketClientArray[i]=-1;
    	sharedStruct->socketClientArray[i]=-1;
    }

    pthread_t* threadClientArray = malloc (MAX * sizeof(pthread_t));
    pthread_t* threadClientUpdateArray = malloc (MAX * sizeof(pthread_t));
    pthread_t* threadMajAffichageUtiArray = malloc (MAX * sizeof(pthread_t));
    struct InfoClient* infoClient;


	printf("Waiting for a connection.\n");
	if(listen(sock, 2) == -1) //10 client max
    {
        perror("Error listen");
        exit(EXIT_FAILURE);
    }
	pid_t pid;
	struct sembuf opp;
    int fils=0;
	while(fils!=1){
		int dSclient;
		int flag; 
		struct sockaddr_in saiClient;

		dSclient= accept(sock, (struct sockaddr*)&saiClient, &socklen);

 	    opp.sem_num=0;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semIDFile,&opp,1);
	
		if(sharedStruct->nbClients>=MAX){
			opp.sem_num=0;
			opp.sem_op=1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);

			flag = 1; //trop de client connecté
			if(envoi_tcp(dSclient,&flag,sizeof(int))!=0){
				perror("Erreur envoi flag connection");
				exit(EXIT_FAILURE);
			}
		}
		else{

			flag = 0; //trop de client connecté
			if(envoi_tcp(dSclient,&flag,sizeof(int))!=0){
				perror("Erreur envoi flag connection");
				exit(EXIT_FAILURE);
			}


	        int position = findPos(socketClientArray);
	        socketClientArray[position]=dSclient;
	        if(socketClientArray[position] == -1)
	        {
	            perror("Error accept");
	            exit(EXIT_FAILURE);
	        }

			printf("CLIENT %i POSITION %i\n",socketClientArray[position], position );
			sharedStruct->socketClientArray[position]=socketClientArray[position];

			for (int i = 0; i < MAX; ++i)
			{
				socketClientArray[i]=sharedStruct->socketClientArray[i];
			}

	        sharedStruct->nbClients++;
	        nbClients=sharedStruct->nbClients++;

	        opp.sem_num=0;
			opp.sem_op=1;
			opp.sem_flg=0;
			semop(semIDFile,&opp,1);
	        //memoire partage + semaphore

	        printf("New connection : %s\n", inet_ntoa((struct in_addr)saiClient.sin_addr));
	        pid = fork();
	        if(pid==-1){ //erreur
	        	printf("ERROR FORK\n");
	        	exit(EXIT_FAILURE);
	        } else if(pid==0){ //fils
	        	fils=1;
		        /* Création du thread pour le client */
		        infoClient = malloc(sizeof(struct InfoClient));
		        init_infoClient(infoClient, socketClientArray, position, id_mem, nbClients);

		      	if(pthread_create(&threadClientArray[position], NULL, gestionClient, infoClient) != 0){
		      		printf("Erreur ! \n");
		      		exit(EXIT_FAILURE);
		      	}

		      	if(pthread_create(&threadClientUpdateArray[position], NULL, update, infoClient) != 0){
		      		printf("Erreur ! \n");
		      		exit(EXIT_FAILURE);
		      	}

		      	if(pthread_create(&threadMajAffichageUtiArray[position], NULL, majAffichageUti, infoClient) != 0){
		      		printf("Erreur ! \n");
		      		exit(EXIT_FAILURE);
		      	}

		      	if(pthread_join(threadClientArray[position],NULL) != 0){
		      		printf("Erreur ! \n");
		      		exit(EXIT_FAILURE);
		      	}

		     } 
		 }

	}
	//on quitte l'application de manière "propre"
	if(pid!=0 && pid != -1){

		if(shmctl(id_mem, IPC_RMID, NULL) == -1){
			perror("Erreur lors du détachement de la mémoire partagée.");
			exit(EXIT_FAILURE);
		}

		free(threadClientArray);
	    free(threadClientUpdateArray);
	    free(threadMajAffichageUtiArray);
	}

	exit(EXIT_SUCCESS);
}
