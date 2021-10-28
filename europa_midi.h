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

gchar *
bintohex(const guchar *bin, guint len, gboolean space);

gchar *
bintodec(const guchar *bin, guint len, gboolean space);

int
validate_patch (guchar *p, gint len);

gchar *
getPatchError();

snd_seq_t *
open_seq (snd_seq_t *seq_handle, int *portidIn, int *portidOut);

void
sendSysex(snd_seq_t *seq_handle, int portidOut, guchar *sysex, guint len);
