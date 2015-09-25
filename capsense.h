#ifndef CAPSENSE_H
#define CAPSENSE_H

#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "util/print.h"
#include "util/lufa.h"


#define ABS(a, b)   ((a > b) ? (a - b) : (b - a))
#define MATRIX_X 16
#define MATRIX_Y 8
#define THRESHOLD_ON    0x50
#define HYSTERESIS      0x20

uint8_t avg[MATRIX_X][MATRIX_Y];

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




void burst(uint8_t x, uint8_t ymask);
uint16_t sense(uint8_t y);
void discharge_all(void);
void calibrate(void);

#endif
