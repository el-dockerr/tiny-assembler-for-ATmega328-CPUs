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
    size_t address = 0;
    for (const auto& line : lines) {
        if (line.empty() || line[0] == ';') {
            continue;
        }
        if (line.back() == ':') {
            std::string label = line.substr(0, line.size() - 1);
            labelMap[label] = address;
        } else {
            address += 2;
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
        else {
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
        std::ofstream file(outputFileName);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open output file: " + outputFileName);
        }
        for (size_t i = 0; i < machineCode.size(); i += 16) {
            std::vector<uint8_t> record;
            size_t remaining = std::min(size_t(16), machineCode.size() - i);
            record.insert(record.end(), machineCode.begin() + i, machineCode.begin() + i + remaining);
            file << generateHexRecord(i, 0x00, record) << "\n";
        }

        // Write EOF record
        file << ":00000001FF\n";
        file.close();
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