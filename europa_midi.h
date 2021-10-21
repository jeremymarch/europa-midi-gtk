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
