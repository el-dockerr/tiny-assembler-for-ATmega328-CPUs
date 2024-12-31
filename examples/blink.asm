; This code is designed for ATmega328 CPUs and can be compiled wit ATmega328Compiler

; Define the LED pin and delay
LDI R16, 0x20       ; Set R16 to 0x20 (Pin 5, PORTB)
OUT 0x24, R16       ; Set DDRB to configure Pin 5 as output

LOOP:
    OUT 0x25, R16   ; Turn LED on (PORTB)
    CALL DELAY      ; Call delay subroutine
    OUT 0x25, R0    ; Turn LED off (PORTB)
    CALL DELAY      ; Call delay subroutine
    JMP LOOP        ; Repeat the loop

DELAY:
    LDI R18, 0xFF   ; Outer loop counter
    LDI R19, 0xFF   ; Inner loop counter
DELAY_LOOP:
    DEC R19         ; Decrement inner loop counter
    BRNE DELAY_LOOP ; Branch if not zero
    DEC R18         ; Decrement outer loop counter
    BRNE DELAY_LOOP ; Branch if not zero
    RET             ; Return from subroutine