// ATmega328Compiler.cpp
// Author: Swen "El Dockerr" Kalski <swen.kalski@camaleao-studio.com>

#include "ATmega328Compiler.hpp"
#include "OpcodeMap.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

ATmega328Compiler::ATmega328Compiler(const std::string& cType,  const std::string& inputFileName, const std::string& outputFileName)
    : compileType(cType)
    ,inputFileName(inputFileName)
    , outputFileName(outputFileName)
    , opcodeMap(Opcodes::MAP) {
}

void ATmega328Compiler::compile() {
    readFile();
    tokenize();
    firstPass();
    secondPass();
    writeOutput();
}

void ATmega328Compiler::readFile() {
    std::ifstream file(inputFileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open input file: " + inputFileName);
    }
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
}

std::string ATmega328Compiler::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

void ATmega328Compiler::tokenize() {
    for (auto& line : lines) {
        line = trim(line);
    }
}

void ATmega328Compiler::firstPass() {
    uint16_t programCounter = 0;

    // First pass: collect all label addresses
    for (const auto& line : lines) {
        if (line.empty() || line[0] == ';') {
            continue;
        }

        // Store label position
        if (line.back() == ':') {
            std::string label = line.substr(0, line.size() - 1);
            if (labelMap.find(label) != labelMap.end()) {
                throw std::runtime_error("Duplicate label: " + label);
            }
            labelMap[label] = programCounter;
            std::cout << "Label " << label << " at address: " << programCounter << std::endl;
            continue;
        }

        // Parse instruction and update program counter
        std::istringstream iss(line);
        std::string mnemonic;
        iss >> mnemonic;

        if (mnemonic.empty()) {
            continue;
        }

        // Determine instruction size
        if (mnemonic == "JMP" || mnemonic == "CALL") {
            programCounter += 4;  // 32-bit instructions
        }
        else if (opcodeMap.find(mnemonic) != opcodeMap.end()) {
            programCounter += 2;  // 16-bit instructions
        }
        else {
            throw std::runtime_error("Unknown instruction: " + mnemonic);
        }

        // Debug output
        std::cout << "Instruction " << mnemonic << " at address: " << (programCounter - ((mnemonic == "JMP" || mnemonic == "CALL") ? 4 : 2)) << std::endl;
    }

    // Validate addresses
    if (programCounter > 0x8000) {
        throw std::runtime_error("Program too large");
    }
}

