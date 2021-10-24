#include <gtk/gtk.h>
#include <alsa/asoundlib.h>
#include <mysql.h>
#include "europa.h"
#include "europa_patch.h"
#include "europa_midi.h"
#include "europa_mysql.h"
#include "europa_library.h"

extern MYSQL *mysql;

snd_seq_t *seq_handle;

int portidIn;
int portidOut;

char *vSliderLabels[NUM_SLIDERS] = 
{ "RATE", 
  "DELAY", 
  "LFO", 
  "ENV1", 
  "PW", 
  "PWM", 
  "MAN", 
  "ENV1",
  "RANGE",
  "RANGE",
  "TUNE",
  "VCO MIXER",
  "FREQ", 
  "RES", 
  "ENV", 
  "LFO", 
  "KYBD", 
  "ENV2-LEV", 
  "LFO", 
  "A", 
  "D", 
  "S", 
  "R", 
  "KF", 
  "A", 
  "D", 
  "S", 
  "R", 
  "KF" 
};


static void
printStatusMesg(GtkStatusbar *statusBar, gchar *mesg)
{
  guint context_id;

  context_id = gtk_statusbar_get_context_id(statusBar, "status ex" );
  gtk_statusbar_push(statusBar, context_id, mesg );
}

gint
get_patch(EuropaPatchForm *patchForm, guchar **p)
{
  guchar *pp;
  gint i;
  const gchar *name;
  guchar namelen;
/*  gchar *z; */

  name = gtk_entry_get_text (GTK_ENTRY(patchForm->name));
  namelen = strlen(name);

  if (namelen > 16)
    namelen = 16; /* truncate name to 16 chars if necessary */

  *p = g_malloc (sizeof(guchar) * (PATCH_MIN_LEN + namelen));
  memset(*p, 0x00, PATCH_MIN_LEN + namelen);

  pp = *p;

  *pp++ = 0xF0;
  *pp++ = 0x41;
  *pp++ = 0x01;
  *pp++ = 0x4A;
  *pp++ = 0x36;
  *pp++ = 0x05;
  *pp++ = 0x00;
  *pp++ = 0x7F;

  *pp++ = namelen; /* name len field */

  for (i = 0; i < namelen; i++)
  {
    *pp++ = name[i]; /* patch name if any */
  }

  for (i = 0; i < NUM_SLIDERS; i++)
  {
    *pp++ = (guchar) gtk_adjustment_get_value (GTK_ADJUSTMENT(patchForm->sliders[i].adjustment));
  }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->lfo_wave_tri)))
    *pp++ = 0x08;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->lfo_wave_saw)))
    *pp++ = 0x04;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->lfo_wave_square)))
    *pp++ = 0x02;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->lfo_wave_random)))
    *pp++ = 0x01;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->pwm_env1)))
    *pp |= 0x08;
  else
    *pp |= 0x04;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1)))
    *pp |= 0x02;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2)))
    *pp |= 0x01;
  pp++;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_tri)))
    *pp |= 0x08;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_saw)))
    *pp |= 0x04;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_pulse)))
    *pp |= 0x02;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_square)))
    *pp |= 0x01;
  pp++;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_tri)))
    *pp |= 0x08;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_saw)))
    *pp |= 0x04;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_pulse)))
    *pp |= 0x02;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_noise)))
    *pp |= 0x01;
  pp++;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf)))
    *pp |= 0x08;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf)))
    *pp |= 0x04;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r)))
    *pp |= 0x02;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l)))
    *pp |= 0x01;
  pp++;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->env1_pos)))
    *pp |= 0x08;
  else
    *pp |= 0x04;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(patchForm->vcf_env1)))
    *pp |= 0x02;
  else
    *pp |= 0x01;
  *++pp = 0xF7;

  if (!validate_patch (*p, PATCH_MIN_LEN + namelen))
  {
    g_print("Patch invalid: %s\n", getPatchError());

    printStatusMesg (GTK_STATUSBAR(patchForm->statusBar), "Patch Invalid");
    return -1;
  }
/*
  z = bintohex(*p, PATCH_MIN_LEN + namelen, TRUE);
  g_print("get_patch hex: %s\n", z);
  g_free(z);
*/
  return PATCH_MIN_LEN + namelen;
}

