 ;This code is designed for ATmega328 CPUs and can be compiled wit ATmega328Compiler

; Minimal LED test for ATmega328
; LED: Pin 5 (PORTB)
; Test program counter and execution flow
; Author: Swen Kalski

; Minimal LED Test With Infinite Loop

START:
    LDI R16, 0x20       ; Set bit 5 (0b00100000)
    OUT 0x24, R16       ; DDRB - configure Pin 5 as output

LOOP:
    OUT 0x25, R16       ; PORTB - LED ON
    CLR R17             ; Clear R17
    OUT 0x25, R17       ; PORTB - LED OFF
    RJMP LOOP           ; Jump back to LOOP