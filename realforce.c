/*
Copyright 2015 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <avr/pgmspace.h>
#include "matrix.h"
#include "led.h"
#include "keymap.h"
#include "capsense.h"
#include "debug.h"


uint16_t counts[MATRIX_X][MATRIX_Y];
uint16_t key[MATRIX_X];

uint8_t matrix_rows(void) { return MATRIX_ROWS; }
uint8_t matrix_cols(void) { return MATRIX_COLS; }
void matrix_setup(void) {
    //debug_enable = true;
    //debug_matrix = true;
}
void matrix_init(void) { 
    memset(counts, 0, sizeof(counts));
    memset(key, 0, sizeof(key));
    calibrate(); 
}
uint8_t matrix_scan(void) {
    uint16_t s;
    // Scan matrix
    for (uint8_t x = 0; x < MATRIX_X; x++) {
        // even group(0,2,4,6)
        burst(x, 0x55);
        for (uint8_t y = 0; y < MATRIX_Y; y += 2) {
            s = sense(y);
            counts[x][y] = s;
            if (s > THRESHOLD_ON) key[x] |= (1<<y);
            if (s < THRESHOLD_ON - HYSTERESIS) key[x] &= ~(1<<y);
            if (key[x] & (1<<y)) counts[x][y] |= 0x1000;
        }
        discharge_all();

        // odd group(1,3,5,7)
        burst(x, 0xAA);
        for (uint8_t y = 1; y < MATRIX_Y; y += 2) {
            s = sense(y);
            counts[x][y] = s;
            if (s > THRESHOLD_ON) key[x] |= (1<<y);
            if (s < THRESHOLD_ON - HYSTERESIS) key[x] &= ~(1<<y);
            if (key[x] & (1<<y)) counts[x][y] |= 0x1000;
        }
        discharge_all();
    }
    return 0;
}
bool matrix_is_on(uint8_t row, uint8_t col) {
    return key[row] & (1<<col);
}
matrix_row_t matrix_get_row(uint8_t row) {
    return key[row];
}
void matrix_print(void) {
    for (uint8_t x = 0; x < MATRIX_X; x++) {
        for (uint8_t y = 0; y < MATRIX_Y; y++) {
            xprintf("%04X(%02X)%c", counts[x][y], avg[x][y], (key[x] & (1<<y) ? '*' : ' '));
        }
        xprintf("\r\n");
    }
}
void matrix_power_up(void) {}
void matrix_power_down(void) {}

void led_set(uint8_t usb_led) {}


/* translates key to keycode */
extern const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS];
extern const uint16_t fn_actions[];
uint8_t keymap_key_to_keycode(uint8_t layer, keypos_t key)
{
    return pgm_read_byte(&keymaps[(layer)][(key.row)][(key.col)]);
}

/* translates Fn keycode to action */
action_t keymap_fn_to_action(uint8_t keycode)
{
    return (action_t){ .code = pgm_read_word(&fn_actions[FN_INDEX(keycode)]) };
}
