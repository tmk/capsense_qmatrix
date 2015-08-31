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

    // burst signal PC0-7
    DDRC = 0xFF;
    PORTC = 0x00;

    // top: PF0
    // bottom: PF1
    // slope: PF2
    DDRF = 0x00;
    PORTF = 0x00;

    for (;;) {
        cli();
        DDRC= 0xFF;
        PORTC = 0x00;
        DDRF = 0x00;
        PORTF = 0x00;

        // Burst
        // lenght: 16-64
        for (int i = 0; i < 32; i++) {
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

        // Analog Comparator setup
        // input+: AIN0 = GND
        // input-: Multiplexer = bottom
        ADMUX |= MUX1;  // bottom - Multiplexer select
        ADCSRA &= ~(1<<ADEN);   // ADC disable
        ADCSRB |= (1<<ACME);    // Analog Comparator Multiplexer Enable
        //ACSR |= (1<<ACIC);      // Analog Comparator Input Capture Enable

        // sense
        DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo 
        DDRF &= ~(1<<1); PORTF &= ~(1<<1);   // bttom: HiZ
        PORTF |=  (1<<2); DDRF |=  (1<<2);    // slope: Hi

        // Analog Comparator Output
        uint16_t count = 0;
        while (ACSR & (1<<ACO)) {
            count++;
        }


        // discharge capacitor top:Lo, bottom: Lo
        DDRF |=  (1<<0); PORTF &= ~(1<<0);   // top: Lo 
        DDRF |=  (1<<1); PORTF &= ~(1<<1);   // bttom: Lo
        _delay_us(10); //wait_us(100);


        sei();
#if !defined(INTERRUPT_CONTROL_ENDPOINT)
        USB_USBTask();
#endif
        xprintf("\r%04X", count);
        _delay_ms(10);
    }
}
