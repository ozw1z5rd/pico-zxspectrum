#include "ZxSpectrumHidKeyboard.h"
#include "stdlib.h"
#include <pico/printf.h>

#define BIT0  0x01
#define BIT1  0x02
#define BIT2  0x04
#define BIT3  0x08
#define BIT4  0x10

typedef struct  {
  uint8_t line;
  uint8_t key;
} ZxSpectrumKeyContact;

typedef struct {
  uint8_t keycode;
  uint8_t contacts;
  ZxSpectrumKeyContact contact[2];
} ZxSpectrumHidKey;

ZxSpectrumHidKey KEYS[] = {
  { HID_KEY_A,           1, { {0, BIT0} }},
  { HID_KEY_S,           1, { {0, BIT1} }},
  { HID_KEY_D,           1, { {0, BIT2} }},
  { HID_KEY_F,           1, { {0, BIT3} }},
  { HID_KEY_G,           1, { {0, BIT4} }},

  { HID_KEY_Q,           1, { {1, BIT0} }},
  { HID_KEY_W,           1, { {1, BIT1} }},
  { HID_KEY_E,           1, { {1, BIT2} }},
  { HID_KEY_R,           1, { {1, BIT3} }},
  { HID_KEY_T,           1, { {1, BIT4} }},

  { HID_KEY_SHIFT_LEFT,  1, { {2, BIT0} }},
  { HID_KEY_SHIFT_RIGHT, 1, { {2, BIT0} }},
  { HID_KEY_Z,           1, { {2, BIT1} }},
  { HID_KEY_X,           1, { {2, BIT2} }},
  { HID_KEY_C,           1, { {2, BIT3} }},
  { HID_KEY_V,           1, { {2, BIT4} }},

  { HID_KEY_1,           1, { {3, BIT0} }},
  { HID_KEY_2,           1, { {3, BIT1} }},
  { HID_KEY_3,           1, { {3, BIT2} }},
  { HID_KEY_4,           1, { {3, BIT3} }},
  { HID_KEY_5,           1, { {3, BIT4} }},

  { HID_KEY_0,           1, { {4, BIT0} }},
  { HID_KEY_9,           1, { {4, BIT1} }},
  { HID_KEY_8,           1, { {4, BIT2} }},
  { HID_KEY_7,           1, { {4, BIT3} }},
  { HID_KEY_6,           1, { {4, BIT4} }},

  { HID_KEY_P,           1, { {5, BIT0} }},
  { HID_KEY_O,           1, { {5, BIT1} }},
  { HID_KEY_I,           1, { {5, BIT2} }},
  { HID_KEY_U,           1, { {5, BIT3} }},
  { HID_KEY_Y,           1, { {5, BIT4} }},

  { HID_KEY_ENTER,       1, { {6, BIT0} }},
  { HID_KEY_L,           1, { {6, BIT1} }},
  { HID_KEY_K,           1, { {6, BIT2} }},
  { HID_KEY_J,           1, { {6, BIT3} }},
  { HID_KEY_H,           1, { {6, BIT4} }},

  { HID_KEY_SPACE,       1, { {7, BIT0} }},
  { HID_KEY_ALT_RIGHT,   1, { {7, BIT1} }},
  { HID_KEY_M,           1, { {7, BIT2} }},
  { HID_KEY_N,           1, { {7, BIT3} }},
  { HID_KEY_B,           1, { {7, BIT4} }},
  
  { HID_KEY_BACKSPACE,   2, { {2, BIT0}, {4, BIT0} }},
  { HID_KEY_PAUSE,       2, { {2, BIT0}, {7, BIT0} }},
  { HID_KEY_COMMA,       2, { {7, BIT1}, {7, BIT3} }},
  { HID_KEY_PERIOD,      2, { {7, BIT1}, {7, BIT2} }},
  { HID_KEY_SLASH,       2, { {7, BIT1}, {0, BIT4} }},
  { HID_KEY_SEMICOLON,   2, { {7, BIT1}, {5, BIT1} }},
  { HID_KEY_APOSTROPHE,  2, { {7, BIT1}, {4, BIT3} }},
  { HID_KEY_MINUS,       2, { {7, BIT1}, {6, BIT3} }},
  { HID_KEY_EQUAL,       2, { {7, BIT1}, {6, BIT1} }},
  { HID_KEY_EUROPE_1,    2, { {7, BIT1}, {3, BIT2} }},

  { HID_KEY_ARROW_LEFT,  2, { {2, BIT0}, {3, BIT4} }},
  { HID_KEY_ARROW_DOWN,  2, { {2, BIT0}, {4, BIT4} }},
  { HID_KEY_ARROW_UP,    2, { {2, BIT0}, {4, BIT3} }},
  { HID_KEY_ARROW_RIGHT, 2, { {2, BIT0}, {4, BIT2} }}
};

static bool KEYS_SORTED  = false;

static int keys_comparator(const ZxSpectrumHidKey *k1, const ZxSpectrumHidKey *k2) {
  int kk1 = (int)k1->keycode;
  int kk2 = (int)k2->keycode;
  return kk2 - kk1;
}

static int keys_v_comparator(const void *k1, const void *k2) {
  return keys_comparator((ZxSpectrumHidKey *)k1, (ZxSpectrumHidKey *)k2);
}

const static unsigned int KEYS_LEN = sizeof(KEYS) / sizeof(ZxSpectrumHidKey);

const static unsigned int KEY_SIZE = sizeof(ZxSpectrumHidKey);