void ATmega328Compiler::secondPass() {
    uint16_t address = 0;
    for (const auto& line : lines) {
        if (line.empty() || line[0] == ';' || line.back() == ':') {
            continue;
        }

        std::istringstream iss(line);
        std::string mnemonic;
        iss >> mnemonic;

        uint16_t opcode = 0;
        if (mnemonic == "LDI") {
            // Format: LDI Rd,K (1110 KKKK dddd KKKK)
            std::string reg, value;
            iss >> reg >> value;

            reg = trimOperand(reg);
            value = trimOperand(value);

            int regNum = parseRegister(reg);  // Use parseRegister
            if (regNum < 16 || regNum > 31) {
                throw std::runtime_error("LDI can only be used with R16 to R31. Found: " + reg);
            }
            int immValue = std::stoi(value, nullptr, 0);  // Auto-detect base
            if (immValue < 0 || immValue > 0xFF) {
                throw std::runtime_error("Immediate value out of range for LDI: " + value);
            }
            opcode = 0xE000 | ((immValue & 0xF0)) | ((regNum - 16) << 4) | (immValue & 0x0F);
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "LDI " << reg << ", " << value << " encoded at address: " << (address - 2) << " as 0x" 
                      << std::hex << opcode << std::dec << std::endl;
            continue;
        } else if (mnemonic == "OUT") {
            // Format: OUT A,Rr (1011 1AAr rrrr AAAA)
            std::string port, reg;
            iss >> port >> reg;

            port = trimOperand(port);
            reg = trimOperand(reg);

            int portAddr = std::stoi(port, nullptr, 0);
            int regNum = parseRegister(reg);  // Use parseRegister
            if (regNum < 0 || regNum > 31) {
                throw std::runtime_error("Invalid register number for OUT: " + reg);
            }
            opcode = 0xB800 | ((portAddr & 0x30) << 5) | ((regNum & 0x1F) << 4) | (portAddr & 0x0F);
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "OUT " << port << ", " << reg << " encoded at address: " << (address - 2) 
                      << " as 0x" << std::hex << opcode << std::dec << std::endl;
            continue;
        }
        else if (mnemonic == "CLR") {
            // Format: CLR Rd (EOR Rd, Rd) -> 0x9400 | (Rd << 4) | Rd
            std::string rd;
            iss >> rd;
            
            // Trim trailing commas (if any)
            rd = trimOperand(rd);
            
            int rdNum = parseRegister(rd);  // Use parseRegister
            opcode = 0x9400 | (rdNum << 4) | rdNum;
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "CLR " << rd << " encoded as 0x" 
                      << std::hex << opcode << std::dec 
                      << " at address: " << (address - 2) << std::endl;
            continue;
        } else if (mnemonic == "RET") {
            // Format: RET (1001 0101 0000 1000)
            opcode = 0x9508;
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "RET encoded at address: " << (address - 2) 
                      << " as 0x" << std::hex << opcode << std::dec << std::endl;
            continue;
        } else if (mnemonic == "JMP") {
            // Format: JMP k (1001 010k kkkk 110k kkkk kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            uint32_t dest = it->second;
            // JMP is 32-bit instruction
            opcode = 0x940C | ((dest & 0x1FFFF) << 3);
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            machineCode.push_back((dest >> 16) & 0xFF);
            machineCode.push_back((dest >> 24) & 0xFF);
            continue;  // Skip normal opcode output
        } else if (mnemonic == "RJMP") {
            // Format: RJMP k (1100 kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }

            // Calculate relative offset in words
            int16_t offset = ((it->second - (address + 2)) / 2); // RJMP offset is relative word distance
            if (offset < -2048 || offset > 2047) {
                throw std::runtime_error("RJMP offset out of range for label: " + label);
            }

            opcode = 0xC000 | (offset & 0x0FFF);
            continue;
        }
        else if (mnemonic == "RCALL") {
            // Format: RCALL k (1101 kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }

            // Calculate relative offset in words
            int16_t offset = ((it->second - (address + 2)) / 2); // RCALL offset is relative word distance
            if (offset < -2048 || offset > 2047) {
                throw std::runtime_error("RCALL offset out of range for label: " + label);
            }

            opcode = 0xD000 | (offset & 0x0FFF);
            continue;
        }
        else if (mnemonic == "CALL") {
            // Format: CALL k (1001 010k kkkk 111k kkkk kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            uint32_t dest = it->second;
            // CALL is 32-bit instruction
            opcode = 0x940E | ((dest & 0x1FFFF) << 3);
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            machineCode.push_back((dest >> 16) & 0xFF);
            machineCode.push_back((dest >> 24) & 0xFF);
            continue;  // Skip normal opcode output
        } else if (mnemonic == "ADD") {
            // Format: ADD Rd,Rr (0000 11rd dddd rrrr)
            std::string rd, rr;
            iss >> rd >> rr;

            rr = trimOperand(rr);
            rd = trimOperand(rd);

            int rdNum = parseRegister(rd);  // Use parseRegister
            int rrNum = parseRegister(rr);  // Use parseRegister
            opcode = 0x0C00 | (rdNum << 4) | rrNum;
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "ADD " << rd << ", " << rr << " encoded at address: " << (address - 2) 
                    << " as 0x" << std::hex << opcode << std::dec << std::endl;
            continue;
        } else if (mnemonic == "SUB") {
            // Format: SUB Rd,Rr (0001 10rd dddd rrrr)
            std::string rd, rr;
            iss >> rd >> rr;
            int rdNum = std::stoi(rd.substr(1));
            int rrNum = std::stoi(rr.substr(1));
            opcode = 0x1800 | (rdNum << 4) | (rrNum & 0x0F);
        }
        else if (mnemonic == "IN") {
            // Format: IN Rd,A (1011 0AAd dddd AAAA)
            std::string rd, port;
            iss >> rd >> port;
            int rdNum = std::stoi(rd.substr(1));
            int portAddr = std::stoi(port, nullptr, 0);
            opcode = 0xB000 | ((portAddr & 0x30) << 5) | (rdNum << 4) | (portAddr & 0x0F);
        } 
        else if (mnemonic == "RET") {
            // Format: RET (1001 0101 0000 1000)
            opcode = 0x9508;
            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
            address += 2;
            std::cout << "RET encoded at address: " << (address - 2) 
                      << " as 0x" << std::hex << opcode << std::dec << std::endl;
        } else if (mnemonic == "CP") {
            // Format: CP Rd,Rr (0001 01rd dddd rrrr)
            std::string rd, rr;
            iss >> rd >> rr;
            int rdNum = std::stoi(rd.substr(1));
            int rrNum = std::stoi(rr.substr(1));
            opcode = 0x1400 | (rdNum << 4) | (rrNum & 0x0F);
            continue;
        }
        else if (mnemonic == "BRNE") {
            // Format: BRNE k (1111 01kk kkkk k001)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            int8_t offset = (it->second - address - 1) & 0x7F;
            opcode = 0xF401 | (offset << 3);
            continue;
        }
        else if (mnemonic == "BRGE") {
            // Format: BRGE k (1111 01kk kkkk k100)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            int8_t offset = (it->second - address - 1) & 0x7F;
            opcode = 0xF404 | (offset << 3);
            continue;
        }
        else if (mnemonic == "BRLT") {
            // Format: BRLT k (1111 00kk kkkk k100)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            int8_t offset = (it->second - address - 1) & 0x7F;
            opcode = 0xF400 | (offset << 3);
            continue;
        } else if (mnemonic == "DEC") {
            // Format: DEC Rd (1001 010d dddd 1010)
            std::string reg;
            iss >> reg;
            int regNum = std::stoi(reg.substr(1));
            opcode = 0x940A | (regNum << 4);
            continue;
        } else {
            auto it = opcodeMap.find(mnemonic);
            if (it == opcodeMap.end()) {
                throw std::runtime_error("Unknown instruction: " + mnemonic);
            }
            opcode = it->second;
        }

        machineCode.push_back(opcode & 0xFF);
        machineCode.push_back((opcode >> 8) & 0xFF);
        address += 2;
    }
}

