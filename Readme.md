# ピコ (pico)
### ATmega328 CPU用小型アセンブラ 
### *The tiny assembler for ATmega328 CPUs*

私は英語を話さない日本人の仲間のためにものを作ろうとしているので、後で納得のいく翻訳を提供するよう努力するので、我慢してほしい。

## You've been warned
This is work-in-progress. I expect that it will not work as expected yet so be careful. I will do some more code and other important stuff here.

**Bucketlist**
* add more oppcodes (see docs/Opcodes.md what is actually done)
* add more examples
* add some more documentation
* Make assembly great again! (*The only MAGA that makes sense! Don't trust other MAGA's!!! Belive me, that's the best MAGA!!! You can ask other people - this is the best MAGA!!!*)
* implement more oppcodes that it might be possible to use the compilier right on the board and the compiler can compile himself right on the board. (Guess what's possible then!!)


## Why Program Arduino Boards Directly in Assembly?
*By El Dockerr*

When it comes to programming Arduino boards, most developers default to high-level languages like C or Python. They’re familiar, convenient, and come with extensive libraries that simplify even the most complex tasks. But there’s another side to programming these boards—a raw, untamed frontier for those who seek to master their craft: assembly language.

## The Bare-Metal Appeal
Programming directly in assembly isn’t just about nostalgia or geek cred (though it has plenty of both). It’s about control. When you write in assembly, you’re speaking the microcontroller’s native tongue, issuing precise instructions that run exactly as you wrote them. There’s no compiler optimizations that you didn’t authorize, no unnecessary overhead from bloated libraries—just you, the ATmega328’s instruction set, and the silicon.

For tasks that demand the utmost efficiency, such as time-critical operations or low-power designs, assembly can achieve things that high-level languages simply can’t. It also deepens your understanding of how microcontrollers work at the lowest level, making you a better programmer in any language.

## The Challenges
Of course, assembly isn’t for the faint of heart. Forget human-readable function names or helpful error messages. You’ll trade them for mnemonics like LDI, BRNE, and OUT, each corresponding to specific hardware operations. The debugging process is equally hands-on, often involving hexadecimal dumps and intimate knowledge of the ATmega328’s architecture.

Writing a simple program, like blinking an LED, takes more effort in assembly than in C or Python. But in exchange, you learn how to manipulate registers, optimize memory, and control hardware directly.

## Why Bother?
* Performance: Your code is lean and mean, free of unnecessary abstractions.
Learning: You gain a deep understanding of your hardware, which benefits you in higher-level programming as well.
* Control: Every byte and every cycle is in your hands.
* Passion: There’s a unique satisfaction in building something from scratch, knowing you’ve orchestrated every detail.

## Is It Worth It?
If your goal is to get things working as quickly as possible, assembly is overkill. But if you love the challenge, the purity, and the control it offers, there’s nothing quite like it. Writing assembly for an Arduino board like the Uno is like stepping into the cockpit of a classic airplane—you feel every bump and control every lever.


So, if you’re ready to ditch the training wheels and explore what’s really under the hood, welcome to the world of assembly language. Let’s make the ATmega328 our playground.

## How it Works:

**Input:** The program reads an assembly source file (input.asm).

**Tokenization:** It removes comments and whitespace.

**Parsing and Code Generation:** Each line is parsed, and the corresponding machine code is generated using a predefined opcode map.

**Output:** The machine code is saved to a binary file (output.bin).

Example Assembly File (input.asm):

```
; Example program for ATmega328
LDI R16, 0xFF
ADD R17, R16
NOP
```

Compilation Command:
```
./compiler bin input.asm output.bin
```

## Examples

Example Assembly Code with Labels:
```
START:
    LDI R16, 0xFF
    ADD R17, R16
    JMP START
```
This will create a loop that continually adds R16 to R17.

**Here’s a step-by-step guide to create an assembly program that blinks an LED using your compiler for the ATmega328.**

###Assembly Code: LED Blink###
Save the following code in a file named blink.asm:

```
START:
    LDI R16, 0x20       ; Set bit 5 (0b00100000)
    OUT 0x24, R16       ; DDRB - configure Pin 5 as output

LOOP:
    OUT 0x25, R16       ; PORTB - LED ON
    CLR R17             ; Clear R17
    OUT 0x25, R17       ; PORTB - LED OFF
    RJMP LOOP           ; Jump back to LOOP
```

***Steps to Assemble and Run the Code***

Compile the Assembly Code: Use the compiler you've written to convert blink.asm into a binary file.

```
./compiler hex blink.asm blink.hex
```

Upload to Arduino:

* Install AVRDUDE if not already installed.
* Connect your Arduino to your computer via USB.
* Use the following command to upload the binary file:

```
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:blink.hex

>> avrdude: AVR device initialized and ready to accept instructions
>> avrdude: device signature = 0x1e950f (probably m328p)
>> avrdude: Note: flash memory has been specified, an erase cycle will be performed.
>>          To disable this feature, specify the -D option.
>> avrdude: erasing chip
>> avrdude: reading input file blink.hex for flash
>>          with 22 bytes in 1 section within [0, 0x15]
>>          using 1 page and 106 pad bytes
>> avrdude: writing 22 bytes flash ...
>> 
>> Writing | ################################################## | 100% 0.04 s
>> 
>> avrdude: 22 bytes of flash written
>> avrdude: verifying flash memory against blink.hex
>> 
>> Reading | ################################################## | 100% 0.02 s
>> 
>> avrdude: 22 bytes of flash verified
>> 
>> avrdude done.  Thank you.
```
***Info:** *Replace /dev/ttyUSB0 with the correct port on your system.*

**Verify the Setup:**

* Ensure the Arduino is powered.
* An LED connected to pin 5 (PORTB) should start blinking.
* Explanation of the Code

**Initialization:**

* LDI R16, 0x20: Sets the register R16 to configure PORTB Pin 5.
* OUT 0x24, R16: Writes to DDRB (Data Direction Register B) to set Pin 5 as output.

**Main Loop:**

* **OUT 0x25, R16** Turns the LED on by writing to PORTB.
* **CALL DELAY** Waits for a short period using the delay subroutine.
* **OUT 0x25, R0** Turns the LED off.

**Delay Subroutine:**

A nested loop creates a delay to make the blinking visible.

**Hardware Setup**
* Connect an LED to Pin 5 (PORTB) on the Arduino Uno.
* The longer leg (anode) of the LED goes to Pin 5.
* The shorter leg (cathode) connects to ground through a 220-ohm resistor.
* Power the Arduino and observe the blinking LED.


# Compile and Test

Compile the Program:

```
g++ -o atmega328_compiler atmega328_compiler.cpp
```

Regular Compilation: To compile an assembly file to machine code, use:

```
./atmega328_compiler <hex/bin> <input.asm> <output.bin>
```

