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


/*
#define HIZ(P, B)   do { DDR#P &= ~(1<<(B)); PORT#P &= ~(1<<(B)) } while (0)
#define LO(P, B)    do { DDR#P |=  (1<<(B)); PORT#P &= ~(1<<(B)) } while (0)
#define HI(P, B)    do { DDR#P |=  (1<<(B)); PORT#P |=  (1<<(B)) } while (0)
*/
#define HIZ(P)      do { DDR##P = 0x00; PORT##P = 0x00; } while (0)
#define LO(P)       do { DDR##P = 0xFF; PORT##P = 0x00; } while (0)
#define HI(P)       do { DDR##P = 0xFF; PORT##P = 0xFF; } while (0)


// x: burst, y: sense
static void burst(uint8_t x, uint8_t y)
{
    // Burst: Hiz
    DDRD= 0xFF; PORTD = 0x00;
    // Top: Lo
    DDRA = 0x00; PORTA = 0x00;
    // Bottom: Lo
    DDRF = 0xFF; PORTF = 0x00;
    // Slope:Hiz
    DDRB = 0x00; PORTB = 0x00;

    // Burst
    // lenght: 16-64
    for (int i = 0; i < 64; i++) {
        HIZ(A);
        LO(F);
        //DDRA &= ~(1<<y); PORTA &= ~(1<<y);   // top: HiZ
        //DDRF |=  (1<<y); PORTF &= ~(1<<y);   // bttom: Lo

        // rising edge
        DDRD |= (1<<x); PORTD |= (1<<x);    // Burst: Hi

        _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); // 1us cycle

        HIZ(F);
        LO(A);
        //DDRF &= ~(1<<y); PORTF &= ~(1<<y);   // bttom: HiZ
        //DDRA |=  (1<<y); PORTA &= ~(1<<y);   // top: Lo

        // falling edge
        DDRD |= (1<<x); PORTD &= ~(1<<x);   // Burst: Lo

        _NOP();   // 1us cycle
    }
        HIZ(A);
        LO(F);
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
    DDRF  &= ~(1<<y); PORTF &= ~(1<<y); // bttom: HiZ
    DDRA  |=  (1<<y); PORTA &= ~(1<<y); // top: Lo
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
    DDRA |=  (1<<y); PORTA &= ~(1<<y);  // top: Lo
    DDRF |=  (1<<y); PORTF &= ~(1<<y);  // bttom: Lo
    DDRB |=  (1<<0); PORTB &= ~(1<<0);  // slope: Lo
    _delay_us(50); //wait_us(100);
}


int main(void)
{
    setup_mcu();
    setup_usb();




    // burst signal
    DDRD = 0xFF;
    PORTD = 0x00;

    // top:
    DDRA = 0x00; PORTA = 0x00;

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
            burst(x, 0);
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                //burst(x, y);
                counts[x][y] = sense(y);
                //discharge(y);
            }
            //discharge(0);
            // discharge capacitor top:Lo, bottom: Lo
            LO(A); //DDRA |=  (1<<y); PORTA &= ~(1<<y);  // top: Lo
            LO(F); //DDRF |=  (1<<y); PORTF &= ~(1<<y);  // bttom: Lo
            //DDRB |=  (1<<0); PORTB &= ~(1<<0);  // slope: Lo
            //DDRB = 0xFF; PORTB = 0x00;
            //_delay_us(50); //wait_us(100);
        }

        sei();

        xprintf("\033[H");
        for (uint8_t x = 0; x < MATRIX_X; x++) {
            for (uint8_t y = 0; y < MATRIX_Y; y++) {
                xprintf("%04X ", counts[x][y]);
            }
            xprintf("\r\n");
        }

        //_delay_ms(1);

#if !defined(INTERRUPT_CONTROL_ENDPOINT)
        USB_USBTask();
#endif
    }
}
