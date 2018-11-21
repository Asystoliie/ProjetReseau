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

	struct sembuf opp;

	/* Recuperation du semaphore*/
	key_t key = ftok("./key.txt", 42);
	int semID =semget(key,position,0666);
		if(semID==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}


	do{
		// il a bien reçus qu'il peut update !
		opp.sem_num=position;
		opp.sem_op=-1;
		opp.sem_flg=0;
		semop(semID,&opp,1);

		//dit aux autres de s'update


	}while(1);

}

void* majAffichageUti(void * tmp){
	//attend un update des autres clients
	//mise à jour client
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

	/* Recuperation du semaphore*/
	key_t key = ftok("./key.txt", 42);
	int semID =semget(key,position,0666);
		if(semID==-1){printf("Erreur semaphore \n"); exit(EXIT_FAILURE);}

	/* Réception du pseudo */
	char pseudo[20];
	if(recv(socketClientArray[position], pseudo, sizeof(pseudo), 0) == -1)
	{
		perror("Erreur à la reception du pseudo client");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", pseudo);
	//semaphore
	strcpy(sharedStruct->listPseudo[0], pseudo);

	if(envoi_tcp(socketClientArray[position],sharedStruct->listPseudo,sizeof(char)*20*30)!=0){
			perror("Erreur reception flag");
			exit(EXIT_FAILURE);
	}
	//semaphore
	struct sembuf opp;
	int flag; //flag pour savoir si le client quitte l'application
	do{
		if(reception_tcp(socketClientArray[position],&flag,sizeof(int))!=0){
			perror("Erreur reception flag");
			exit(EXIT_FAILURE);
		}
		if(flag==0){
			//déconnexion à gérer 
		}
		else{
			struct SharedStruct data;
			if(reception_tcp(socketClientArray[position],&data,sizeof(struct SharedStruct))!=0){
				perror("Erreur reception flag");
				exit(EXIT_FAILURE);
			}

			// semaphore pour modifier le fichier et prévenir


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
	key_t key;
	int id_mem;

	if((key = ftok("./key.txt", 42)) == -1){
		fprintf(stderr, "Erreur lors de l'assignation de la clé.\n");
		perror("");
		exit(EXIT_FAILURE);
	}

	if((id_mem = shmget(key, sizeof(struct SharedStruct), IPC_CREAT|0666)) == -1){
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
	int semID = semget(key,NB_CLIENT,IPC_CREAT|0666);
	if(semID==-1){perror("");printf("Erreur création sémaphore \n");return -1;}


	union semun egCtrl ;
	egCtrl.val=0; //nb de personne ayant la ressource en même temps

	for (int i = 0; i < NB_CLIENT; ++i) //
	{
		int t = semctl(semID,i,SETVAL, egCtrl);
		if(t==-1){printf("Erreur initialisation \n");return -1;}
	}

	//sémaphores ici
	sharedStruct->nbClients = 0;
	sharedStruct->nbFichiers = 0;
	//sémpahores ici
	if(shmctl(id_mem, IPC_RMID, NULL) == -1){
		perror("Erreur lors du détachement de la mémoire partagée.");
		exit(EXIT_FAILURE);
	}

	/* ------------------------------------ */

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
    memset(socketClientArray, -1, MAX);

    pthread_t* threadClientArray = malloc (MAX * sizeof(pthread_t));
    pthread_t* threadClientUpdateArray = malloc (MAX * sizeof(pthread_t));
    pthread_t* threadMajAffichageUtiArray = malloc (MAX * sizeof(pthread_t));
    struct InfoClient* infoClient;


	printf("Waiting for a connection.\n");
	if(listen(sock, NB_CLIENT) == -1) //10 client max
    {
        perror("Error listen");
        exit(EXIT_FAILURE);
    }
	pid_t pid;

    int fils=0;
	while(fils!=1){

        struct sockaddr_in saiClient;
        int position = findPos(socketClientArray);
        socketClientArray[position] = accept(sock, (struct sockaddr*)&saiClient, &socklen);
        if(socketClientArray[nbClients] == -1)
        {
            perror("Error accept");
            exit(EXIT_FAILURE);
        }
        //memoire partage + semaphore
        nbClients++;
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

	      	if(pthread_join(threadClientUpdateArray[position],NULL) != 0){
	      		printf("Erreur ! \n");
	      		exit(EXIT_FAILURE);
	      	}

	      	if(pthread_join(threadMajAffichageUtiArray[position],NULL) != 0){
	      		printf("Erreur ! \n");
	      		exit(EXIT_FAILURE);
	      	}

	     } 

	}
	//on quitte l'application de manière "propre"
	if(pid!=0 && pid != -1){
		free(threadClientArray);
	    free(threadClientUpdateArray);
	    free(threadMajAffichageUtiArray);
	}

	exit(EXIT_SUCCESS);
}