void
set_slider (EuropaPatchForm *patchForm, int slider, guchar value)
{
  gtk_adjustment_set_value (GTK_ADJUSTMENT(patchForm->sliders[slider].adjustment), (guint) value);
}

void
set_lfo_waveform(EuropaPatchForm *patchForm, guchar value)
{
  switch (value)
  {
    case 0x08:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_tri), TRUE);
      break;
    case 0x04:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_saw), TRUE); 
      break;
    case 0x02:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_square), TRUE);
      break;
    case 0x01:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_random), TRUE);
      break;
  }
}

void
cc_set_lfo_waveform(EuropaPatchForm *patchForm, guchar value)
{
  switch (value)
  {
    case 0x03:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_tri), TRUE);
      break;
    case 0x02:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_saw), TRUE); 
      break;
    case 0x01:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_square), TRUE);
      break;
    case 0x00:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patchForm->lfo_wave_random), TRUE);
      break;
  }
}

void
set_pwm_vco_mod (EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->pwm_env1), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->pwm_lfo), TRUE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), FALSE);
}

void
cc_set_vco_mod (EuropaPatchForm *patchForm, guchar value)
{
  switch (value)
  {
    case 0x00:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), FALSE);
      break;
    case 0x01:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), TRUE);
      break;
    case 0x02:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), FALSE);
      break;
    case 0x03:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco1), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_mod_vco2), TRUE);
      break;
  }
}

void
cc_set_pwm_mod (EuropaPatchForm *patchForm, guchar value)
{
  if (value == 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->pwm_env1), TRUE);
  else if (value == 0x00)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->pwm_lfo), TRUE);
}

void
set_vco1_waveform(EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_tri), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_tri), FALSE);

  if (value & 0x04)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_saw), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_saw), FALSE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_pulse), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_pulse), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_square), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_square), FALSE);
}

void
cc_set_vco1_waveform (EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_tri), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_tri), FALSE);

  if (value & 0x04)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_saw), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_saw), FALSE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_pulse), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_pulse), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_square), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco1_wave_square), FALSE);
}

void
set_vco2_waveform(EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_tri), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_tri), FALSE);

  if (value & 0x04)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_saw), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_saw), FALSE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_pulse), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_pulse), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_noise), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_noise), FALSE);
}

void
cc_set_vco2_waveform(EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_tri), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_tri), FALSE);

  if (value & 0x04)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_saw), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_saw), FALSE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_pulse), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_pulse), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_noise), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco2_wave_noise), FALSE);
}

void
set_vcf_filter_vco_sync (EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08 && value & 0x04)
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), TRUE);
  }
  else if (value & 0x04)
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), FALSE);
  }
  else if (value & 0x08)
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), TRUE);
  }

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), FALSE);

  if (value & 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), FALSE);
}

void
cc_set_vcf_filter_mode (EuropaPatchForm *patchForm, guchar value)
{
  switch (value)
  {
    case 0x00:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), FALSE);
      break;
    case 0x01:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), TRUE);
      break;
    case 0x02:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_hpf), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_lpf), TRUE);
      break;
  }
}

void
cc_set_sync_mode (EuropaPatchForm *patchForm, guchar value)
{
  switch (value)
  {
    case 0x00:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), FALSE);
      break;
    case 0x01:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), FALSE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), TRUE);
      break;
    case 0x02:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), FALSE);
      break;
    case 0x03:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_r), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vco_sync_l), TRUE);
      break;
  }
}

void
set_vcf_env_env_pol (EuropaPatchForm *patchForm, guchar value)
{
  if (value & 0x08)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->env1_pos), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->env1_neg), TRUE);

  if (value & 0x02)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_env1), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_env2), TRUE);
}

void
cc_set_vcf_env_mod (EuropaPatchForm *patchForm, guchar value)
{
  if (value == 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_env1), TRUE);
  else if (value == 0x00)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->vcf_env2), TRUE);
}

void
cc_set_env1_pol (EuropaPatchForm *patchForm, guchar value)
{
  if (value == 0x01)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->env1_pos), TRUE);
  else if (value == 0x00)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(patchForm->env1_neg), TRUE);
}

