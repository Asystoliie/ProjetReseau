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

void clientLeave(GtkWidget *widget, GdkEvent *event, gpointer ptr){
  int flag = 0;
  ClientStruct* socketStruct = ptr;
  if(envoi_tcp(socketStruct->socket, &flag, sizeof(flag)) != 0){
    perror("Erreur lors de l'envoi du flag 0 de deconnexion");
    exit(EXIT_FAILURE);
  }
  free(socketStruct);
  exit(0);
}

void clientUpdate(GtkWidget *widget, GdkEvent *event, gpointer ptr){
  ClientStruct* socketStruct = ptr;
  int socket =socketStruct->socket ;
  int flag = 1;

  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_start_iter (socketStruct->buffer, &start);
  gtk_text_buffer_get_end_iter (socketStruct->buffer, &end);
  socketStruct->fichier = gtk_text_buffer_get_text(socketStruct->buffer, &start, &end, FALSE);

  printf("FILE = %s\n", socketStruct->fichier);

  printf("SOCKET : %d\n",socket);
  if(envoi_tcp(socket, &flag, sizeof(flag)) != 0){
    perror("Erreur lors de l'envoi du flag 1 pour l'update du fichier");
    exit(EXIT_FAILURE);
  }

  if(envoi_tcp(socketStruct->socket, socketStruct->fichier, sizeof(char)*5000) != 0){
    perror("Erreur lors de l'envoi du fichier au serveur");
    exit(EXIT_FAILURE);
  }
  printf("FICHIER ENVOYE -> %s\n", socketStruct->fichier );
  printf("j'arrive ici\n");
}



GtkWidget* init_menu(ClientStruct* socketStruct){
	GtkWidget *pButton;
	GtkWidget *pVBox;
	GtkWidget *zone;

	pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	zone = gtk_fixed_new ();

	pButton = gtk_button_new_with_label("Quitter");

	gtk_box_pack_start(GTK_BOX(pVBox), pButton, TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(clientLeave), (gpointer) socketStruct);

	gtk_widget_set_size_request(pVBox, 300, 150);
	gtk_fixed_put(GTK_FIXED(zone), pVBox, 100, 100);

	return zone;
}

GtkTreeStore* init_users(GtkWidget* ListBox){
	GtkTreeStore * store_Utilisateurs = NULL;
  GtkWidget *tree_Utilisateurs = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkWidget * scrolled_window = NULL;

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scrolled_window, 400, 350);

  /* Liste utilisateurs */
  store_Utilisateurs = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
  GtkTreeIter iter1;
	GtkTreeIter iter2;

	enum
	{
	   USER_COLUMN,
	   NUMBER_COLUMN,
	   N_COLUMNS
	};

  tree_Utilisateurs = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_Utilisateurs));

  renderer = gtk_cell_renderer_text_new();
 	g_object_set (G_OBJECT (renderer),
               "foreground", "red",
               NULL);

 	column = gtk_tree_view_column_new_with_attributes ("Utilisateur", renderer,
                                                    "text", USER_COLUMN,
                                                    NULL);
 	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Utilisateurs), column);

 	gtk_container_add(GTK_CONTAINER(scrolled_window), tree_Utilisateurs);

 	GtkWidget *zone_tree;
 	zone_tree = gtk_fixed_new();
 	gtk_fixed_put(GTK_FIXED(zone_tree), scrolled_window, 50, 50);

  gtk_container_add(GTK_CONTAINER(ListBox), zone_tree);
 	
 	return store_Utilisateurs;
}

GtkWidget* init_files(GtkTextBuffer *buffer){
  GtkWidget *scrolled_window = NULL;
  GtkWidget *text_view = NULL;

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled_window, 500, 600);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 5);

  text_view = gtk_text_view_new_with_buffer (buffer);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD); 

  gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);

  GtkWidget *zone_file;
  zone_file = gtk_fixed_new ();
  gtk_fixed_put(GTK_FIXED(zone_file), scrolled_window, 175, 50);
    
  return zone_file;
}

GtkWidget* init_update(char* fichier, int socket, GtkTextBuffer* buffer){
  GtkWidget *pButton;
  GtkWidget *zone;
  GtkWidget *pVBox;

  zone = gtk_fixed_new ();
  pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  pButton = gtk_button_new_with_label("Update");
  gtk_box_pack_start(GTK_BOX(pVBox), pButton, FALSE, FALSE, 0);

  ClientStruct* socketStruct = malloc(sizeof(ClientStruct));
  socketStruct->socket = socket;
  socketStruct->fichier = fichier;
  socketStruct->buffer = buffer;
  printf("init update %d\n", socket );
  g_signal_connect(G_OBJECT(pButton), "clicked", G_CALLBACK(clientUpdate), (gpointer) socketStruct);

  gtk_widget_set_size_request(pVBox, 500, 150);
  gtk_fixed_put(GTK_FIXED(zone), pVBox, 175, 0);

  return zone;
}

void init_pseudo_box(GtkWidget* MainWindow, char* result){
  GtkWidget *pEntry;
    GtkWidget *dialog;
    GtkWidget *error;
    GtkWidget *box;
    const gchar* sNom;
    char pseudo[20];
  
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  dialog = gtk_dialog_new_with_buttons ("Saisie du nom",
                                        GTK_WINDOW(MainWindow),
                                        flags,
                                        ("OK"),
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  pEntry = gtk_entry_new();

  gtk_entry_set_text(GTK_ENTRY(pEntry), "Saisissez votre nom");
  box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_box_pack_start(GTK_BOX(box), pEntry, TRUE, FALSE, 0);

  gtk_widget_show_all(dialog);
  gtk_widget_show_all(MainWindow);

  switch (gtk_dialog_run(GTK_DIALOG(dialog)))
    {
        case GTK_RESPONSE_ACCEPT:
            sNom = gtk_entry_get_text(GTK_ENTRY(pEntry));
            strcpy(pseudo, sNom);
            if(strcmp(pseudo, "Saisissez votre nom") == 0){
              strcpy(pseudo, "default");
              printf("%s\n", pseudo);
            }
            break;

        case GTK_RESPONSE_NONE:
        default:
            strcpy(pseudo, "default");
            break;
    }
    gtk_widget_destroy(dialog);

    strcpy(result, pseudo);
}