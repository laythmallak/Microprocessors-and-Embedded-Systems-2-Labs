; traffic.S
; Low-level GPIO control for LEDs and buzzer
; Arduino Nano (ATmega328P)

; D2 = PD2
; D3 = PD3
; D4 = PD4
; D13 = PB5

.global gpio_init
.global red_on
.global red_off
.global green_on
.global green_off
.global yellow_on
.global yellow_off
.global buzzer_on
.global buzzer_off

; *******************************************
; gpio_init
; Configures LED and buzzer pins as outputs.
; Called once in setup().
; *******************************************
gpio_init:
    ; Set PD2, PD3, PD4 as outputs (Red, Green, Yellow)
    ldi r16, (1<<DDD2)|(1<<DDD3)|(1<<DDD4)
    out DDRD, r16

    ; Set PB5 as output (Buzzer)
    sbi DDRB, DDB5

    ret


; *******************************************
; RED LED
; *******************************************
red_on:
    sbi PORTD, PD2   ; Set PD2 HIGH
    ret

red_off:
    cbi PORTD, PD2   ; Set PD2 LOW
    ret


; *******************************************
; GREEN LED
; *******************************************
green_on:
    sbi PORTD, PD3
    ret

green_off:
    cbi PORTD, PD3
    ret


; *******************************************
; YELLOW LED
; *******************************************
yellow_on:
    sbi PORTD, PD4
    ret

yellow_off:
    cbi PORTD, PD4
    ret


; *******************************************
; BUZZER (D13 / PB5)
; *******************************************
buzzer_on:
    sbi PORTB, PB5
    ret

buzzer_off:
    cbi PORTB, PB5
    ret