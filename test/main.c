#include <string.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include "capsense.h"
#include "print.h"
#include "lufa.h"


static inline void setup_mcu(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);
}


uint8_t avg[MATRIX_X][MATRIX_Y];
static void calibrate(void)
{
    memset(avg, 0, sizeof(avg));
    for (uint8_t i = 0; i < 64; i++) {
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            // To reduce cross-talk between adjacent Y lines
            // sense two odd and even group alternately.

            // even group(0,2,4,6)
            burst(x, 0x55);
            for (uint8_t y = 0; y < MATRIX_Y; y += 2) {
                avg[x][y] = avg[x][y] - (avg[x][y]>>2) + (sense(y)>>2);
                _delay_us(10);
            }
            discharge_all();

            // odd group(1,3,5,7)
            burst(x, 0xAA);
            for (uint8_t y = 1; y < MATRIX_Y; y += 2) {
                avg[x][y] = avg[x][y] - (avg[x][y]>>2) + (sense(y)>>2);
                _delay_us(10);
            }
            discharge_all();
        }
    }
}
int main(void)
{
    setup_mcu();
    setup_usb();
    sei();

    burst_lo_all();
    top_hiz_mask(0xFF);
    bottom_hiz_mask(0xFF);
    slope_hiz();

    calibrate();

    uint16_t counts[MATRIX_X][MATRIX_Y];
    uint16_t key[MATRIX_X];
    uint16_t s;
    memset(counts, 0, sizeof(counts));
    memset(key, 0, sizeof(key));

    for (;;) {

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

        xprintf("\033[H");  // Move cursor to upper left corner
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                xprintf("%04X(%02X)%c", counts[x][y], avg[x][y], (key[x] & (1<<y) ? '*' : ' '));
            }
            xprintf("\r\n");
        }
        xprintf("\033[J");  // Clear screen from cursor down
        _delay_ms(1);

#if !defined(INTERRUPT_CONTROL_ENDPOINT)
        USB_USBTask();
#endif
    }
}
