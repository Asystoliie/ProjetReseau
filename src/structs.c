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

int MAX = 10;
int SIZEMAXFICHIER = 5000;

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
	int socketClientArray[10];
	char listPseudo[10][30];
	char fichier[5000];
};

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

typedef struct ClientStruct ClientStruct;
struct ClientStruct{
	int socket;
	char* fichier;
	GtkTextBuffer* buffer;
	GtkTreeStore* store_Utilisateurs;
};

