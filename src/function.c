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
			return 1;
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

