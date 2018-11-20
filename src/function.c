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

#include <gtk/gtk.h>


/** GESTION DU FLAG NUMERO 2
*
*/
void flag2(InfoClient* infoClient, SharedStruct* sharedStruct, char* pseudo){

	int flag, recep;
	char pseudoEnvoi[30];

	for(int i = 0; i < MAX; i++){

		if(i != infoClient->position){
			/* ENVOIE DU FLAG + RECEPTION */
			flag = 2;
			if(send(sharedStruct->socketClientArray[i], &flag, sizeof(flag), 0) == -1)
			{
				perror("Erreur lors de l'envoi du flag numéro 2");
				exit(EXIT_FAILURE);
			}

			recep = 0;
			if(recv(sharedStruct->socketClientArray[i], &recep, sizeof(recep), 0) == -1)
			{
				perror("Erreur lors de la reception de la confirmatio du flag");
				exit(EXIT_FAILURE);
			}

			if(recep != 1){
				perror("Erreur dans le message de confirmation : recep != 1");
				exit(EXIT_FAILURE);
			}
			/*----------------------------*/

			strcpy(pseudoEnvoi, pseudo);
			if(send(sharedStruct->socketClientArray[i], pseudoEnvoi, sizeof(pseudoEnvoi), 0) == -1)
			{
				perror("Erreur lors de l'envoie du pseudo à l'utilisateur");
				exit(EXIT_FAILURE);
			}
		}
	}
}

