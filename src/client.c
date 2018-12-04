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

void* gestionFichier(void* tmp){
	ClientStruct* clientStruct = tmp;
	int socket = clientStruct->socket;
	int rez;
	int flag;
	do{
		printf("Thread gestion fichier en attente...\n");
		if(rez = reception_tcp(socket,&flag,sizeof(int))!=0){
			if(rez==2)
				perror("Serveur fermé !");
			else
				perror("Erreur reception flag fichier");
			exit(EXIT_FAILURE);
		}
		printf("flag = %i\n", flag);
		printf("reception !\n");
		if(flag==1){
			char fichier[5000];
			if(reception_tcp(socket,fichier,sizeof(fichier))!=0){
				perror("Erreur reception fichier");
				exit(EXIT_FAILURE);
			}

			printf("fichier = %s\n", fichier);

			strcpy(clientStruct->fichier, fichier);
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
	socklen_t lgA = sizeof(struct sockaddr_in) ;

	// Demande connexion au serveur
	printf("Tentative de connexion...\n");
	if (connect(dS,(struct sockaddr*)&aD, lgA) == -1)
	{
		perror("Erreur connect ");
		exit(EXIT_FAILURE);
	}	
	printf("Connecté.\n");

    /* Variables */
    GtkWidget * MainWindow = NULL;
    GtkWidget * MainBox = NULL;
    GtkWidget * ListBoxG = NULL;
    GtkWidget * ListBoxD = NULL;
    GdkRGBA color;
    color.red = 0.5;
    color.blue = 0.5;
    color.green = 0.5;
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
    GtkWidget *zone_files = init_files(buffer);
    gtk_container_add(GTK_CONTAINER(ListBoxD), zone_files);

    /* Initialisation du bouton update */
    char fichier[5000];
    GtkWidget *zone_update = init_update(fichier, dS);
    gtk_container_add(GTK_CONTAINER(ListBoxD), zone_update);

    /* Construction de l'interface */
    gtk_container_add(GTK_CONTAINER(MainWindow), MainBox);

    /* Choix du nom utilisateur */
    char pseudo[30];
    init_pseudo_box(MainWindow, pseudo);


    /*==================== COMMUNICATION ==================== */

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
	for(int i = 0; i < NB_CLIENT; i++){
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

	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	gtk_text_buffer_insert(buffer, &iter, fichier, -1);

	int verif = 1;
	printf("dS = %d\n", dS);
	if(send(dS, &verif, sizeof(verif), 0) ==-1){
		perror("Erreur envoie verification");
		exit(EXIT_FAILURE);
	}
	/*--------------------*/	

	/* Creation des threads */
	pthread_t* threadClientArray = malloc (2 * sizeof(pthread_t));

	// Thread reception fichier
	ClientStruct* fichierStruct = malloc(sizeof(ClientStruct));
	fichierStruct->socket = dS;
	fichierStruct->fichier = fichier;
	if(pthread_create(&threadClientArray[0], NULL, gestionFichier, fichierStruct) != 0){
  		printf("Erreur thread fichier! \n");
  		exit(EXIT_FAILURE);
  	}

	// Thread reception utilisateurs
	/*if(pthread_create(&threadClientArray[1], NULL, gestionClient, infoClient) != 0){
  		printf("Erreur thread utilisateur! \n");
  		exit(EXIT_FAILURE);
  	}*/

	/* Affichage et boucle évènementielle */
    gtk_main();

    /* On quitte.. */
    return EXIT_SUCCESS;
}