void
set_patch_name(EuropaPatchForm *patchForm, guchar *p, int len)
{
  gchar name[17];
  gint i, j, name_len;

  name_len = p[PATCH_NAME_LEN_BYTE];

  for (i = PATCH_HEADER_LENGTH, j = 0; i < name_len + PATCH_HEADER_LENGTH; i++, j++)
  {
    name[j] = p[i];
  }
  name[j] = '\0';

  gtk_entry_set_text(GTK_ENTRY(patchForm->name), name);
}

gboolean
sendRandom (GtkWidget *button, gpointer data)
{
  guchar midi[8];

  midi[0] = 0xF0;
  midi[1] = 0x41;
  midi[2] = 0x01;
  midi[3] = 0x4A;
  midi[4] = 0x36;
  midi[5] = 0x03;
  midi[6] = 0x7F;
  midi[7] = 0xF7;

  sendSysex(seq_handle, portidOut, (guchar *) &midi, 8);

  return FALSE;
}

gboolean
requestPatch (GtkWidget *button, gpointer data)
{
  guchar midi[9];

  midi[0] = 0xF0;
  midi[1] = 0x41;
  midi[2] = 0x01;
  midi[3] = 0x4A;
  midi[4] = 0x36;
  midi[5] = 0x04;
  midi[6] = 0x00;
  midi[7] = 0x7F;
  midi[8] = 0xF7;

  sendSysex(seq_handle, portidOut, (guchar *) &midi, 9);

  return FALSE;
}

gboolean
sendPatch (GtkWidget *button, gpointer patchForm)
{
  guchar *midi;
  gchar  *text;
  guint   len;

  if ((len = get_patch((EuropaPatchForm *) patchForm, &midi)) < 0)
  {
    g_free(midi);
    g_print("sendPatch failed: invalid patch\n");
    return FALSE;
  }

  text = bintohex (midi, len, TRUE);
  g_print ("send patch: %s\n", text);
  g_free (text);

  sendSysex (seq_handle, portidOut, midi, len);

  g_free (midi);

  return FALSE;
}

gboolean
savePatch(GtkWidget *button, gpointer patchForm)
{
  gint len;
  guchar *p;

  if ((len = get_patch((EuropaPatchForm *) patchForm, &p)) < PATCH_MIN_LEN)
  {
    g_print("blah\n");
    return FALSE;
  }

  real_save_patch(p, len);

/*  fill_library(listStore); */

  return FALSE;
}

/* at least one must be pressed */
gboolean
clickedPassFilter (GtkWidget *button, gpointer otherButton)
{
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(otherButton)))
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), TRUE);
  }
  return FALSE;
}

/* only one or none may be pressed */
gboolean
clickedVcoOne (GtkWidget *button, gpointer otherButton)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(otherButton)))
  {
    g_signal_handlers_block_by_func (otherButton, (gpointer) clickedVcoOne, button);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(otherButton), FALSE);

    g_signal_handlers_unblock_by_func (otherButton, (gpointer) clickedVcoOne, button);
  }
  return FALSE;
}

