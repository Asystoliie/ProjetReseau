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

#include "structs.c"
#include "function.c"
#include "function_init_client.c"

static gboolean time_handler(gpointer ptr) {
	ClientStruct* socketStruct = ptr;
    gtk_text_buffer_set_text(socketStruct->buffer, socketStruct->fichier, strlen(socketStruct->fichier));
    return TRUE;
}

void* gestionFichier(void* tmp){
	ClientStruct* clientStruct = tmp;
	int socket = clientStruct->socket;
	int rez;
	int flag;
	do{
		if(rez = reception_tcp(socket,&flag,sizeof(int))!=0){
			if(rez==2)
				perror("Serveur fermé !");
			else
				perror("Erreur reception flag fichier");
			exit(EXIT_FAILURE);
		}

		if(flag==1){
			char fichier[SIZEMAXFICHIER];
			if(reception_tcp(socket,fichier,sizeof(fichier))!=0){
				perror("Erreur reception fichier");
				exit(EXIT_FAILURE);
			}

			strcpy(clientStruct->fichier, fichier);
		}
		if(flag==2){
			char listPseudo[10][30];
			if(reception_tcp(socket,listPseudo,sizeof(char)*10*30)!=0){
				perror("Erreur reception listPseudo");
				exit(EXIT_FAILURE);
			}

			enum
			{
			   USER_COLUMN,
			   NUMBER_COLUMN,
			   N_COLUMNS
			};

			gtk_tree_store_clear(clientStruct->store_Utilisateurs);

			GtkTreeIter iter1;
			for(int i = 0; i < MAX; i++){
				if(strlen(listPseudo[i]) != 0){
					gtk_tree_store_append (clientStruct->store_Utilisateurs, &iter1, NULL);
					gtk_tree_store_set (clientStruct->store_Utilisateurs, &iter1,
				                    USER_COLUMN, listPseudo[i],
				                    NUMBER_COLUMN, 0,
				                    -1);
				}
			}
		}
	}while(flag!=0);
}


