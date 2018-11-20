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

GtkWidget* init_menu(){
	GtkWidget *pButton[3];
	GtkWidget *pVBox;
	GtkWidget *zone;

	pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	zone = gtk_fixed_new ();

	pButton[0] = gtk_button_new_with_label("Nouveau");
	pButton[1] = gtk_button_new_with_label("Rejoindre");
	pButton[2] = gtk_button_new_with_label("Quitter");

	gtk_box_pack_start(GTK_BOX(pVBox), pButton[0], TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(pVBox), pButton[1], TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(pVBox), pButton[2], TRUE, TRUE, 0);

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

	gtk_tree_store_append (store_Utilisateurs, &iter1, NULL);
	gtk_tree_store_set (store_Utilisateurs, &iter1,
                    USER_COLUMN, "Robert",
                    NUMBER_COLUMN, 1,
                    -1);

	gtk_tree_store_append (store_Utilisateurs, &iter1, NULL);
	gtk_tree_store_set (store_Utilisateurs, &iter1,
                    USER_COLUMN, "Julie",
                    NUMBER_COLUMN, 2,
                    -1);

  tree_Utilisateurs = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_Utilisateurs));

  renderer = gtk_cell_renderer_text_new();
 	g_object_set (G_OBJECT (renderer),
               "foreground", "red",
               NULL);

 	column = gtk_tree_view_column_new_with_attributes ("Utilisateur", renderer,
                                                    "text", USER_COLUMN,
                                                    NULL);
 	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Utilisateurs), column);

 	column = gtk_tree_view_column_new_with_attributes ("Numéro", renderer,
                                                    "text", NUMBER_COLUMN,
                                                    NULL);
 	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Utilisateurs), column);

 	gtk_container_add(GTK_CONTAINER(scrolled_window), tree_Utilisateurs);

 	GtkWidget *zone_tree;
 	zone_tree = gtk_fixed_new ();
 	gtk_fixed_put(GTK_FIXED(zone_tree), scrolled_window, 50, 50);

  gtk_container_add(GTK_CONTAINER(ListBox), zone_tree);
 	
 	return store_Utilisateurs;
}

GtkWidget* init_files(){
  GtkTreeStore * store_Files = NULL;
  GtkWidget *tree_Files = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkWidget * scrolled_window = NULL;

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scrolled_window, 500, 600);

  /* Liste utilisateurs */
  store_Files = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
  GtkTreeIter iter1;

  enum
  {
     NAME_COLUMN,
     NUMBER_COLUMN,
     OWNER_COLUMN,
     N_COLUMNS
  };

  gtk_tree_store_append (store_Files, &iter1, NULL);
  gtk_tree_store_set (store_Files, &iter1,
                    NAME_COLUMN, "Récapitulatif réunion 1",
                    NUMBER_COLUMN, 1,
                    OWNER_COLUMN, "Robert",
                    -1);

  gtk_tree_store_append (store_Files, &iter1, NULL);
  gtk_tree_store_set (store_Files, &iter1,
                    NAME_COLUMN, "Récapitulatif réunion 2",
                    NUMBER_COLUMN, 6,
                    OWNER_COLUMN, "Pierre",
                    -1);

    tree_Files = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_Files));

    renderer = gtk_cell_renderer_text_new();
    g_object_set (G_OBJECT (renderer),
                 "foreground", "black",
                 NULL);

    column = gtk_tree_view_column_new_with_attributes ("Nom", renderer,
                                                      "text", NAME_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Files), column);

    column = gtk_tree_view_column_new_with_attributes ("Nombre de participants", renderer,
                                                      "text", NUMBER_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Files), column);

    column = gtk_tree_view_column_new_with_attributes ("Propriétaire", renderer,
                                                      "text", OWNER_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_Files), column);

    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_Files);

    GtkWidget *zone_tree;
    zone_tree = gtk_fixed_new ();
    gtk_fixed_put(GTK_FIXED(zone_tree), scrolled_window, 175, 50);
    
    return zone_tree;
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