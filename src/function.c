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

int reception_tcp(int destinataire, void * msg, int taille_msg)
{
	int nb_octet_recu=0;
	int rez;
	while(nb_octet_recu!=taille_msg)
	{
		rez= recv(destinataire, msg+nb_octet_recu, taille_msg-nb_octet_recu, 0);
		if(rez<0)
		{
			printf("PROBLEME RECEPTION FONCTION\n");
			return 1;
		}
		else if(rez==0)
		{
			printf("Socket ferme\n");
			return 2;
		}
		nb_octet_recu= nb_octet_recu+rez;
	}
	return 0;
}

int envoi_tcp(int destinataire,const void * msg, int taille_msg)
{
	int nb_octet_envoi=0;
	int rez;
	while(nb_octet_envoi!=taille_msg)
	{
		rez = send(destinataire, msg+nb_octet_envoi, taille_msg-nb_octet_envoi, 0);
		if(rez<0)
		{
			printf("PROBLEME ENVOI FONCTION\n");
			exit(1);
		}
		else if(rez==0)
		{
			printf("Socket ferme\n");
			return 2;
		}

		nb_octet_envoi=nb_octet_envoi+rez;
	}
	return 0;
}

