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

void
patch_received (EuropaPatchForm *patchForm, guchar *patch, int len);
