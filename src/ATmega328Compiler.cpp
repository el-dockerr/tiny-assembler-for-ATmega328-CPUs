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
    
    for (const auto& line : lines) {
        if (line.empty() || line[0] == ';') {
            continue;
        }
        
        // Store label addresses at current program counter
        if (line.back() == ':') {
            std::string label = line.substr(0, line.size() - 1);
            if (labelMap.find(label) != labelMap.end()) {
                throw std::runtime_error("Duplicate label: " + label);
            }
            labelMap[label] = programCounter;
            continue;
        }
        
        // Parse instruction and update program counter
        std::istringstream iss(line);
        std::string mnemonic;
        iss >> mnemonic;
        
        if (mnemonic.empty()) {
            continue;
        }
        
        // Handle special cases for 32-bit instructions
        if (mnemonic == "JMP" || mnemonic == "CALL") {
            programCounter += 4;  // 32-bit instructions
        }
        else if (opcodeMap.find(mnemonic) != opcodeMap.end()) {
            programCounter += 2;  // Standard 16-bit instructions
        }
        else {
            throw std::runtime_error("Unknown instruction: " + mnemonic);
        }
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
            int regNum = std::stoi(reg.substr(1));  // Remove 'R' prefix
            int immValue = std::stoi(value, nullptr, 0);  // Auto-detect base
            opcode = 0xE000 | ((regNum - 16) & 0xF) << 4 | ((immValue & 0xF0) << 4) | (immValue & 0x0F);
        }
        else if (mnemonic == "OUT") {
            // Format: OUT A,Rr (1011 1AAr rrrr AAAA)
            std::string port, reg;
            iss >> port >> reg;
            int portAddr = std::stoi(port, nullptr, 0);
            int regNum = std::stoi(reg.substr(1));
            opcode = 0xB800 | ((portAddr & 0x30) << 5) | (regNum & 0x1F) | (portAddr & 0x0F);
        }else if (mnemonic == "JMP") {
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
        }
        else if (mnemonic == "RJMP") {
            // Format: RJMP k (1100 kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            
            // Calculate relative offset (-2K to +2K words)
            int16_t offset = it->second - address - 1;
            if (offset < -2048 || offset > 2047) {
                throw std::runtime_error("RJMP offset out of range");
            }
            
            // Encode offset in instruction
            opcode = 0xC000 | (offset & 0x0FFF);
        }
        else if (mnemonic == "RCALL") {
            // Format: RCALL k (1101 kkkk kkkk kkkk)
            std::string label;
            iss >> label;
            auto it = labelMap.find(label);
            if (it == labelMap.end()) {
                throw std::runtime_error("Unknown label: " + label);
            }
            int16_t offset = (it->second - address - 1) & 0x0FFF;
            opcode = 0xD000 | offset;
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
            int rdNum = std::stoi(rd.substr(1));
            int rrNum = std::stoi(rr.substr(1));
            opcode = 0x0C00 | (rdNum << 4) | (rrNum & 0x0F);
        }
        else if (mnemonic == "SUB") {
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
        }
        else if (mnemonic == "CP") {
            // Format: CP Rd,Rr (0001 01rd dddd rrrr)
            std::string rd, rr;
            iss >> rd >> rr;
            int rdNum = std::stoi(rd.substr(1));
            int rrNum = std::stoi(rr.substr(1));
            opcode = 0x1400 | (rdNum << 4) | (rrNum & 0x0F);
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
        } else if (mnemonic == "DEC") {
            // Format: DEC Rd (1001 010d dddd 1010)
            std::string reg;
            iss >> reg;
            int regNum = std::stoi(reg.substr(1));
            opcode = 0x940A | (regNum << 4);
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