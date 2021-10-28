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

#define _GNU_SOURCE //fixes realpath warning

#include <limits.h>
#include <gtk/gtk.h>
#include <alsa/asoundlib.h>
#include <mysql.h>
#include "europa.h"
#include "europa_patch.h"
#include "europa_midi.h"
#include "europa_mysql.h"
#include "europa_library.h"

extern MYSQL *mysql;
extern snd_seq_t *seq_handle;
extern int portidIn;
extern int portidOut;

gchar *db_user_name = NULL;
gchar *db_password = NULL;
gchar *db_host = NULL;
gchar *db_db = NULL;

static GOptionEntry entries[] =
{
  {"user", 'u', 0, G_OPTION_ARG_STRING, &db_user_name, "config file", "file"},
  {"host", 'h', 0, G_OPTION_ARG_STRING, &db_host, "config file", "file"},
  {"pass", 'p', 0, G_OPTION_ARG_STRING, &db_password, "config file", "file"},
  {"database", 'd', 0, G_OPTION_ARG_STRING, &db_db, "config file", "file"},
  { NULL }
};

int
main (int argc, char **argv)
{
  GIOChannel *chan;
  int npfd;
  struct pollfd *pfd;
  EuropaPatchForm *patchForm;

  char pathbuf[PATH_MAX];
  char pathbuf2[PATH_MAX];
  char *res = realpath(argv[0], pathbuf);
  g_snprintf(pathbuf2, PATH_MAX, "%.*seuropa_rc", (int)strlen(res) - 6, res); //program path - length of program name + rc name
  //g_print("arg: %s\n", pathbuf2);
  gtk_rc_parse (pathbuf2);
  //gtk_rc_parse ("/home/jeremy/Documents/code/europa-midi-gtk/europa_rc");
  
  GError * error = NULL;
  GOptionContext * context = g_option_context_new ("- convert fastq");
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error)){
      g_print("error: %s\n", error->message);
      exit(1);
  }

  gtk_init(&argc, &argv);
  
  if ((mysql = do_connect (db_host, db_user_name, db_password, db_db, 0, NULL, 0)) == NULL)
  {
    g_print("Could not connect to database\n");
    exit(1);
  }

  init_db(mysql);

  patchForm = draw_panel();
  draw_library_window (patchForm);

  seq_handle = open_seq (seq_handle, &portidIn, &portidOut);

  npfd = snd_seq_poll_descriptors_count (seq_handle, POLLIN);

  pfd = (struct pollfd *) alloca (npfd * sizeof(struct pollfd));

  snd_seq_poll_descriptors (seq_handle, pfd, npfd, POLLIN);

  chan = g_io_channel_unix_new (pfd->fd);
  g_io_channel_set_encoding (chan, NULL, NULL);
  g_io_add_watch (chan, G_IO_IN, poll_func, patchForm); /* when we start catching presets we'll need to pass that too */

  gtk_main ();

  do_disconnect (mysql);

  snd_seq_close (seq_handle);

  exit(0);
}
