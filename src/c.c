/* Compilez ce programme avec :
  * $ gcc -L/usr/X11R6/lib/ -I/usr/local/include/ -lgtk -lgdk -lglib -lX11 -lm -Wall main.c
  */
 #include        <gtk/gtk.h>
 #include        <stdio.h>
 
 /* Chaîne pour stocker les données dans les items de la liste. */
 
 const   gchar   *list_item_data_key="list_item_data";
 
 
 /* prototypes des gestionnaires de signaux que l'on connectera au widget GtkList. */
 
 static  void    sigh_print_selection    (GtkWidget      *gtklist,
                                          gpointer       func_data);
 static  void    sigh_button_event       (GtkWidget      *gtklist,
                                          GdkEventButton *event,
                                          GtkWidget      *frame);
 
 /* fonction principale pour configurer l'interface utilisateur */
 
 gint main (int argc, gchar *argv[])
 {                                  
     GtkWidget       *separator;
     GtkWidget       *window;
     GtkWidget       *vbox;
     GtkWidget       *scrolled_window;
     GtkWidget       *frame;
     GtkWidget       *gtklist;
     GtkWidget       *button;
     GtkWidget       *list_item;
     GList           *dlist;
     guint           i;
     gchar           buffer[64];
     
     
     /* initialise gtk (et donc gdk) */
 
     gtk_init(&argc, &argv);
     
     
     /* Création d'une fenêtre pour placer tous les widgets.
      * Connexion de  gtk_main_quit() à l'événement "destroy" de
      * la fenêtre afin de prendre en charge les événements « fermeture d'une
      * fenêtre » du gestionnaire de fenêtre. */ 
 
     window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title(GTK_WINDOW(window), "Exemple de widget GtkList");
     gtk_signal_connect(GTK_OBJECT(window),
                        "destroy",
                        GTK_SIGNAL_FUNC(gtk_main_quit),
                        NULL);
    
    
     /* À l'intérieur de la fenêtre, on a besoin d'une boîte pour placer 
      * verticalement les widgets. */
 
     vbox=gtk_vbox_new(FALSE, 5);
     gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
     gtk_container_add(GTK_CONTAINER(window), vbox);
     gtk_widget_show(vbox);
    
     /* Fenêtre à défilement pour placer le widget GtkList à l'intérieur. */
 
     scrolled_window=gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_usize(scrolled_window, 250, 150);
     gtk_container_add(GTK_CONTAINER(vbox), scrolled_window);
     gtk_widget_show(scrolled_window);
    
     /* Création du widget GtkList
      * Connexion du gestionnaire de signal sigh_print_selection() au signal
      * "selection_changed" du GtkList pour afficher les items sélectionnés
      * à chaque fois que la sélection change. */
 
     gtklist=gtk_list_new();
     gtk_container_add(GTK_CONTAINER(scrolled_window), gtklist);
     gtk_widget_show(gtklist);
     gtk_signal_connect(GTK_OBJECT(gtklist),
                        "selection_changed",
                        GTK_SIGNAL_FUNC(sigh_print_selection),
                        NULL);
     
     /* Création d'une « Prison » pour y mettre un item. */
 
     frame=gtk_frame_new("Prison");
     gtk_widget_set_usize(frame, 200, 50);
     gtk_container_border_width(GTK_CONTAINER(frame), 5);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
     gtk_container_add(GTK_CONTAINER(vbox), frame);
     gtk_widget_show(frame);
    
     /* Connexion du gestionnaire de signal sigh_button_event() au signal
      * « mise au arrêts » des items du GtkList. */
 
     gtk_signal_connect(GTK_OBJECT(gtklist),
                        "button_release_event",
                        GTK_SIGNAL_FUNC(sigh_button_event),
                        frame);
    
     /* Création d'un séparateur. */
 
     separator=gtk_hseparator_new();
     gtk_container_add(GTK_CONTAINER(vbox), separator);
     gtk_widget_show(separator);
    
     /* Création d'un bouton et connexion de son signal "clicked" à la
      * destruction de la fenêtre. */
 
     button=gtk_button_new_with_label("Fermeture");
     gtk_container_add(GTK_CONTAINER(vbox), button);
     gtk_widget_show(button);
     gtk_signal_connect_object(GTK_OBJECT(button),
                               "clicked",
                               GTK_SIGNAL_FUNC(gtk_widget_destroy),
                               GTK_OBJECT(window));
    
     /* Création de 5 items, chacun ayant son propre label.
      * Ajout de ceux-ci au GtkList en utilisant gtk_container_add().
      * On interroge le texte du label et on l'associe avec
      * list_item_data_key à chaque item. */
    
     for (i=0; i<5; i++) {
         GtkWidget       *label;
         gchar           *string;
        
         sprintf(buffer, "ListItemContainer avec Label #%d", i);
         label=gtk_label_new(buffer);
         list_item=gtk_list_item_new();
         gtk_container_add(GTK_CONTAINER(list_item), label);
         gtk_widget_show(label);
         gtk_container_add(GTK_CONTAINER(gtklist), list_item);
         gtk_widget_show(list_item);
         gtk_label_get(GTK_LABEL(label), &string);
         gtk_object_set_data(GTK_OBJECT(list_item),
                             list_item_data_key,
                             string);
     }
 
     /* Création de 5 autres labels. Cette fois-ci, on utilise
      * gtk_list_item_new_with_label(). On ne peut interroger la chaîne
      * des labels car on n'a pas les pointeurs de labels et on associe
      * donc simplement le list_item_data_key de chaque item ayant la même 
      * chaîne de texte pour l'ajouter au items que l'on place dans une liste
      * doublement chaînée (GList). On les ajoute alors par un simple appel à
      * gtk_list_append_items().
      * Comme on utilise g_list_prepend() pour mettre les items dans la liste
      * doublement chaînée, leur ordre sera décroissant (au lieu d'être croissant si
      * on utilisait g_list_append()). */
     
     dlist=NULL;
     for (; i<10; i++) {
         sprintf(buffer, "Item avec le label %d", i);
         list_item=gtk_list_item_new_with_label(buffer);
         dlist=g_list_prepend(dlist, list_item);
         gtk_widget_show(list_item);
         gtk_object_set_data(GTK_OBJECT(list_item),
                             list_item_data_key,
                             "Item avec label intégré");
     }
     gtk_list_append_items(GTK_LIST(gtklist), dlist);
    
     /* Enfin, on veut voir la fenêtre... */
    
     gtk_widget_show(window);
    
     /* Lancement de la boucle principale de gtk */
    
     gtk_main();
    
     /* On arrive ici après que gtk_main_quit() ait été appelée lorsque
      * la fenêtre principale a été détruite. */
 
 }
 
 /* Gestionnaire de signal connecté aux événements boutons presser/relâcher
  * du GtkList. */
 
 void
 sigh_button_event       (GtkWidget      *gtklist,
                          GdkEventButton *event,
                          GtkWidget      *frame)
 {
     /* On ne fait quelque chose que si le troisième bouton (celui de droite) a été
      * relâché. */
 
     if (event->type==GDK_BUTTON_RELEASE &&
         event->button==3) {
         GList           *dlist, *free_list;
         GtkWidget       *new_prisoner;
        
         /* On recherche l'item sélectionné à ce moment précis. 
          * Ce sera notre prisonnier ! */
 
         dlist=GTK_LIST(gtklist)->selection;
         if (dlist)
                 new_prisoner=GTK_WIDGET(dlist->data);
         else
                 new_prisoner=NULL;
         
         /* On recherche les items déjà prisonniers et on les
          * remet dans la liste.
          * Il ne faut pas oublier de libérer la liste doublement
          * chaînée que gtk_container_children() retourne. */
        
         dlist=gtk_container_children(GTK_CONTAINER(frame));
         free_list=dlist;
         while (dlist) {
             GtkWidget       *list_item;
            
             list_item=dlist->data;
            
             gtk_widget_reparent(list_item, gtklist);
            
             dlist=dlist->next;
         }
         g_list_free(free_list);
        
         /* Si l'on a un nouveau prisonnier, on l'ôte du GtkList et on le place
          * dans le cadre « Prison ». On doit désélectionner l'item avant.*/
 
         if (new_prisoner) {
             GList   static_dlist;
             
             static_dlist.data=new_prisoner;
             static_dlist.next=NULL;
             static_dlist.prev=NULL;
            
             gtk_list_unselect_child(GTK_LIST(gtklist),
                                     new_prisoner);
             gtk_widget_reparent(new_prisoner, frame);
         }
     }
 }
 
 /* Gestionnaire de signal appelé lorsque le GtkList
  * émet le signal "selection_changed". */
 
 void
 sigh_print_selection    (GtkWidget      *gtklist,
                          gpointer       func_data)
 {
     GList   *dlist;
    
     /* Recherche dans la liste doublement chaînée des items sélectionnés
      * du GtkList, à faire en lecture seulement ! */
 
     dlist=GTK_LIST(gtklist)->selection;
    
     /* S'il n'y a pas d'items sélectionné, il n'y a rien d'autre à faire que
      * de le dire à l'utilisateur. */
 
     if (!dlist) {
         g_print("Sélection nettoyée\n");
         return;
     }
     /* Ok, on a une sélection et on l'affiche. */
 
     g_print("La sélection est ");
    
     /* On récupère l'item dans la liste doublement chaînée 
      * puis on interroge la donnée associée par list_item_data_key
      * et on l'affiche. */
 
     while (dlist) {
         GtkObject       *list_item;
         gchar           *item_data_string;
        
         list_item=GTK_OBJECT(dlist->data);
         item_data_string=gtk_object_get_data(list_item,
                                              list_item_data_key);
         g_print("%s ", item_data_string);
        
         dlist=dlist->next;
     }
     g_print("\n");
 }