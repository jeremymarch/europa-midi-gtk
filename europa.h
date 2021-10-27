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

#define SYSEX_COMMAND_BYTE 5

#define CC_VALUE_BYTE 6

#define PATCH_NAME_LEN_BYTE 8

#define PATCH_NAME_MAX_LEN 16
#define PATCH_NAME_MIN_LEN 0

#define PATCH_HEADER_LENGTH 9 /* F0 - patch_name_len_byte */

#define PATCH_BODY_LENGTH 35 /* first slider - end not including F7 */

#define PATCH_MIN_LEN 45
#define PATCH_MAX_LEN 61

enum sliders {
  LFO1_RATE = 0,
  LFO1_DELAY,
  VCOMOD_LFO,
  VCOMOD_ENV1,
  PWM_PW,
  PWM_PWM,
  VCO1_MANUAL,
  VCO1_ENV1,

  VCO1_RANGE,
  VCO2_RANGE,
  VCO2_FINE_TUNE,
  VCO_MIXER,

  VCF_FREQ,
  VCF_RES,
  VCF_ENV,
  VCF_LFO,
  VCF_KYBD,
  VCA_ENV2,
  VCA_LFO,

  ENV1_A,
  ENV1_D,
  ENV1_S,
  ENV1_R,
  ENV1_KF,
  ENV2_A,
  ENV2_D,
  ENV2_S,
  ENV2_R,
  ENV2_KF,

  NUM_SLIDERS
};

enum continuous_controllers {
  CC_LFO1_RATE         = 0x20,
  CC_LFO1_DELAY        = 0x21,
  CC_VCO_MOD_LFO       = 0x22,
  CC_VCO_MOD_ENV1      = 0x23,
  CC_PWM_PW            = 0x24,
  CC_PWM_PWM           = 0x25,
  CC_VCO1_XMOD_MANUAL  = 0x26,
  CC_VCO1_XMOD_ENV1    = 0x27,

  /* KNOBS */
  CC_VC01_RANGE        = 0x28,
  CC_VCO2_RANGE        = 0x29,
  CC_VC02_FINE_TUNE    = 0x2A,
  CC_VCO_MIXER         = 0x2B,
  /* END KNOBS */

  CC_VCF_FREQ          = 0x2C,
  CC_VCF_RES           = 0x2D,
  CC_VCF_ENV           = 0x2E,
  CC_VCF_LFO1          = 0x2F,
  CC_VCF_KEY_MOD       = 0x30,
  CC_VCA_ENV2          = 0x31,
  CC_VCA_LFO           = 0x32,
  CC_ENV1_A            = 0x33,
  CC_ENV1_D            = 0x34,
  CC_ENV1_S            = 0x35,
  CC_ENV1_R            = 0x36,
  CC_ENV1_KF           = 0x37,
  CC_ENV2_A            = 0x38,
  CC_ENV2_D            = 0x39,
  CC_ENV2_S            = 0x3A,
  CC_ENV2_R            = 0x3B,
  CC_ENV2_KF           = 0x3C,

  /* KNOBS */
  CC_ARP_RATE          = 0x3D, 
  CC_ARP_GLIDE_TIME    = 0x3E, 
  CC_UNISON_DETUNE     = 0x3F, 

  CC_SUSTAIN_PEDAL     = 0x40,
  CC_PORTAMENTO        = 0x41,

  /* BUTTONS */
  CC_LFO1_WAVEFORM     = 0x42,
  CC_VCO_MOD           = 0x43,           /* DOCS REVERSED P. 60 */
  CC_PWM_MOD           = 0x44,           /* DOCS REVERSED P. 60 */
  CC_VCO1_WAVEFORM     = 0x45,
  CC_VCO2_WAVEFORM     = 0x46,
  CC_VCF_FILTER_MODE   = 0x47,
  CC_VCO_SYNC_MODE     = 0x48,
  CC_VCF_ENV_MOD       = 0x49,           /* DOCS REVERSED P. 61 */
  CC_ENV1_POLARITY     = 0x4A,           /* DOCS REVERSED P. 61 */ 
  CC_KEY_MODE          = 0x4B,           /* 2/4 OR 4/2 OR WHOLE */
  CC_PANEL_MODE        = 0x4C,           /* UPPER/LOWER */
  CC_VOICE_ASSIGN_MODE = 0x4D,
  CC_MISC              = 0x4E,           /* HOLD, GLISS, PORTA, BENDER */

  /* KNOBS */
  CC_MASTER_TUNE       = 0x4F,
  CC_SPLIT_BALANCE     = 0x50,
  CC_ARP_SYNC          = 0x51,
  CC_ARP_MODE          = 0x52,
  CC_ARP_CLOCK_DIV     = 0x53,

  /* BUTTONS */
  CC_ARP_DIRECTION     = 0x54,
  CC_ARP_RANGE         = 0x55,
  CC_SEQ_RECORD        = 0x56,
  CC_SEQ_REST          = 0x57,
  CC_ARP_RHYTHM        = 0x58,
  CC_ARP_OPTIONS       = 0x59,
  CC_EUROPA_SOFT_OPTS  = 0x5A,      /* SYSEX TRANSMIT, VOICE WATCH, ETC */
  CC_MANUAL_MODE       = 0x5B,
  CC_PRESET_MODE       = 0x5C,
  CC_SPLIT_POINT       = 0x5D
};

enum patch_offsets {
  PO_LFO1_RATE = 0,
  PO_LFO1_DELAY,
  PO_VCO_MOD_LFO,
  PO_VCO_MOD_ENV1,
  PO_PWM_PW,
  PO_PWM_PWM,
  PO_VCO1_XMOD_MANUAL,
  PO_VCO1_XMOD_ENV1,
  PO_VC01_RANGE,
  PO_VCO2_RANGE,
  PO_VC02_FINE_TUNE,
  PO_VCO_MIXER,
  PO_VCF_FREQ,
  PO_VCF_RES,
  PO_VCF_ENV,
  PO_VCF_LFO1,
  PO_VCF_KEY_MOD,
  PO_VCA_ENV2,
  PO_VCA_LFO,
  PO_ENV1_A,
  PO_ENV1_D,
  PO_ENV1_S,
  PO_ENV1_R,
  PO_ENV1_KF,
  PO_ENV2_A,
  PO_ENV2_D,
  PO_ENV2_S,
  PO_ENV2_R,
  PO_ENV2_KF,
  PO_LFO1_WAVEFORM,
  PO_PWM_VCO_MOD,
  PO_VCO1_WAVEFORM,
  PO_VCO2_WAVEFORM,
  PO_VCF_SYNC,
  PO_FILTER_MODE, 
  PO_NUM
};

enum commands {
  CMD_LOAD_EUROPA_DEFAULTS  = 0x00,
  CMD_LOAD_FACTORY_PATCHES  = 0x01, /* and Presets */
  CMD_SET_MIDI_CHANNEL      = 0x02,
  CMD_GEN_RANDOM_PATCH      = 0x03,
  CMD_REQUEST_DUMP          = 0x04,
  CMD_WRITE_TO_FLASH_NVRAM  = 0x05,
  CMD_WRITE_EDIT_TO_FLASH   = 0x06,
  CMD_ASSIGN_CONTROLLER     = 0x07
};

enum dump_type {
  PATCH_DUMP    = 0x00,
  PRESET_DUMP   = 0x10,
  SEQUENCE_DUMP = 0x30
};
