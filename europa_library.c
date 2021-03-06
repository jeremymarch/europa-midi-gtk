/*
europa-gtk - a midi librarian for the Roland Jupiter 6 synth with Europa mod.

Copyright (C) 2005-2021  Jeremy March

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. 
*/

#include <gtk/gtk.h>
#include <mysql.h>
#include <stdlib.h>
#include "europa.h"
#include "europa_patch.h"
#include "europa_mysql.h"
#include "europa_library.h"

extern MYSQL *mysql;

static gboolean
libSetPatch (GtkTreeSelection *sel, gpointer patchForm)
{
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  gint patch_id = 0;
  gint querylen = 128;
  gchar query[querylen];
  MYSQL_RES *res_set;
  MYSQL_ROW  row;
  unsigned long *len;

  if (gtk_tree_selection_get_selected (sel, &model, &iter)) {

    gtk_tree_model_get (model, &iter, 0, &patch_id, -1);

    snprintf (query, querylen, "SELECT UNHEX(patch) FROM patches WHERE patch_id = %i", patch_id);

    if (mysql_query (mysql, query) != 0)
    {
      return FALSE;
    }
    if ((res_set = mysql_store_result (mysql)) == NULL)
    {
      g_print ("store result failed");
      return FALSE;
    }

    if ((row = mysql_fetch_row (res_set)) == NULL)
    {
      mysql_free_result (res_set);
      return FALSE;
    }

    len = mysql_fetch_lengths (res_set);

    patch_received ((EuropaPatchForm *) patchForm, (guchar *) row[0], (unsigned int) len[0]);  

    mysql_free_result (res_set);

  }
  return FALSE;
}

void
fill_library(GtkListStore *listStore)
{
  MYSQL_RES *res_set;
  MYSQL_ROW  row;
  gchar query[] = "SELECT patch_id, name FROM patches ORDER BY name;";
  GtkTreeIter iter;

  if (mysql_query(mysql, query) != 0) {
    return;
  }

  if ((res_set = mysql_store_result (mysql)) == NULL) {
    return;
  }

  gtk_list_store_clear(GTK_LIST_STORE(listStore));

  while ((row = mysql_fetch_row(res_set)) != NULL)
  {
    gtk_list_store_append(GTK_LIST_STORE(listStore), &iter);
    gtk_list_store_set(GTK_LIST_STORE(listStore), &iter, 0, atoi(row[0]), 1, row[1], -1);
  }

  mysql_free_result(res_set);
}

gulong sigSelectionChanged = 0;
gulong sigClicked = 0;

gboolean
cb_fill(GtkWidget *button, gpointer list)
{
  GtkTreeModel *model;
  GtkTreeSelection *sel;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(list));

  g_signal_handler_block(sel, sigSelectionChanged);
  fill_library(GTK_LIST_STORE(model));
  g_signal_handler_unblock(sel, sigSelectionChanged);

  return FALSE;
}

void
draw_library_window(EuropaPatchForm *patchForm)
{
  GtkWidget *window, *list, *button, *vbox;
  GtkListStore *listStore = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeSelection *sel = NULL; 

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 700);
  gtk_window_set_title (GTK_WINDOW(window), "Europa Patch Library");

  listStore = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);

  list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(list),
                                               -1,
                                               "Patches",
                                               renderer,
                                               "text", 1,
                                               NULL);

  button = gtk_button_new_with_label("Get");

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), list, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(list));

  sigSelectionChanged = g_signal_connect (G_OBJECT(sel), "changed", G_CALLBACK(libSetPatch), (gpointer) patchForm);
  sigClicked = g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(cb_fill), (gpointer) list);

  fill_library(listStore);

  gtk_widget_show_all(window);
}