GtkWidget *
draw_lfo1_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 3);
  
  patch->lfo_wave_tri = gtk_radio_button_new_with_label_from_widget(NULL, "tri");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->lfo_wave_tri), FALSE);

  patch->lfo_wave_saw = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->lfo_wave_tri), "saw");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->lfo_wave_saw), FALSE);

  patch->lfo_wave_square = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->lfo_wave_saw), "square");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->lfo_wave_square), FALSE);

  patch->lfo_wave_random = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->lfo_wave_square), "rand");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->lfo_wave_random), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox), patch->lfo_wave_tri, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->lfo_wave_saw, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->lfo_wave_square, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->lfo_wave_random, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vco_mod_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vco_mod_vco1 = gtk_toggle_button_new_with_label("VC01");

  patch->vco_mod_vco2 = gtk_toggle_button_new_with_label("VC02");

  gtk_box_pack_start(GTK_BOX(vbox), patch->vco_mod_vco1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco_mod_vco2, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_pwm_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 3);
  
  patch->pwm_env1 = gtk_radio_button_new_with_label_from_widget(NULL, "ENV1");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->pwm_env1), FALSE);

  patch->pwm_lfo = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->pwm_env1), "LFO");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->pwm_lfo), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox), patch->pwm_env1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->pwm_lfo, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vco1_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vco1_wave_tri = gtk_toggle_button_new_with_label("TRI");

  patch->vco1_wave_saw = gtk_toggle_button_new_with_label("SAW");

  patch->vco1_wave_pulse = gtk_toggle_button_new_with_label("PULSE");

  patch->vco1_wave_square = gtk_toggle_button_new_with_label("SQUARE");

  gtk_box_pack_start(GTK_BOX(vbox), patch->vco1_wave_tri, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco1_wave_saw, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco1_wave_pulse, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco1_wave_square, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vco_sync_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vco_sync_r = gtk_toggle_button_new_with_label("->");

  patch->vco_sync_l = gtk_toggle_button_new_with_label("<-");

  gtk_box_pack_start(GTK_BOX(vbox), patch->vco_sync_r, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco_sync_l, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vco2_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vco2_wave_tri = gtk_toggle_button_new_with_label("TRI");

  patch->vco2_wave_saw = gtk_toggle_button_new_with_label("SAW");

  patch->vco2_wave_pulse = gtk_toggle_button_new_with_label("PULSE");

  patch->vco2_wave_noise = gtk_toggle_button_new_with_label("NOISE");

  gtk_box_pack_start(GTK_BOX(vbox), patch->vco2_wave_tri, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco2_wave_saw, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco2_wave_pulse, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vco2_wave_noise, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vcf_buttons1 (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vcf_hpf = gtk_toggle_button_new_with_label("HPF");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(patch->vcf_hpf), TRUE);

  patch->vcf_lpf = gtk_toggle_button_new_with_label("LPF");

  gtk_box_pack_start(GTK_BOX(vbox), patch->vcf_hpf, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vcf_lpf, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_vcf_buttons2 (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->vcf_env1 = gtk_radio_button_new_with_label_from_widget(NULL, "ENV1");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->vcf_env1), FALSE);

  patch->vcf_env2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->vcf_env1), "ENV2");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->vcf_env2), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox), patch->vcf_env1, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->vcf_env2, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_env1_buttons (EuropaPatchForm *patch)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new(TRUE, 5);

  patch->env1_pos = gtk_radio_button_new_with_label_from_widget(NULL, "POS");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->env1_pos), FALSE);

  patch->env1_neg = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(patch->env1_pos), "NEG");
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON(patch->env1_neg), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox), patch->env1_pos, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->env1_neg, FALSE, FALSE, 0);

  return vbox;
}

GtkWidget *
draw_slider (EuropaPatchForm *patch, int adj)
{
  patch->sliders[adj].adjustment = gtk_adjustment_new(0, 0, 127, 1, 10, 0);
  patch->sliders[adj].slider = gtk_vscale_new(GTK_ADJUSTMENT(patch->sliders[adj].adjustment));
  gtk_range_set_inverted(GTK_RANGE(patch->sliders[adj].slider), TRUE);
  gtk_scale_set_value_pos (GTK_SCALE(patch->sliders[adj].slider), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE(patch->sliders[adj].slider), 0);

  gtk_widget_set_size_request(patch->sliders[adj].slider, 1, 120);

  patch->sliders[adj].label = gtk_label_new(vSliderLabels[adj]);

  patch->sliders[adj].vBox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(patch->sliders[adj].vBox), patch->sliders[adj].label, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(patch->sliders[adj].vBox), patch->sliders[adj].slider, TRUE, TRUE, 0);

  return patch->sliders[adj].vBox;
}

GtkWidget *
draw_lfo1_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, LFO1_RATE);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, LFO1_DELAY);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_lfo1_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vco_mod_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, VCOMOD_LFO);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCOMOD_ENV1);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_vco_mod_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_pwm_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, PWM_PW);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, PWM_PWM);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_pwm_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vco1_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, VCO1_MANUAL);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCO1_ENV1);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCO1_RANGE);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_vco1_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vco_sync_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = GTK_WIDGET(draw_vco_sync_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vco2_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, VCO2_RANGE);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCO2_FINE_TUNE);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_vco2_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_mixer_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, VCO_MIXER);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vcf_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = GTK_WIDGET(draw_vcf_buttons1(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCF_FREQ);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCF_RES);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = GTK_WIDGET(draw_vcf_buttons2(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCF_ENV);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCF_LFO);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, VCF_KYBD);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_vca_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, VCA_ENV2);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  vbox = draw_slider(patch, VCA_LFO);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_env1_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = GTK_WIDGET(draw_env1_buttons(patch));
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV1_A);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV1_D);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV1_S);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV1_R);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV1_KF);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

