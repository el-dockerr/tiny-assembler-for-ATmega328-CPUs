; This code is designed for ATmega328 CPUs and can be compiled wit ATmega328Compiler

; ATmega328 LED Blink - 1 second on/off
; Frequency: 16MHz
; LED: Pin 5 (PORTB)
; Author: Swen Kalski

; Initialize LED pin
     LDI R16, 0x20       ; Set bit 5 (0b00100000)
    OUT 0x24, R16       ; DDRB - configure Pin 5 as output
    CLR R17             ; Clear R17 for LED off state

MAIN:
    OUT 0x25, R16       ; PORTB - LED ON
