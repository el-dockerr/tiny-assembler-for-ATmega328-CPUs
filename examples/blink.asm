; This code is designed for ATmega328 CPUs and can be compiled wit ATmega328Compiler

; ATmega328 LED Blink - 1 second on/off
; Frequency: 16MHz
; LED: Pin 5 (PORTB)
; Author: Swen Kalski

; Initialize LED pin
    LDI R16, 0x20       ; Set bit 5 (0b00100000)
    OUT 0x24, R16       ; DDRB - configure Pin 5 as output
    CLR R0              ; Clear R0 for LED off state

MAIN_LOOP:
    OUT 0x25, R16       ; PORTB - LED ON
    CALL DELAY_1SEC     ; Wait 1 second
    OUT 0x25, R0        ; PORTB - LED OFF
    CALL DELAY_1SEC     ; Wait 1 second
    JMP MAIN_LOOP       ; Repeat forever

DELAY_1SEC:
    ; At 16MHz, need 16,000,000 cycles for 1 second
    ; Each inner loop takes 4 cycles
    ; Each outer loop takes 256 * 4 = 1024 cycles
    ; Need ~15,625 outer loops for 1 second
    LDI R18, 0x3D       ; Load 61 (0x3D) for outer loop
    LDI R19, 0xFF       ; Load 255 for inner loop

DELAY_OUTER:
    LDI R19, 0xFF       ; Reset inner counter
DELAY_INNER:
    NOP                 ; 1 cycle padding
    DEC R19             ; Decrement inner counter
    BRNE DELAY_INNER    ; Branch if not zero
    DEC R18             ; Decrement outer counter
    BRNE DELAY_OUTER    ; Branch if not zero
    RET                 ; Return from subroutine