GtkWidget *
draw_env2_panel (EuropaPatchForm *patch)
{
  GtkWidget *hbox, *vbox;

  hbox = gtk_hbox_new(FALSE, 5);

  vbox = draw_slider(patch, ENV2_A);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV2_D);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV2_S);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV2_R);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  vbox = draw_slider(patch, ENV2_KF);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  return hbox;
}

EuropaPatchForm *
draw_panel()
{
  GtkWidget *vbox;
  GtkWidget *newbox, *table, *table2, *frame;
  GtkWidget *label, *button, *patchbutton, *buttonbox, *reqPatchbutton, *saveButton;
  EuropaPatchForm * patch;

  patch = (EuropaPatchForm *) g_malloc(sizeof(EuropaPatchForm) * 1);

  patch->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(patch->window), 900, 500);
  gtk_window_set_title (GTK_WINDOW(patch->window), "Europa Patch Control");
  g_signal_connect (patch->window, "delete_event", gtk_main_quit, NULL);

  table = gtk_table_new(4, 6, FALSE);

  label = gtk_label_new("LFO-1");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 0, 1, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_lfo1_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("VCO MOD");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 1, 2, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vco_mod_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("PWM");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 2, 3, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_pwm_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 2, 3, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("VCO1");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 3, 4, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vco1_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 3, 4, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("SYNC");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 4, 5, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vco_sync_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 4, 5, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("VCO2");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 5, 6, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vco2_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 5, 6, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("MIXER");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table), frame, 6, 7, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_mixer_panel(patch);
  gtk_table_attach (GTK_TABLE(table), newbox, 6, 7, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  table2 = gtk_table_new(4, 6, FALSE);

  label = gtk_label_new("VCF");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table2), frame, 0, 1, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vcf_panel(patch);
  gtk_table_attach (GTK_TABLE(table2), newbox, 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("VCA");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table2), frame, 1, 2, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_vca_panel(patch);
  gtk_table_attach (GTK_TABLE(table2), newbox, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("ENV1");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table2), frame, 2, 3, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_env1_panel(patch);
  gtk_table_attach (GTK_TABLE(table2), newbox, 2, 3, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  label = gtk_label_new("ENV2");
  frame = gtk_frame_new("");
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_table_attach (GTK_TABLE(table2), frame, 3, 4, 0, 1, GTK_SHRINK|GTK_FILL, GTK_SHRINK|GTK_FILL, 3, 3);

  newbox = draw_env2_panel(patch);
  gtk_table_attach (GTK_TABLE(table2), newbox, 3, 4, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 3, 3);

  patch->statusBar = gtk_statusbar_new();
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(patch->statusBar), TRUE);

  patch->name = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(patch->name), PATCH_NAME_MAX_LEN);
  gtk_entry_set_width_chars (GTK_ENTRY(patch->name), PATCH_NAME_MAX_LEN);

  button = gtk_button_new_with_label("Random");
  patchbutton = gtk_button_new_with_label("Send Patch");
  reqPatchbutton = gtk_button_new_with_label("Request Patch");
  saveButton = gtk_button_new_with_label("Save Patch");
  buttonbox = gtk_hbox_new(FALSE, 5);

  gtk_box_pack_start(GTK_BOX(buttonbox), patch->name, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(buttonbox), button, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(buttonbox), patchbutton, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(buttonbox), reqPatchbutton, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(buttonbox), saveButton, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(sendRandom), NULL);
  g_signal_connect (G_OBJECT(patchbutton), "clicked", G_CALLBACK(sendPatch), (gpointer) patch);
  g_signal_connect (G_OBJECT(reqPatchbutton), "clicked", G_CALLBACK(requestPatch), NULL);
  g_signal_connect (G_OBJECT(saveButton), "clicked", G_CALLBACK(savePatch), (gpointer) patch);

  g_signal_connect (G_OBJECT(patch->vcf_hpf), "toggled", G_CALLBACK(clickedPassFilter), patch->vcf_lpf);
  g_signal_connect (G_OBJECT(patch->vcf_lpf), "toggled", G_CALLBACK(clickedPassFilter), patch->vcf_hpf);

  g_signal_connect_after (G_OBJECT(patch->vco1_wave_square), "toggled", G_CALLBACK(clickedVcoOne), patch->vco1_wave_pulse);
  g_signal_connect_after (G_OBJECT(patch->vco1_wave_pulse), "toggled", G_CALLBACK(clickedVcoOne), patch->vco1_wave_square);

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), table2, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), buttonbox, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), patch->statusBar, FALSE, TRUE, 0);  

  gtk_container_add(GTK_CONTAINER(patch->window), vbox);

  gtk_widget_show_all(GTK_WIDGET(patch->window));

  return patch;
}

