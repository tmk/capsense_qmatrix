#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "util/print.h"
#include "util/lufa.h"


static inline void setup_mcu(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);
}



/* Burst X lines
 * X0-7:    PD0-7
 * X8-15:   PC0-7
 */
__attribute__ ((always_inline))
static inline void burst_hi(uint8_t x) {
    if (x < 8) {
        DDRD |= (1<<x); PORTD |= (1<<x);
    } else {
        DDRC |= (1<<(x&0x07)); PORTC |= (1<<(x&0x07));
    }
}
__attribute__ ((always_inline))
static inline void burst_lo(uint8_t x) {
    if (x < 8) {
        DDRD |= (1<<x); PORTD &= ~(1<<x);
    } else {
        DDRC |= (1<<(x&0x07)); PORTC &= ~(1<<(x&0x07));
    }
}
__attribute__ ((always_inline))
static inline void burst_lo_all(void)  { DDRD= 0xFF; PORTD = 0x00; DDRC= 0xFF; PORTC = 0x00; }
//static inline void burst_hi_all(void)  { DDRD= 0xFF; PORTD = 0xFF; DDRC= 0xFF; PORTC = 0xFF; }
//static inline void burst_hiz_all(void) { DDRD= 0x00; PORTD = 0x00; DDRC= 0x00; PORTC = 0x00; }


/* Sense Y lines
 * Y0-7#Top:    PA0-7
 * Y0-7#Bottom: PF0-7
 */
__attribute__ ((always_inline))
static inline void top_lo_mask(uint8_t m)      { DDRA |=  m; PORTA &= ~m; }
//static inline void top_hi_mask(uint8_t m)      { DDRA |=  m; PORTA |=  m; }
__attribute__ ((always_inline))
static inline void top_hiz_mask(uint8_t m)     { DDRA &= ~m; PORTA &= ~m; }
__attribute__ ((always_inline))
static inline void bottom_lo_mask(uint8_t m)   { DDRF |=  m; PORTF &= ~m; }
//static inline void bottom_hi_mask(uint8_t m)   { DDRF |=  m; PORTF |=  m; }
__attribute__ ((always_inline))
static inline void bottom_hiz_mask(uint8_t m)  { DDRF &= ~m; PORTF &= ~m; }


/* Slope line
 * Slope:       PB0
 */
__attribute__ ((always_inline))
static inline void slope_lo(void)      { DDRB |=  (1<<0); PORTB &= ~(1<<0); }
__attribute__ ((always_inline))
static inline void slope_hi(void)      { DDRB |=  (1<<0); PORTB |=  (1<<0); }
__attribute__ ((always_inline))
static inline void slope_hiz(void)     { DDRB &= ~(1<<0); PORTB &= ~(1<<0); }



/* Charge sample capacitors with burst of pulse on X line
 * all capacitors on same Y line are charged at once
 */
static void burst(uint8_t x, uint8_t ymask)
{
    burst_lo_all();
    top_lo_mask(ymask);
    bottom_lo_mask(ymask);
    slope_hiz();

    // Burst length: 16-64
    for (uint8_t i = 0; i < 64; i++) {
        top_hiz_mask(ymask);
        bottom_lo_mask(ymask);

        // rising edge
        burst_hi(x);

        //_NOP(); _NOP(); _NOP(); _NOP(); _NOP(); // 1us cycle

        bottom_hiz_mask(ymask);
        top_lo_mask(ymask);

        // falling edge
        burst_lo(x);

        //_NOP();   // 1us cycle
    }
    top_hiz_mask(ymask);
    bottom_lo_mask(ymask);
}


static uint16_t sense(uint8_t y)
{
    // Analog Comparator setup
    // input+: AIN0 = GND
    // input-: Multiplexer = bottom
    ADMUX   =  y;           // bottom - Multiplexer select
    ADCSRA &= ~(1<<ADEN);   // ADC disable
    ADCSRB |=  (1<<ACME);   // Analog Comparator Multiplexer Enable
    //ACSR |=  (1<<ACIC);   // Analog Comparator Input Capture Enable

    // sense
    bottom_hiz_mask(1<<y);
    top_lo_mask(1<<y);
    slope_hi();

    // Analog Comparator Output
    uint16_t count = 0;
    cli();
    while (ACSR & (1<<ACO)) {
        count++;
    }
    sei();

    slope_hiz();
    top_hiz_mask(1<<y);
    bottom_hiz_mask(1<<y);

    ADCSRB &= ~(1<<ACME);   // Analog Comparator Multiplexer Disable

    // discharge
    top_lo_mask(1<<y);
    bottom_lo_mask(1<<y);

    return count;
}


static void discharge_all(void)
{
    top_lo_mask(0xFF);
    bottom_lo_mask(0xFF);
    slope_lo();
    //_delay_us(10);
}



#define ABS(a, b)   ((a > b) ? (a - b) : (b - a))
#define MATRIX_X 16
#define MATRIX_Y 8
#define THRESHOLD_ON    0x50
#define HYSTERESIS      0x20
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
