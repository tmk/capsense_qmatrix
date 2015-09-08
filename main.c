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


#define HIZ(P, B)   do { DDR#P &= ~(1<<(B)); PORT#P &= ~(1<<(B)) } while (0)
#define LO(P, B)    do { DDR#P |=  (1<<(B)); PORT#P &= ~(1<<(B)) } while (0)
#define HI(P, B)    do { DDR#P |=  (1<<(B)); PORT#P |=  (1<<(B)) } while (0)
#define HIZ(P)      do { DDR#P = 0x00; PORT#P = 0x00 } while (0)
#define LO(P)       do { DDR#P = 0xFF; PORT#P = 0x00 } while (0)
#define HI(P)       do { DDR#P = 0xFF; PORT#P = 0xFF } while (0)


// x: bust, y: sense
static void burst(uint8_t x, uint8_t y)
{
    // Burst: Hiz
    DDRC= 0x00; PORTC = 0x00;
    // Top: Lo  LED(PD6): on(Hi)
    //DDRD = 0xFF | (1<<6); PORTD = 0x00 | (1<<6);
    DDRD = 0x00; PORTD = 0x00;
    // Bottom: Lo
    DDRF = 0xFF; PORTF = 0x00;
    // Slope:Hiz
    DDRB = 0x00; PORTB = 0x00;

    // Burst
    // lenght: 16-64
    for (int i = 0; i < 64; i++) {
        DDRD &= ~(1<<y); PORTD &= ~(1<<y);   // top: HiZ
        DDRF |=  (1<<y); PORTF &= ~(1<<y);   // bttom: Lo
        //DDRB &= ~(1<<0); PORTB &= ~(1<<0);   // slope: HiZ

        // rising edge
        DDRC |= (1<<x); PORTC |= (1<<x);    // Burst: Hi

        _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); // 1us cycle

        DDRF &= ~(1<<y); PORTF &= ~(1<<y);   // bttom: HiZ
        DDRD |=  (1<<y); PORTD &= ~(1<<y);   // top: Lo
        //DDRB &= ~(1<<0); PORTF &= ~(1<<0);   // slope: HiZ

        // falling edge
        DDRC |= (1<<x); PORTC &= ~(1<<x);   // Burst: Lo

        _NOP();   // 1us cycle
    }
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
    DDRD  |=  (1<<y); PORTD &= ~(1<<y); // top: Lo
    DDRF  &= ~(1<<y); PORTF &= ~(1<<y); // bttom: HiZ
    PORTB |=  (1<<0); DDRB  |=  (1<<0); // slope: Hi

    // Analog Comparator Output
    uint16_t count = 0;
    while (ACSR & (1<<ACO)) {
        count++;
    }

    return count;
}

static void discharge(uint8_t y)
{
    // discharge capacitor top:Lo, bottom: Lo
    DDRD |=  (1<<y); PORTD &= ~(1<<y);  // top: Lo
    DDRF |=  (1<<y); PORTF &= ~(1<<y);  // bttom: Lo
    DDRB |=  (1<<0); PORTB &= ~(1<<0);  // slope: Lo
    _delay_us(100); //wait_us(100);
}


int main(void)
{
    setup_mcu();
    setup_usb();




    // burst signal PC0-7
    DDRC = 0xFF;
    PORTC = 0x00;

    // top: PD0-7  LED(PD6): on(Hi)
    DDRD = (1<<6); PORTD = (1<<6);
    //DDRD = 0x00; PORTD = 0x00;

    // bottom: PF0-7
    DDRF = 0x00;
    PORTF = 0x00;

    // slope: PB0
    DDRB = 0x00;
    PORTB = 0x00;

#define MATRIX_X 8
#define MATRIX_Y 8
    for (;;) {
        cli();

        uint16_t counts[MATRIX_X][MATRIX_Y];
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                burst(x, y);
                counts[x][y] = sense(y);
                discharge(y);
            }
        }

        //xprintf("\r%04X %04X %04X %04X %04X %04X",
        xprintf("\033[H"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n"
                "%04X %04X %04X %04X  %04X %04X %04X %04X\r\n",
                counts[0][0], counts[0][1], counts[0][2], counts[0][3], counts[0][4], counts[0][5], counts[0][6], counts[0][7],
                counts[1][0], counts[1][1], counts[1][2], counts[1][3], counts[1][4], counts[1][5], counts[1][6], counts[1][7],
                counts[2][0], counts[2][1], counts[2][2], counts[2][3], counts[2][4], counts[2][5], counts[2][6], counts[2][7],
                counts[3][0], counts[3][1], counts[3][2], counts[3][3], counts[3][4], counts[3][5], counts[3][6], counts[3][7],
                counts[4][0], counts[4][1], counts[4][2], counts[4][3], counts[4][4], counts[4][5], counts[4][6], counts[4][7],
                counts[5][0], counts[5][1], counts[5][2], counts[5][3], counts[5][4], counts[5][5], counts[5][6], counts[5][7],
                counts[6][0], counts[6][1], counts[6][2], counts[6][3], counts[6][4], counts[6][5], counts[6][6], counts[6][7],
                counts[7][0], counts[7][1], counts[7][2], counts[7][3], counts[7][4], counts[7][5], counts[7][6], counts[7][7]);

        sei();
        _delay_ms(1);

#if !defined(INTERRUPT_CONTROL_ENDPOINT)
        USB_USBTask();
#endif
    }
}
