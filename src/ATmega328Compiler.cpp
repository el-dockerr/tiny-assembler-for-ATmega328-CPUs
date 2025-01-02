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

ATmega328Compiler::ATmega328Compiler(const std::string& inputFileName, const std::string& outputFileName)
    : inputFileName(inputFileName)
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
    for (const auto& line : lines) {
        if (line.empty() || line[0] == ';' || line.back() == ':') {
            continue;
        }
        std::istringstream iss(line);
        std::string mnemonic;
        iss >> mnemonic;
        
        auto it = opcodeMap.find(mnemonic);
        if (it == opcodeMap.end()) {
            throw std::runtime_error("Unknown instruction: " + mnemonic);
        }
        
        uint16_t opcode = it->second;
        machineCode.push_back(opcode & 0xFF);
        machineCode.push_back((opcode >> 8) & 0xFF);
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
    std::ofstream file(outputFileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFileName);
    }

    // Write program data
    for (size_t i = 0; i < machineCode.size(); i += 16) {
        std::vector<uint8_t> record;
        size_t remaining = std::min(size_t(16), machineCode.size() - i);
        record.insert(record.end(), machineCode.begin() + i, machineCode.begin() + i + remaining);
        file << generateHexRecord(i, 0x00, record) << "\n";
    }

    // Write EOF record
    file << ":00000001FF\n";
    file.close();
}