void
patch_received (EuropaPatchForm *patchForm, guchar *patch, int len)
{
  int i, name_len, start;
  gchar *patchHex;

  if (!validate_patch(patch, len))
  {
    g_print("Invalid Patch: %s\n", getPatchError());
/*    printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), patchHex); */
    return;
  }

  name_len = patch[PATCH_NAME_LEN_BYTE];

  start = name_len + PATCH_HEADER_LENGTH;

  for (i = start; i < start + NUM_SLIDERS; i++)
  {
    set_slider (patchForm, i - start, patch[i]);
  }

  set_lfo_waveform (patchForm, patch[start + PO_LFO1_WAVEFORM]);

  set_pwm_vco_mod (patchForm, patch[start + PO_PWM_VCO_MOD]);
 
  set_vco1_waveform (patchForm, patch[start + PO_VCO1_WAVEFORM]);

  set_vco2_waveform (patchForm, patch[start + PO_VCO2_WAVEFORM]);

  set_vcf_filter_vco_sync (patchForm, patch[start + PO_VCF_SYNC]);

  set_vcf_env_env_pol (patchForm, patch[start + PO_FILTER_MODE]);

  set_patch_name (patchForm, patch, len);


  patchHex = bintohex (patch, len, FALSE);

  g_print("patch: %s old\n", patchHex);

  printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), patchHex);
  g_free(patchHex);
}

void
preset_received (guchar *preset, int len)
{
  gchar *dis;

  dis = bintohex (preset, len, TRUE);
/*  g_snprintf(str, 128, "Sysex len: %u, patch: %s", len, dis);
   gtk_entry_set_text (entry, str); */
  g_print("preset: %s\n", dis);
  g_free(dis);
}

void
sequence_received (guchar *sequence, int len)
{
  g_print("received sequence\n");
}

