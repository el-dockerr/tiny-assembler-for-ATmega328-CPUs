; This code is designed for ATmega328 CPUs and can be compiled wit ATmega328Compiler

; ATmega328 LED Blink - 1 second on/off
; Frequency: 16MHz
; LED: Pin 5 (PORTB)
; Author: Swen Kalski

    LDI R16, 0x20       ; Set bit 5 (0b00100000)
    OUT 0x24, R16       ; DDRB - set PB5 as output
    CLR R17             ; Clear R17 for LED off state

LOOP:
    OUT 0x25, R16       ; PORTB - LED ON
    RCALL DELAY         ; Wait 1 second
    OUT 0x25, R17       ; PORTB - LED OFF
    RCALL DELAY         ; Wait 1 second
    RJMP LOOP           ; Repeat forever

DELAY:                  ; Delay subroutine
    LDI R20, 82        ; Outer loop counter (82 cycles)
D1: LDI R21, 255       ; Middle loop counter
D2: LDI R22, 255       ; Inner loop counter
D3: DEC R22            ; 1 cycle
    BRNE D3            ; 2 cycles if branch taken, 1 if not
    DEC R21            ; 1 cycle
    BRNE D2            ; 2 cycles if branch taken, 1 if not
    DEC R20            ; 1 cycle
    BRNE D1            ; 2 cycles if branch taken, 1 if not
    RET                ; 4 cycles