std::string ATmega328Compiler::trimOperand(const std::string& operand) {
    size_t end = operand.find_last_not_of(", \t\n\r");
    if (end == std::string::npos) {
        return "";
    }
    return operand.substr(0, end + 1);
}

int ATmega328Compiler::parseRegister(const std::string& reg) {
    if (reg.empty() || (reg[0] != 'R' && reg[0] != 'r')) {
        throw std::runtime_error("Invalid register format: " + reg);
    }
    
    // Extract the register number substring (supports R0-R31)
    std::string regNumStr = reg.substr(1);
    
    // Ensure that the remaining characters are digits
    for (char c : regNumStr) {
        if (!isdigit(c)) {
            throw std::runtime_error("Non-digit character in register: " + reg);
        }
    }
    
    int regNum = std::stoi(regNumStr);
    
    if (regNum < 0 || regNum > 31) {
        throw std::runtime_error("Register number out of range (R0-R31): " + reg);
    }
    
    return regNum;
}

std::string ATmega328Compiler::toHex(uint8_t byte) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return ss.str();
}

std::string ATmega328Compiler::generateHexRecord(uint16_t address, uint8_t recordType, const std::vector<uint8_t>& data) {
    std::stringstream ss;
    uint8_t checksum = 0;
    
    // Start code
    ss << ':';
    
    // Byte count
    ss << toHex(data.size());
    checksum += data.size();
    
    // Address
    ss << toHex((address >> 8) & 0xFF) << toHex(address & 0xFF);
    checksum += (address >> 8) & 0xFF;
    checksum += address & 0xFF;
    
    // Record type
    ss << toHex(recordType);
    checksum += recordType;
    
    // Data
    for (uint8_t byte : data) {
        ss << toHex(byte);
        checksum += byte;
    }
    
    // Checksum
    checksum = (~checksum + 1) & 0xFF;
    ss << toHex(checksum);
    
    return ss.str();
}

void ATmega328Compiler::writeOutput() {
    if(compileType == "hex") {
        writeHexOutput();
    } else if (compileType == "bin") {
        std::ofstream file(outputFileName, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open output file: " + outputFileName);
        }
        file.write(reinterpret_cast<const char*>(machineCode.data()), machineCode.size());
        file.close();
    } else {
        throw std::runtime_error("Unknown output format: " + compileType);
    }
}

void ATmega328Compiler::writeHexOutput() {
    std::ofstream file(outputFileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFileName);
    }

    // Write program data in Intel HEX format
    for (size_t i = 0; i < machineCode.size(); i += 16) {
        // Calculate remaining bytes in this record
        size_t count = std::min(size_t(16), machineCode.size() - i);
        
        // Calculate record checksum
        uint8_t checksum = count;  // Start with byte count
        checksum += (i >> 8) & 0xFF;  // Add address high byte
        checksum += i & 0xFF;         // Add address low byte
        
        // Write record start, length, address and type
        file << ":" << std::hex << std::setfill('0') << std::setw(2) << count;
        file << std::hex << std::setfill('0') << std::setw(4) << i;
        file << "00";  // Record type 00 = data
        
        // Write data bytes
        for (size_t j = 0; j < count; j++) {
            uint8_t byte = machineCode[i + j];
            file << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
            checksum += byte;
        }
        
        // Write checksum (two's complement of sum)
        checksum = (~checksum + 1) & 0xFF;
        file << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(checksum) << "\n";
    }
    
    // Write end-of-file record
    file << ":00000001FF\n";
    file.close();
}

void ATmega328Compiler::writeBinOutput() {
    std::ofstream file(outputFileName, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFileName);
    }
    file.write(reinterpret_cast<const char*>(machineCode.data()), machineCode.size());
    file.close();
}