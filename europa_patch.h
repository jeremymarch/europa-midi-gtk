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

typedef struct {
  GtkWidget *slider;
  GtkWidget *label;
  GtkObject *adjustment;
  GtkWidget *vBox;
} vSliders;

typedef struct {
  GtkWidget *window;
  GtkWidget *lfo_wave_tri;
  GtkWidget *lfo_wave_saw;
  GtkWidget *lfo_wave_square;
  GtkWidget *lfo_wave_random;
  GtkWidget *vco_mod_vco1;
  GtkWidget *vco_mod_vco2;
  GtkWidget *pwm_env1;
  GtkWidget *pwm_lfo;
  GtkWidget *vco1_wave_tri;
  GtkWidget *vco1_wave_saw;
  GtkWidget *vco1_wave_pulse;
  GtkWidget *vco1_wave_square;
  GtkWidget *vco_sync_r;
  GtkWidget *vco_sync_l;
  GtkWidget *vco2_wave_tri;
  GtkWidget *vco2_wave_saw;
  GtkWidget *vco2_wave_pulse;
  GtkWidget *vco2_wave_noise;
  GtkWidget *vcf_hpf;
  GtkWidget *vcf_lpf;
  GtkWidget *vcf_env1;
  GtkWidget *vcf_env2;
  GtkWidget *env1_pos;
  GtkWidget *env1_neg;
  GtkWidget *name;
  GtkWidget *statusBar;
  vSliders   sliders[NUM_SLIDERS];
} EuropaPatchForm;

void patch_received (EuropaPatchForm *patchForm, guchar *patch, int len);

EuropaPatchForm *draw_panel();

gboolean poll_func (GIOChannel *chan, GIOCondition con, gpointer patchForm);
