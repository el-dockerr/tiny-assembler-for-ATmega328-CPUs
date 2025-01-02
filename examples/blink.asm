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
    RCALL DELAY         ; Wait 1 second
    OUT 0x25, R17       ; PORTB - LED OFF
    RCALL DELAY         ; Wait 1 second
    RJMP MAIN          ; Repeat forever

DELAY:
    LDI R18, 82        ; Outer loop counter
OUTER:
    LDI R19, 255       ; Middle loop counter
MIDDLE:
    LDI R20, 255       ; Inner loop counter
INNER:
    DEC R20            ; 1 cycle
    BRNE INNER         ; 2 cycles (branch) or 1 cycle (no branch)
    DEC R19            ; 1 cycle
    BRNE MIDDLE        ; 2 cycles (branch) or 1 cycle (no branch)
    DEC R18            ; 1 cycle
    BRNE OUTER         ; 2 cycles (branch) or 1 cycle (no branch)
    RET                ; Return from subroutine