#include <avr/pgmspace.h>
#include "keycode.h"


/* Realforce 106S
 * ,---.   ,---------------. ,---------------. ,---------------.  ,-----------.
 * |Esc|   |F1 |F2 |F3 |F4 | |F5 |F6 |F7 |F8 | |F9 |F10|F11|F12|  |Psc|Slk|Pau|
 * `---'   `---------------' `---------------' `---------------'  `-----------'
 * ,-----------------------------------------------------------.  ,-----------.  ,---------------.
 * |`  |1  |2  |3  |4  |5  |6  |7  |8  |9  |-  |=  |\  |   |Bs |  |Ins|Hom|PgU|  |Nlk|/  |*  |-  |
 * |-----------------------------------------------------------|  |-----------|  |---------------|
 * |Tab  |Q  |W  |E  |R  |T  |Y  |U  |I  |O  |P  |[  |]  |Ret  |  |Del|End|PgD|  |7  |8  |9  |   |
 * |------------------------------------------------------`    |  `-----------'  |-----------|+  |
 * |Ctrl  |A  |S  |D  |F  |G  |H  |J  |K  |L  |;  |'  |\  |    |                 |4  |5  |6  |   |
 * |-----------------------------------------------------------|      ,---.      |---------------|
 * |Shift   |Z  |X  |C  |V  |B  |N  |M  |,  |.  |/  |Shift     |      |Up |      |1  |2  |3  |   |
 * |-----------------------------------------------------------|  ,-----------.  |-----------|Ent|
 * |     |   |     |         Spc            |    |     |       |  |Lef|Dow|Rig|  |0      |.  |   |
 * `-----------------------------------------------------------'  `-----------'  `---------------'
 */
#define KEYMAP( \
    K07,    K16,K17,K27,K37,  K57,K56,K67,K77,  K86,K87,K96,K97,   KE7,KE6,KF7, \
    K04,K05,K14,K15,K25,K35,K44,K54,K55,K64,K74,K84,K94,KA5,KA4,   KB4,KC5,KC4,  KD4,KE5,KE4,KF5, \
    K03,K13,K23,K24,K33,K34,K43,K53,K62,K63,K73,K83,K93,    KA2,   KB3,KB2,KC3,  KD3,KE3,KF4,KF3, \
    K02,K12,K22,K21,K32,K41,K42,K52,K61,K72,K82,K92,KA1,                         KD2,KE2,KF2, \
    K01,K11,K20,K31,K30,K40,K51,K60,K71,K70,K81,K91,        K90,       KB1,      KD1,KE1,KF1,KF0, \
    K00,K06,K10,   K36,K46,  K50,  K45,K66,  K76,   K80,KA6,KA0,   KB0,KC0,KC1,  KD0,    KE0 \
) { \
    { KC_##K00, KC_##K01, KC_##K02, KC_##K03, KC_##K04, KC_##K05, KC_##K06, KC_##K07 }, \
    { KC_##K10, KC_##K11, KC_##K12, KC_##K13, KC_##K14, KC_##K15, KC_##K16, KC_##K17 }, \
    { KC_##K20, KC_##K21, KC_##K22, KC_##K23, KC_##K24, KC_##K25, KC_NO,    KC_##K27 }, \
    { KC_##K30, KC_##K31, KC_##K32, KC_##K33, KC_##K34, KC_##K35, KC_##K36, KC_##K37 }, \
    { KC_##K40, KC_##K41, KC_##K42, KC_##K43, KC_##K44, KC_##K45, KC_##K46, KC_NO    }, \
    { KC_##K50, KC_##K51, KC_##K52, KC_##K53, KC_##K54, KC_##K55, KC_##K56, KC_##K57 }, \
    { KC_##K60, KC_##K61, KC_##K62, KC_##K63, KC_##K64, KC_NO,    KC_##K66, KC_##K67 }, \
    { KC_##K70, KC_##K71, KC_##K72, KC_##K73, KC_##K74, KC_NO,    KC_##K76, KC_##K77 }, \
    { KC_##K80, KC_##K81, KC_##K82, KC_##K83, KC_##K84, KC_NO,    KC_##K86, KC_##K87 }, \
    { KC_##K90, KC_##K91, KC_##K92, KC_##K93, KC_##K94, KC_NO,    KC_##K96, KC_##K97 }, \
    { KC_##KA0, KC_##KA1, KC_##KA2, KC_NO,    KC_##KA4, KC_##KA5, KC_##KA6, KC_NO    }, \
    { KC_##KB0, KC_##KB1, KC_##KB2, KC_##KB3, KC_##KB4, KC_NO,    KC_NO,    KC_NO    }, \
    { KC_##KC0, KC_##KC1, KC_NO,    KC_##KC3, KC_##KC4, KC_##KC5, KC_NO,    KC_NO    }, \
    { KC_##KD0, KC_##KD1, KC_##KD2, KC_##KD3, KC_##KD4, KC_NO,    KC_NO,    KC_NO    }, \
    { KC_##KE0, KC_##KE1, KC_##KE2, KC_##KE3, KC_##KE4, KC_##KE5, KC_##KE6, KC_##KE7 }, \
    { KC_##KF0, KC_##KF1, KC_##KF2, KC_##KF3, KC_##KF4, KC_##KF5, KC_NO,    KC_##KF7 } \
}


const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    [0] = KEYMAP(
ESC, F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9,  F10, F11, F12,            PSCR,SLCK,PAUS,
ESC, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   MINS,EQL, BSLS,BSPC, INS, HOME,PGUP,  NLCK,PSLS,PAST,PMNS,
TAB, Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   LBRC,RBRC,     ENT,  DEL, END, PGDN,  P7,  P8,  P9,  PPLS,
LCTL,A,   S,   D,   F,   G,   H,   J,   K,   L,   SCLN,QUOT,GRV,                             P4,  P5,  P6,
LSFT,Z,   X,   C,   V,   B,   N,   M,   COMM,DOT, SLSH,RO,           RSFT,       UP,         P1,  P2,  P3,  PENT,
LGUI,NO,  LALT,     MHEN,NO,  SPC, NO,  HENK,KANA,           RALT,NO,RGUI,  LEFT,DOWN,RGHT,  P0,       PDOT),
};

const uint16_t fn_actions[] PROGMEM = {
};
