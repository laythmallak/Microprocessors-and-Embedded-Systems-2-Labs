#define __SFR_OFFSET 0x00
#include "avr/io.h"

.global gpio_init
.global red_on
.global red_off
.global green_on
.global green_off
.global yellow_on
.global yellow_off
.global buzzer_on
.global buzzer_off

.section .text

gpio_init:
  SBI DDRD, 2      ; D2 red
  SBI DDRD, 3      ; D3 green
  SBI DDRD, 4      ; D4 yellow
  SBI DDRB, 5      ; D13 buzzer
  RET

red_on:    SBI PORTD, 2 ; D2
           RET
red_off:   CBI PORTD, 2
           RET

green_on:  SBI PORTD, 3 ; D3
           RET
green_off: CBI PORTD, 3
           RET

yellow_on: SBI PORTD, 4 ; D4
           RET
yellow_off:CBI PORTD, 4
           RET

buzzer_on: SBI PORTB, 5 ; D13
           RET
buzzer_off:CBI PORTB, 5
           RET