int main(int argc, char **argv)
{
    /* Conditions */
	int port;
	char adresseIP[16];
	if (argc != 3)
	{
		printf("Invalid syntax.\n");
        printf("./client ip port\n");
   		exit(EXIT_FAILURE); 
	}
	else
	{
	 	port = atoi(argv[2]);
	 	strcpy(adresseIP, argv[1]);
	}

	// Création de la socket
	int dS;
	if((dS = socket(PF_INET, SOCK_STREAM, 0))== -1)
	{
		perror("Erreur lors de la création de la socket ");
		exit(EXIT_FAILURE);
	}

	// Initialisation structure socket
	struct sockaddr_in aD;
	aD.sin_family = AF_INET ;

	// Stockage ip dans la struct
	if(inet_pton(AF_INET,adresseIP,&(aD.sin_addr)) == -1)
	{
		perror("Erreur lors du stockage de l'IP dans la struct ");
		exit(EXIT_FAILURE);
	}

	// Passage port au format reseau
	aD.sin_port = htons(port);
	socklen_t lgA = sizeof(struct sockaddr_in);

    /* Variables */
    GtkWidget * MainWindow = NULL;
    GtkWidget * MainBox = NULL;
    GtkWidget * ListBoxG = NULL;
    GtkWidget * ListBoxD = NULL;
    GdkRGBA color;
    color.red = 0.6;
    color.blue = 0.6;
    color.green = 0.6;
    color.alpha = 1.0;
    

    /* Initialisation de GTK+ */
    gtk_init(&argc, &argv);

    /* Initde la fenêtre */
    MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(MainWindow), "Pannel de connexion");
    gtk_window_resize (GTK_WINDOW(MainWindow), 1000, 800);
    gtk_window_set_position(GTK_WINDOW(MainWindow), GTK_WIN_POS_CENTER);
	gtk_widget_override_background_color(MainWindow, GTK_STATE_NORMAL, &color);

    ClientStruct* socketStruct = malloc(sizeof(ClientStruct));
    socketStruct->socket = dS;
    g_signal_connect(G_OBJECT(MainWindow), "delete-event", G_CALLBACK(clientLeave), (gpointer) socketStruct);
    

    MainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(MainBox), 5);
    gtk_widget_set_size_request(MainBox, 1200, 700);
    gtk_widget_show(MainBox);

    ListBoxG = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(ListBoxG), 5);
    gtk_widget_set_size_request(ListBoxG, 450, 400);
    gtk_widget_show(ListBoxG);
    gtk_container_add(GTK_CONTAINER(MainBox), ListBoxG);

    ListBoxD = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(ListBoxD), 5);
    gtk_widget_set_size_request(ListBoxD, 450, 400);
    gtk_widget_show(ListBoxD);
    gtk_container_add(GTK_CONTAINER(MainBox), ListBoxD);


    /* Initialisation des utilisateurs */
    GtkTreeStore *store_Utilisateurs = init_users(ListBoxG);

    /* Initialisation du menu */
    GtkWidget *zone_menu = init_menu(socketStruct);
    gtk_container_add(GTK_CONTAINER(ListBoxG), zone_menu);


    GtkTextBuffer *buffer = gtk_text_buffer_new (NULL);
    /* Initialisation des fichiers */
    char fichier[SIZEMAXFICHIER];
    ClientStruct* fichierStruct = malloc(sizeof(ClientStruct));
    fichierStruct->socket = dS;
	fichierStruct->fichier = fichier;
	fichierStruct->buffer = buffer;
	fichierStruct->store_Utilisateurs = store_Utilisateurs;

	GtkTextIter start;
    GtkTextIter end;
	fichierStruct->start = start;
	fichierStruct->end = end;
    GtkWidget *zone_files = init_files(fichierStruct);
    gtk_container_add(GTK_CONTAINER(ListBoxD), zone_files);

    /* Construction de l'interface */
    gtk_container_add(GTK_CONTAINER(MainWindow), MainBox);

    /* Choix du nom utilisateur */
    char pseudo[30];
    init_pseudo_box(MainWindow, pseudo);


    /*==================== COMMUNICATION ==================== */

    // Demande connexion au serveur
	printf("Tentative de connexion...\n");
	if (connect(dS,(struct sockaddr*)&aD, lgA) == -1)
	{
		perror("Erreur connect ");
		exit(EXIT_FAILURE);
	}
	printf("Connecté.\n");

	char flag_connexion;
	if(reception_tcp(dS,&flag_connexion,sizeof(int))!=0){
		perror("Erreur reception listPseudo");
		exit(EXIT_FAILURE);
	}

	if(flag_connexion != 0){
		perror("Erreur connexion serveur (possiblement plein !)");
		exit(EXIT_FAILURE);
	}

    /* Envoie du pseudo + affichage des utilisateurs*/
    if(send(dS, pseudo, sizeof(pseudo), 0) == -1)
	{
		perror("Erreur lors de l'envoi du pseudo utilisateur");
		exit(EXIT_FAILURE);
	}

	char listPseudo[10][30];
	if(reception_tcp(dS,listPseudo,sizeof(char)*10*30)!=0){
		perror("Erreur reception listPseudo");
		exit(EXIT_FAILURE);
	}

	enum
	{
	   USER_COLUMN,
	   NUMBER_COLUMN,
	   N_COLUMNS
	};

	GtkTreeIter iter1;
	for(int i = 0; i < MAX; i++){
		if(strlen(listPseudo[i]) != 0){
			gtk_tree_store_append (store_Utilisateurs, &iter1, NULL);
			gtk_tree_store_set (store_Utilisateurs, &iter1,
		                    USER_COLUMN, listPseudo[i],
		                    NUMBER_COLUMN, 0,
		                    -1);
		}
	}
	/*--------------------*/

	/* Reception du buffer*/
	if(reception_tcp(dS,fichier,sizeof(fichier))!=0){
		perror("Erreur reception du fichier");
		exit(EXIT_FAILURE);
	}

	gtk_text_buffer_set_text(buffer, fichier, -1);

	int verif = 1;
	if(send(dS, &verif, sizeof(verif), 0) ==-1){
		perror("Erreur envoie verification");
		exit(EXIT_FAILURE);
	}
	/*--------------------*/	

	/* Creation des threads */
	pthread_t* threadClientArray = malloc (2 * sizeof(pthread_t));

	// Thread reception fichier
	if(pthread_create(&threadClientArray[0], NULL, gestionFichier, fichierStruct) != 0){
  		printf("Erreur thread fichier! \n");
  		exit(EXIT_FAILURE);
  	}

  	g_timeout_add(3000, (GSourceFunc) time_handler, (gpointer) fichierStruct);

	/* Affichage et boucle évènementielle */
    gtk_main();

    /* On quitte.. */
    return EXIT_SUCCESS;
}

