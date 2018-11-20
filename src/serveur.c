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

	/* Réception du pseudo */
	char pseudo[20];
	if(recv(socketClientArray[position], pseudo, sizeof(pseudo), 0) == -1)
	{
		perror("Erreur à la reception du pseudo client");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", pseudo);
	//AJOUTER DES SEMAHPORES ICI POUR L'ACCES A LA MEMOIRE PARTAGÉE
	strcpy(sharedStruct->listPseudo[0], pseudo);

	/** FLAG NUMERO 2
	*	Lorsqu'un nouvel utilisateur arrive sur le serveur, il doit mettre à jour tous
	*   les autres clients connectés sur le serveur
	*/
	// semaphores ici
	flag2(infoClient, sharedStruct, pseudo);
	// semaphores ici

	printf("test\n");
	printf("%s\n", sharedStruct->listPseudo[0]);


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

    int nbClients = 0;
    int* socketClientArray = malloc(MAX * sizeof(int));
    memset(socketClientArray, -1, MAX);

    pthread_t* threadClientArray = malloc (MAX * sizeof(pthread_t));
    struct InfoClient* infoClient;

	while(1){

		printf("Waiting for a connection.\n");
		if(listen(sock, 10) == -1)
        {
            perror("Error listen");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in saiClient;
        int position = findPos(socketClientArray);
        socketClientArray[position] = accept(sock, (struct sockaddr*)&saiClient, &socklen);
        if(socketClientArray[nbClients] == -1)
        {
            perror("Error accept");
            exit(EXIT_FAILURE);
        }
        nbClients++;

        /* Création du thread pour le client */
        infoClient = malloc(sizeof(struct InfoClient));
        init_infoClient(infoClient, socketClientArray, position, id_mem, nbClients);
        pthread_create(&threadClientArray[position], NULL, gestionClient, infoClient);

        printf("New connection : %s\n", inet_ntoa((struct in_addr)saiClient.sin_addr));
	}

	exit(EXIT_SUCCESS);
}
