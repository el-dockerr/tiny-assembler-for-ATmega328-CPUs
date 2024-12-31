These are the AVR microcontroller opcodes supported by this release and their basic functions:

### Basic Operations
- `NOP (0x0000)`: **No Operation** - Does nothing for one cycle
- `LDI (0xE000)`: **Load Immediate** - Loads an immediate value into a register
- `ADD (0x0C00)`: **Add** - Adds two registers together
- `SUB (0x1800)`: **Subtract** - Subtracts one register from another

### Memory Operations
- `LD (0x900C)`: **Load** - Loads data from memory into a register
- `ST (0x920C)`: **Store** - Stores data from a register into memory
- `IN (0xB000)`: **Input** - Reads data from an I/O port
- `OUT (0xB800)`: **Output** - Writes data to an I/O port

### Control Flow
- `JMP (0x940C)`: **Jump** - Unconditional jump to an address
- `CALL (0x940E)`: **Call** - Calls a subroutine
- `RET (0x9508)`: **Return** - Returns from a subroutine
- `CP (0x1400)`: **Compare** - Compares two registers

### Branch Operations
- `BRNE (0xF401)`: **Branch if Not Equal** - Branches if the result is not equal
- `BRGE (0xF404)`: **Branch if Greater or Equal** - Branches if greater than or equal
- `BRLT (0xF400)`: **Branch if Less Than** - Branches if less than


