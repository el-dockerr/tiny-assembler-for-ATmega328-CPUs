#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class ATmega328Compiler {
public:
    ATmega328Compiler(const std::string& inputFileName, const std::string& outputFileName);
    void compile();

private:
    std::string inputFileName;
    std::string outputFileName;
    std::vector<std::string> lines;
    std::vector<uint8_t> machineCode;
    std::unordered_map<std::string, uint16_t> opcodeMap;
    std::unordered_map<std::string, size_t> labelMap;

    void readFile();
    void tokenize();
    void firstPass();
    void secondPass();
    uint16_t parseOperand(const std::string &operand);
    void writeOutput();
    static std::string trim(const std::string& str);
};