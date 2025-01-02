#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>

namespace Opcodes {
    const std::unordered_map<std::string, uint16_t> MAP = {
        {"NOP", 0x0000},
        {"LDI", 0xE000},
        {"ADD", 0x0C00},
        {"SUB", 0x1800},
        {"JMP", 0x940C},
        {"OUT", 0xB800},
        {"IN", 0xB000},
        {"CALL", 0x940E},
        {"RET", 0x9508},
        {"LD", 0x900C},
        {"ST", 0x920C},
        {"CP", 0x1400},
        {"BRNE", 0xF401},
        {"BRGE", 0xF404},
        {"BRLT", 0xF400},
        {"DEC", 0x940A},
        {"CLR", 0x2400}
    };
}