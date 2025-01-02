#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class ATmega328Compiler {
public:
    ATmega328Compiler(const std::string& cType, const std::string& inputFileName, const std::string& outputFileName);
    void compile();

private:
    std::string compileType;
    std::string inputFileName;
    std::string outputFileName;
    std::vector<std::string> lines;
    std::vector<uint8_t> machineCode;
    std::unordered_map<std::string, uint16_t> opcodeMap;
    std::unordered_map<std::string, size_t> labelMap;
    std::string toHex(uint8_t byte);
    std::string generateHexRecord(uint16_t address, uint8_t recordType, const std::vector<uint8_t>& data);
    
    void writeHexOutput();
    void writeBinOutput();
    void readFile();
    void tokenize();
    void firstPass();
    void secondPass();
    uint16_t parseOperand(const std::string &operand);
    void writeOutput();
    static std::string trim(const std::string& str);
};