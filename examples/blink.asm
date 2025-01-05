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
    RJMP MAIN           ; Repeat forever

DELAY:
    LDI R18, 82         ; Load outer counter once
    LDI R19, 255        ; Load middle counter once
    LDI R20, 255        ; Load inner counter once
DELAY_LOOP:
    DEC R20             ; Decrement inner (1 cycle)
    BRNE DELAY_LOOP     ; Branch if not zero (2 cycles)
    DEC R19             ; Decrement middle (1 cycle)
    LDI R20, 255        ; Reload inner counter (1 cycle)
    BRNE DELAY_LOOP     ; Branch back to inner loop (2 cycles)
    DEC R18             ; Decrement outer (1 cycle)
    LDI R19, 255        ; Reload middle counter (1 cycle)
    BRNE DELAY_LOOP     ; Branch back to middle loop (2 cycles)
    RET                 ; Return (4 cycles)