static void sort_keys() {
  if (!KEYS_SORTED) {
    qsort(KEYS, KEYS_LEN, KEY_SIZE, keys_v_comparator);
  }
  KEYS_SORTED = true;
}

static ZxSpectrumHidKey* findKey(const unsigned char keycode) {
  if (keycode <= 1) return 0;
  const ZxSpectrumHidKey k0 = {keycode, 0, 0};
  return (ZxSpectrumHidKey *) bsearch(&k0, KEYS, KEYS_LEN, KEY_SIZE, keys_v_comparator);
}

static bool isInReport(hid_keyboard_report_t const *report, const unsigned char keycode) {
  for(unsigned int i = 0; i < 6; i++) {
    if (report->keycode[i] == keycode)  return true;
  }
  return false;
}

ZxSpectrumHidKeyboard::ZxSpectrumHidKeyboard(
  ZxSpectrumFileLoop *zxSpectrumSnapList,
  QuickSave* quickSave, 
  ZxSpectrumJoystick * zxSpectrumJoystick,
  ZxSpectrumMouse * zxSpectrumMouse  
) :
  ZxSpectrumKeyboard(zxSpectrumJoystick, zxSpectrumMouse),
  _zxSpectrumSnapList(zxSpectrumSnapList),
  _quickSave(quickSave),
  _kiosk(false)
{
  sort_keys();
}

ZxSpectrumHidKeyboard::ZxSpectrumHidKeyboard(
  QuickSave* quickSave, 
  ZxSpectrumJoystick * zxSpectrumJoystick,
    ZxSpectrumMouse * zxSpectrumMouse
) :
  ZxSpectrumKeyboard(zxSpectrumJoystick, zxSpectrumMouse),
  _zxSpectrumSnapList(0),
  _quickSave(quickSave),
  _kiosk(false)
{
  sort_keys();
}

static uint8_t func_keys[] = {
  HID_KEY_F1,
  HID_KEY_F2,
  HID_KEY_F3,
  HID_KEY_F4,
  HID_KEY_F5,
  HID_KEY_F6,
  HID_KEY_F7,
  HID_KEY_F8,
  HID_KEY_F9,
  HID_KEY_F10,
  HID_KEY_F11,
  HID_KEY_F12,
  HID_KEY_F13,
  HID_KEY_F14  
};

int ZxSpectrumHidKeyboard::processHidReport(hid_keyboard_report_t const *report, hid_keyboard_report_t const *prev_report) {
  int r = 0;

  reset();
  if (report->keycode[0] == 1) return r;
  const unsigned char m = report->modifier;
  // printf("hid:%02X %02X%02X%02X%02X%02X%02X\n", m, report->keycode[0], report->keycode[1], report->keycode[2], report->keycode[3], report->keycode[4], report->keycode[5]);

  if (m & 0x22) press(2, BIT0); // Shift
  if (m & 0x40) press(7, BIT1); // AtlGr -> Symbol

  uint32_t fkd = 0;
  uint32_t fkp = 0;
  
  for(unsigned int i = 0; i < 6; ++i) {
    const unsigned char hidKeyCode = report->keycode[i];
    
    for (unsigned int fk = 0; fk < sizeof(func_keys); ++fk) {
      const uint32_t fkb = 1 << fk;
      const unsigned char fkc = func_keys[fk];
      if (hidKeyCode == fkc) {
        fkd |= fkb;
        if (!isInReport(prev_report, fkc)) fkp |= fkb;
      }
    }

    const ZxSpectrumHidKey *k = findKey(hidKeyCode);
    if (k) {
      for (uint32_t c = 0; c < k->contacts; ++c) {
        const ZxSpectrumKeyContact *contact = &k->contact[c];
        press(contact->line, contact->key);
      }
    }
  }

  if ((m & HID_KEY_MOD_SAVE) && !_kiosk) {
    for (int i = 0; i < 12; ++i) {
      if (fkp & (1 << i)) {
        if (_quickSave) _quickSave->save(_ZxSpectrum, i);
      }
    }
  }
  else if (m & HID_KEY_MOD_LOAD) {
    for (int i = 0; i < 12; ++i) {
      if (fkp & (1 << i)) {
        if (_quickSave) _quickSave->load(_ZxSpectrum, i);
      }
    }
  }
  else {
    // F1 open menu
    if ((fkp & (1 << 0)) && !_kiosk) r = 1;
    
    // F3 toggle mute
    if ((fkp & (1 << 2))) _ZxSpectrum->toggleMute();    
    
    if ((fkp & (1 << 12)) && _quickSave) _quickSave->save(_ZxSpectrum, 0);
    if ((fkp & (1 << 13)) && _quickSave) _quickSave->load(_ZxSpectrum, 0);

    // F11 reset to 48k
    if (fkp & (1 << 10)) _ZxSpectrum->reset(ZxSpectrum48k);
    // F11 reset to 128k
    if (fkp & (1 << 11)) _ZxSpectrum->reset(ZxSpectrum128k);
    // F4 toggle moderate
    if (fkp & (1 << 3)) _ZxSpectrum->toggleModerate();
    
    if (_zxSpectrumSnapList) {
      // F8 curr snap
      if (fkp & (1 << 7)) _zxSpectrumSnapList->curr(_ZxSpectrum);
      // F9 previous snap
      if (fkp & (1 << 8)) _zxSpectrumSnapList->prev(_ZxSpectrum);
      // F10 next snap
      if (fkp & (1 << 9)) _zxSpectrumSnapList->next(_ZxSpectrum);
    }
  }
  
  return r;
}
