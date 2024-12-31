#include "ATmega328Compiler.hpp"
#include "OpcodeMap.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

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

void ATmega328Compiler::writeOutput() {
    std::ofstream file(outputFileName, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFileName);
    }
    file.write(reinterpret_cast<const char*>(machineCode.data()), machineCode.size());
    file.close();
}