void
cc_received (EuropaPatchForm *patchForm, guchar *cc, int len)
{
  gchar *dis;
  gchar str[50];
  dis = bintohex (cc, len, TRUE);

  snprintf(str, 50, "CC: %s", dis);
  printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), str);

  g_free(dis);

  if (cc[SYSEX_COMMAND_BYTE] >= 0x20 && cc[SYSEX_COMMAND_BYTE] <= 0x3C) /* patch slider */
  {
    int slider;
    slider = cc[SYSEX_COMMAND_BYTE] - 0x20;

    set_slider(patchForm, slider, cc[CC_VALUE_BYTE]);
    return;
  }

  switch (cc[SYSEX_COMMAND_BYTE])
  {
    case CC_LFO1_WAVEFORM:
      cc_set_lfo_waveform (patchForm, cc[CC_VALUE_BYTE]);
      break;
    case CC_VCO_MOD:
      cc_set_vco_mod (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_PWM_MOD:
      cc_set_pwm_mod (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_VCO1_WAVEFORM:
      cc_set_vco1_waveform (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_VCO2_WAVEFORM:
      cc_set_vco2_waveform (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_VCF_FILTER_MODE:
      cc_set_vcf_filter_mode (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_VCO_SYNC_MODE:
      cc_set_sync_mode (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_VCF_ENV_MOD:
      cc_set_vcf_env_mod (patchForm, cc[CC_VALUE_BYTE]);
      break;

    case CC_ENV1_POLARITY:
      cc_set_env1_pol (patchForm, cc[CC_VALUE_BYTE]);
      break;
  }
}

void
sysex_received (snd_seq_event_t *ev, EuropaPatchForm *patchForm)
{
  guchar *m;
  gchar *x; 

  m = (guchar *) g_malloc (sizeof(guchar) * ev->data.ext.len);
  memcpy (m, ev->data.ext.ptr, ev->data.ext.len);

  if (m[0] != 0xF0 || 
      m[1] != 0x41 || /* m[2] is the midi channel */
      m[3] != 0x4A || 
      m[4] != 0x36)
    return; /* ignore if not europa sysex */

  switch (m[SYSEX_COMMAND_BYTE])
  {
    case CMD_LOAD_EUROPA_DEFAULTS: 
      break;
    case CMD_LOAD_FACTORY_PATCHES: 
      break;
    case CMD_SET_MIDI_CHANNEL: 
      break;
    case CMD_GEN_RANDOM_PATCH:

      x = bintohex(m, ev->data.ext.len, TRUE);
      g_print("rand: %s\n", x);
      g_free(x);
      break;
    case CMD_REQUEST_DUMP:
      break;

    case CMD_WRITE_TO_FLASH_NVRAM: 

      switch (m[6])
      {
        case PATCH_DUMP:
          patch_received (patchForm, m, ev->data.ext.len);
          break;

        case PRESET_DUMP:
          preset_received (m, ev->data.ext.len);
          break;
        case SEQUENCE_DUMP:
          sequence_received (m, ev->data.ext.len);
          break;
      }
      break;

    case CMD_WRITE_EDIT_TO_FLASH:

      break;
    case CMD_ASSIGN_CONTROLLER:

      break;

    default:

      if (m[5] > 0x07 && m[5] < 0x20)
      {
        g_print("invalid command\n");
        return;
      }

      if (m[5] > 0x1F && m[5] < 0x80) /* cc mesg */
      {
        cc_received(patchForm, m, ev->data.ext.len);
      }
      else
      {
        printf("invalid command\n");
      }

      break;
  }
  g_free(m);
}

void
midi_received (EuropaPatchForm *patchForm)
{
  snd_seq_event_t *ev;
  gchar str[256];

  do 
  {
    snd_seq_event_input (seq_handle, &ev);

    switch (ev->type)
    {
      case SND_SEQ_EVENT_CONTROLLER: 
        g_snprintf (str, 128, "Control event on Channel %2d: %5d",
                ev->data.control.channel, ev->data.control.value);

        printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), str);
        break;

      case SND_SEQ_EVENT_PITCHBEND:
        g_snprintf (str, 128, "Pitchbender event on Channel %2d: %5d", 
                ev->data.control.channel, ev->data.control.value);

        printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), str);
        break;

      case SND_SEQ_EVENT_NOTEON:
        g_snprintf (str, 128, "Note On event on Channel %2d: %5d",
                ev->data.control.channel, ev->data.note.note);

        printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), str);
        break;        

      case SND_SEQ_EVENT_NOTEOFF:
        g_snprintf (str, 128, "Note Off event on Channel %2d: %5d",         
                ev->data.control.channel, ev->data.note.note);

        printStatusMesg(GTK_STATUSBAR(patchForm->statusBar), str);
        break;

      case SND_SEQ_EVENT_SYSEX:
        sysex_received (ev, patchForm);
        break;

      default:
        break; 
    }

    snd_seq_free_event (ev);

  } while (snd_seq_event_input_pending (seq_handle, 0) > 0);
}

gboolean
poll_func (GIOChannel *chan, GIOCondition con, gpointer patchForm)
{
  midi_received ((EuropaPatchForm *) patchForm);
 
  return TRUE;
}

int
main (int argc, char **argv)
{
  GIOChannel *chan;
  int npfd;
  struct pollfd *pfd;
  EuropaPatchForm *patchForm;

  gtk_rc_parse ("/home/jeremy/europa_rc");

  gtk_init(&argc, &argv);

  if ((mysql = do_connect ("localhost", "root", "clam1234", "europa", 0, NULL, 0)) == NULL)
  {
    g_print("Could not connect to database\n");
    exit(1);
  }

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
