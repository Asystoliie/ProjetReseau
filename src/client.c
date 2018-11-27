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

	/* Réception de la clé mémoire + connexion*/
	/*int id_mem;
	if(recv(dS, &id_mem, sizeof(id_mem), 0) == -1)
	{
		perror("Erreur à la reception du message client");
		exit(EXIT_FAILURE);
	}
	printf("clé mémoire = %i\n", id_mem);*/

    /* Variables */
    GtkWidget * MainWindow = NULL;
    GtkWidget * MainBox = NULL;
    GtkWidget * ListBox = NULL;
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

    printf("socket = %i\n", dS);
    g_signal_connect(G_OBJECT(MainWindow), "delete-event", G_CALLBACK(clientLeave), (gpointer) &dS);
    

    MainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(MainBox), 5);
    gtk_widget_set_size_request(MainBox, 1200, 700);
    gtk_widget_show(MainBox);

    ListBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(ListBox), 5);
    gtk_widget_set_size_request(ListBox, 450, 400);
    gtk_widget_show(ListBox);
    gtk_container_add(GTK_CONTAINER(MainBox), ListBox);


    /* Initialisation des utilisateurs */
    GtkTreeStore *store_Utilisateurs = init_users(ListBox);

    /* Initialisation du menu */
    GtkWidget *zone_menu = init_menu();
    gtk_container_add(GTK_CONTAINER(ListBox), zone_menu);

    /* Initialisation des fichiers */
    GtkWidget *zone_files = init_files();
    gtk_container_add(GTK_CONTAINER(MainBox), zone_files);
    gtk_container_add(GTK_CONTAINER(MainWindow), MainBox);

    /* Choix du nom utilisateur */
    char pseudo[20];
    init_pseudo_box(MainWindow, pseudo);


    /*==================== COMMUNICATION ==================== */

    /* Envoie du pseudo */
    if(send(dS, pseudo, sizeof(pseudo), 0) == -1)
	{
		perror("Erreur lors de l'envoi du pseudo utilisateur");
		exit(EXIT_FAILURE);
	}

	enum
	{
	   USER_COLUMN,
	   NUMBER_COLUMN,
	   N_COLUMNS
	};

	GtkTreeIter iter1;

	gtk_tree_store_append (store_Utilisateurs, &iter1, NULL);
	gtk_tree_store_set (store_Utilisateurs, &iter1,
                    USER_COLUMN, pseudo,
                    NUMBER_COLUMN, 2,
                    -1);
	/*--------------------*/

	printf("test\n");


	/* Affichage et boucle évènementielle */
    gtk_main();

    /* On quitte.. */
    return EXIT_SUCCESS;
}

