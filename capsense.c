#include <avr/interrupt.h>
#include "capsense.h"


/* Charge sample capacitors with burst of pulse on X line
 * all capacitors on same Y line are charged at once
 */
void burst(uint8_t x, uint8_t ymask)
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


uint16_t sense(uint8_t y)
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


void discharge_all(void)
{
    top_lo_mask(0xFF);
    bottom_lo_mask(0xFF);
    slope_lo();
    //_delay_us(10);
}
