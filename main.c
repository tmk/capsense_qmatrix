#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "util/print.h"
#include "util/lufa.h"


static void setup_mcu(void)
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
static void burst_hi(uint8_t x) {
    if (x < 8) {
        DDRD |= (1<<x); PORTD |= (1<<x);
    } else {
        DDRC |= (1<<(x&0x07)); PORTC |= (1<<(x&0x07));
    }
}
static void burst_lo(uint8_t x) {
    if (x < 8) {
        DDRD |= (1<<x); PORTD &= ~(1<<x);
    } else {
        DDRC |= (1<<(x&0x07)); PORTC &= ~(1<<(x&0x07));
    }
}
static void burst_lo_all(void) {
    DDRD= 0xFF; PORTD = 0x00;
    DDRC= 0xFF; PORTC = 0x00;
}
static void burst_hi_all(void) {
    DDRD= 0xFF; PORTD = 0xFF;
    DDRC= 0xFF; PORTC = 0xFF;
}
static void burst_hiz_all(void) {
    DDRD= 0x00; PORTD = 0x00;
    DDRC= 0x00; PORTC = 0x00;
}


/* Sense Y lines
 * Y0-7#Top:    PA0-7
 * Y0-7#Bottom: PF0-7
 */
static void top_lo_all(void)        { DDRA = 0xFF; PORTA = 0x00; }
static void top_hi_all(void)        { DDRA = 0xFF; PORTA = 0xFF; }
static void top_hiz_all(void)       { DDRA = 0x00; PORTA = 0x00; }
static void top_lo(uint8_t y)       { DDRA |=  (1<<y); PORTA &= ~(1<<y); }
static void top_hi(uint8_t y)       { DDRA |=  (1<<y); PORTA |=  (1<<y); }
static void top_hiz(uint8_t y)      { DDRA &= ~(1<<y); PORTA &= ~(1<<y); }
static void bottom_lo_all(void)     { DDRF = 0xFF; PORTF = 0x00; }
static void bottom_hi_all(void)     { DDRF = 0xFF; PORTF = 0xFF; }
static void bottom_hiz_all(void)    { DDRF = 0x00; PORTF = 0x00; }
static void bottom_lo(uint8_t y)    { DDRF |=  (1<<y); PORTF &= ~(1<<y); }
static void bottom_hi(uint8_t y)    { DDRF |=  (1<<y); PORTF |=  (1<<y); }
static void bottom_hiz(uint8_t y)   { DDRF &= ~(1<<y); PORTF &= ~(1<<y); }


/* Slope line
 * Slope:       PB0
 */
static void slope_lo(void)      { DDRB = 0xFF; PORTB = 0x00; }
static void slope_hi(void)      { DDRB = 0xFF; PORTB = 0xFF; }
static void slope_hiz(void)     { DDRB = 0x00; PORTB = 0x00; }



/* Charge sample capacitors with burst of pulse on X line
 * all capacitors on same Y line are charged at once
 */
static void burst(uint8_t x)
{
    burst_hiz_all();
    top_lo_all();
    bottom_lo_all();
    slope_hiz();

    // Burst length: 16-64
    for (int i = 0; i < 64; i++) {
        top_hiz_all();
        bottom_lo_all();

        // rising edge
        burst_hi(x);

        _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); // 1us cycle

        bottom_hiz_all();
        top_lo_all();

        // falling edge
        burst_lo(x);

        _NOP();   // 1us cycle
    }
    top_hiz_all();
    bottom_lo_all();
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
    bottom_hiz(y);
    top_lo(y);
    slope_hi();

    // Analog Comparator Output
    uint16_t count = 0;
    cli();
    while (ACSR & (1<<ACO)) {
        count++;
    }
    sei();

    slope_hiz();
    top_hiz(y);
    bottom_hiz(y);

    ADCSRB &= ~(1<<ACME);   // Analog Comparator Multiplexer Disable

    return count;
}


static void discharge(void)
{
    top_lo_all();
    bottom_lo_all();
    slope_lo();
    _delay_us(10);
}



#define ABS(a, b)   ((a > b) ? (a - b) : (b - a))
#define MATRIX_X 16
#define MATRIX_Y 8
#define THRESHOLD_ON    0xA0
#define THRESHOLD_OFF   0x50
uint16_t avg[MATRIX_X][MATRIX_Y];

static void calibrate(void)
{
    memset(avg, 0, sizeof(avg));
    for (uint8_t i = 0; i < 255; i++) {
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            burst(x);
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                avg[x][y] = avg[x][y] - (avg[x][y]>>2) + (sense(y)>>2);
            }
            discharge();
        }
        _delay_ms(1);
    }
}


int main(void)
{
    setup_mcu();
    setup_usb();
    sei();

    burst_lo_all();
    top_hiz_all();
    bottom_hiz_all();
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
            burst(x);
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                s = sense(y);
                counts[x][y] = (counts[x][y] & 0x8000) | s;

                if (s > avg[x][y] + 0x40) {
                    key[x] |= (1<<y);
                    counts[x][y] |= 0x8000;
                }
               if (s < avg[x][y] + 0x30) {
                    key[x] &= ~(1<<y);
                    counts[x][y] &= ~0x8000;
                }
                // when key is off
                if (!(counts[x][y] & 0x8000)) {
                    // TODO: recalibrate threashold according to long time change
                    //avg[x][y] = avg[x][y] - (avg[x][y]>>2) + (s>>2);
                }
            }
            discharge();
        }

        xprintf("\033[H");  // Move cursor to upper left corner
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                xprintf("%04X(%04X) ", counts[x][y], avg[x][y]);
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
