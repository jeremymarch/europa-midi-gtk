#include <gtk/gtk.h>
#include <alsa/asoundlib.h>
#include "europa.h"
#include "europa_midi.h"
#include <string.h>

#define PATCH_ERROR_MAX_LEN 128

static gchar patchError[PATCH_ERROR_MAX_LEN];

gchar *
getPatchError()
{
  return patchError;
}

snd_seq_t *
open_seq (snd_seq_t *seq_handle, int *portidIn, int *portidOut)
{
  snd_seq_addr_t sender, dest;
  snd_seq_port_subscribe_t *subs;

  snd_seq_addr_t sender2, dest2;
  snd_seq_port_subscribe_t *subs2;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
  {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "Europa");
  if ((*portidIn = snd_seq_create_simple_port(seq_handle, "Europa In",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
  {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }

  if ((*portidOut = snd_seq_create_simple_port(seq_handle, "Europa Out",
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
  {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }

/* in */
  sender.client = 64;
  sender.port = 0;
  dest.client = 128;
  dest.port = 0;
  snd_seq_port_subscribe_alloca(&subs);
  snd_seq_port_subscribe_set_sender(subs, &sender);
  snd_seq_port_subscribe_set_dest(subs, &dest);
  snd_seq_port_subscribe_set_queue(subs, 1);
  snd_seq_port_subscribe_set_time_update(subs, 1);
  snd_seq_port_subscribe_set_time_real(subs, 1);
  snd_seq_subscribe_port(seq_handle, subs);

/* out */
  sender2.client = 128;
  sender2.port = 1;
  dest2.client = 64;
  dest2.port = 0;
  snd_seq_port_subscribe_alloca(&subs2);
  snd_seq_port_subscribe_set_sender(subs2, &sender2);
  snd_seq_port_subscribe_set_dest(subs2, &dest2);
  snd_seq_subscribe_port(seq_handle, subs2);

  return (seq_handle);
}

void
sendSysex (snd_seq_t *seq_handle, int portidOut, guchar *sysex, guint len)
{
  snd_seq_event_t ev;

  snd_seq_ev_clear (&ev);
  snd_seq_ev_set_sysex (&ev, len, sysex);
  snd_seq_ev_set_source (&ev, portidOut);
  snd_seq_ev_set_subs (&ev);
  snd_seq_ev_set_direct (&ev);

  snd_seq_event_output (seq_handle, &ev);
  snd_seq_drain_output (seq_handle);
}

gchar *
bintohex (const guchar *bin, guint len, gboolean space)
{
  guint i;
  gchar *dst;
  gchar *pdst;

  if (space)
    dst = (gchar *) g_malloc(sizeof(gchar) * len * 3);
  else
    dst = (gchar *) g_malloc(sizeof(gchar) * len * 2 + 1);

  pdst = dst;

  for (i = 0; i < len; i++)
  {
    if (bin[i] > 15)
      sprintf(pdst, "%2X", bin[i]);
    else
      sprintf(pdst, "0%X", bin[i]);

    pdst += 2;

    if (space && i + 1 != len)
      *pdst++ = ' ';
  }
  *pdst = '\0';

  return dst; /* must be freed by caller */
}

gchar *
bintodec (const guchar *bin, guint len, gboolean space)
{
  guint i;
  gchar *dst;
  gchar *pdst;

  dst = (gchar *) g_malloc(sizeof(gchar) * len * 4);

  pdst = dst;

  for (i = 0; i < len; i++)
  {
    sprintf(pdst, "%3u", bin[i]);

    pdst += 3;

    if (i + 1 != len)
      *pdst++ = ' ';
  }
  *pdst = '\0';

  return dst; /* must be freed by caller */
}

int
validate_patch (guchar *p, gint len)
{
  int i;
  int first_slider = PATCH_HEADER_LENGTH + p[PATCH_NAME_LEN_BYTE];

  if (len > PATCH_MAX_LEN )
  {
    strcpy(patchError, "Patch Too Long");
    return 0;
  }

  if (len < PATCH_MIN_LEN)
  {
    strcpy(patchError, "Patch Too Short");
    return 0;
  }

  if (len != PATCH_HEADER_LENGTH + p[PATCH_NAME_LEN_BYTE] + PATCH_BODY_LENGTH + 1) /* +1 for F7 */
  {
    strcpy(patchError, "Patch Name Length is Incorrect");
    return 0;
  }

  if (p[0] != 0xF0 || p[1] != 0x41 || 
      p[3] != 0x4A || p[4] != 0x36 || 
      p[5] != 0x05)
  {
    strcpy(patchError, "Patch Header is Invalid");
    return 0;
  }

  if (p[2] > 16)
  {
    strcpy(patchError, "Patch Midi Channel is Invalid");
    return 0;
  }

  if (p[6] != 0x00) /* PATCH TYPE = patch */
  {
    strcpy(patchError, "Dump Type is not a Patch");
    return 0;
  }

  if (p[7] > 47 && p[7] != 126 && p[7] != 127) /* PATCH NUMBER */
  {
    strcpy(patchError, "Patch Number is Invalid");
    return 0;
  }

  if (p[8] > 16) /* PATCH NAME LENGTH */
  {
    strcpy(patchError, "Patch Name is too Long");
    return 0;
  }

  for (i = first_slider; i < NUM_SLIDERS; i++)
  {
    if (p[i] > 127)
    {
      snprintf(patchError, PATCH_ERROR_MAX_LEN, "Slider %i value %u is Invalid\n", i, p[i]);
      return 0;
    }
  }

/* LFO waveform is one of four choices */
  if (p[first_slider + PO_LFO1_WAVEFORM] != 0x08 &&
      p[first_slider + PO_LFO1_WAVEFORM] != 0x04 &&
      p[first_slider + PO_LFO1_WAVEFORM] != 0x02 &&
      p[first_slider + PO_LFO1_WAVEFORM] != 0x01)
  {
    snprintf(patchError, PATCH_ERROR_MAX_LEN, "LFO Waveform: %u is Invalid", p[first_slider + PO_LFO1_WAVEFORM]);
    return 0;
  }

/* bits 7-4 must be 0, bits 2 and 3 cannot both be 1 */
  if ((p[first_slider + PO_PWM_VCO_MOD] | 0x0F) != 0x0F || 
      (p[first_slider + PO_PWM_VCO_MOD] | 0xF3) == 0xFF)
  {
    snprintf(patchError, PATCH_ERROR_MAX_LEN, "PWM, VCO MOD Byte: %x is Invalid", p[first_slider + PO_PWM_VCO_MOD]);
    return 0;
  }

/* bits 7-4 must be 0, bits 1 and 0 cannot both be 1 */
  if ((p[first_slider + PO_VCO1_WAVEFORM] | 0x0F) != 0x0F || 
      (p[first_slider + PO_VCO1_WAVEFORM] | 0xFC) == 0xFF)
  {
    strcpy(patchError, "VCO 1 Waveform is Invalid");
    return 0;
  }

/* bits 7-4 must be 0 */
  if ((p[first_slider + PO_VCO2_WAVEFORM] | 0x0F) != 0x0F)
  {
    strcpy(patchError, "VCO 2 Waveform is Invalid");
    return 0;
  }

/* bits 7-4 must be 0, bits 2 and 3 cannot both be 1 */
  if ((p[first_slider + PO_VCF_SYNC] | 0x0F) != 0x0F ||
      (p[first_slider + PO_VCF_SYNC] | 0xF3) == 0xF3)
  {
    strcpy(patchError, "VCF Sync is Invalid");
    return 0;
  }

/* bits 7-4 must be 0, neither bits 2 and 3 nor bits 1 and 0 can be the same */
  if ((p[first_slider + PO_FILTER_MODE] | 0x0F) != 0x0F ||
      (p[first_slider + PO_FILTER_MODE] | 0xF3) == 0xFF ||
      (p[first_slider + PO_FILTER_MODE] | 0xF3) == 0xF3 ||
      (p[first_slider + PO_FILTER_MODE] | 0xFC) == 0xFF ||
      (p[first_slider + PO_FILTER_MODE] | 0xFC) == 0xFC)
  {
    strcpy(patchError, "Filter Mode is Invalid");
    return 0;
  }

  return 1;
}
