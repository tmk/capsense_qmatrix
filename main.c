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




int main(void)
{
    setup_mcu();
    setup_usb();



    // LED on
    DDRD = (1<<6);
    PORTD = (1<<6);

    // burst signal
    DDRC = 0xFF;
    PORTC = 0x00;

    // top: PF0
    // bottom: PF1
    // slope: PF2
    DDRF = 0x00;
    PORTF = 0x00;

    //cli();

    for (;;) {
        cli();
        DDRC= 0xFF;
        PORTC = 0x00;
        DDRF = 0x00;
        PORTF = 0x00;

        for (int i = 0; i < 64; i++) {
            DDRF &= ~(1<<0); PORTF &= ~(1<<0);   // top: HiZ
            DDRF |=  (1<<1); PORTF &= ~(1<<1);   // bttom: Lo
            //DDRF &= ~(1<<2); PORTF &= ~(1<<2);   // slope: HiZ

            // rising edge
            PORTC = 0x01;

            _NOP(); _NOP(); _NOP(); _NOP(); _NOP(); // 1us cycle
            //_delay_us(5);


            DDRF &= ~(1<<1); PORTF &= ~(1<<1);   // bttom: HiZ
            DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo
            //DDRF &= ~(1<<2); PORTF &= ~(1<<2);   // slope: HiZ

            // falling edge
            PORTC = 0x00;

            _NOP();   // 1us cycle
            //_delay_us(5);
        }

        // sense
        DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo 
        DDRF &= ~(1<<1); PORTF &= ~(1<<1);   // bttom: HiZ
        PORTF |=  (1<<2); DDRF |=  (1<<2);    // slope: Hi
        _delay_us(300); //wait_us(100);


        // discharge capacitor top:Lo, bottom: Lo
        DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo 
        DDRF |=  (1<<1); PORTF &= ~(1<<1);   // bttom: Lo
        _delay_us(10); //wait_us(100);

/*
        DDRC= 0x00;    PORTC = 0x00;

        DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo 
        DDRF &= ~(1<<1); PORTF &= ~(1<<1);   // bttom: HiZ
        PORTF |=  (1<<2); DDRF |=  (1<<2);    // slope: Hi
        _delay_us(1000); //wait_us(100);

        PORTF |= (1<<0) | (1<<1);
        DDRF |=  (1<<2); PORTF &=  ~(1<<2);   // slope: Lo
*/

        sei();
#if !defined(INTERRUPT_CONTROL_ENDPOINT)
        USB_USBTask();
#endif
    }



/* not swing with full voltage (around 3.0V)
    //Input with pullup
    DDRC = 0x00;
    PORTC = 0xFF;

    for (;;) {
        PINC = 0xFF;
        wait_us(10);
    }
*/
}
