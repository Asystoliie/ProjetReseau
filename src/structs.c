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

int MAX = 20;

typedef struct InfoClient InfoClient;
struct InfoClient{
	int* socketClientArray;
	int position;
	int id_mem;
	int numClient;
	char pseudoClient[30];
};

typedef struct SharedStruct SharedStruct;
struct SharedStruct{
	int nbClients;
	int socketClientArray[20];
	char listPseudo[20][30];
	int nbFichiers;
	char fichier[5000];
	char listFichiers[20][40];
};
