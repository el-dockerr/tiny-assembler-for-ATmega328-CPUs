#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <iomanip>
#include <cassert>
#include <cstdint>

class ATmega328Compiler {
public:
    ATmega328Compiler(const std::string& inputFileName, const std::string& outputFileName)
        : inputFileName(inputFileName), outputFileName(outputFileName) {}

    void compile() {
        readFile();
        tokenize();
        firstPass();
        secondPass();
        writeOutput();
    }

private:
    std::string inputFileName;
    std::string outputFileName;
    std::vector<std::string> lines;
    std::vector<uint8_t> machineCode;
    std::unordered_map<std::string, uint16_t> opcodeMap = {
        {"NOP", 0x0000},
        {"LDI", 0xE000},
        {"ADD", 0x0C00},
        {"SUB", 0x1800},
        {"JMP", 0x940C},
        {"OUT", 0xB800},  // Write to I/O port
        {"IN", 0xB000},   // Read from I/O port
        {"CALL", 0x940E}, // Call subroutine
        {"RET", 0x9508},  // Return from subroutine
        {"LD", 0x900C},   // Load from memory
        {"ST", 0x920C},   // Store to memory
        {"CP", 0x1400},   // Compare
        {"BRNE", 0xF401}, // Branch if not equal
        {"BRGE", 0xF404}, // Branch if greater or equal
        {"BRLT", 0xF400}  // Branch if less than
    };
    std::unordered_map<std::string, size_t> labelMap;

    void readFile() {
        std::ifstream file(inputFileName);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open input file.");
        }
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }

    void tokenize() {
        for (auto& line : lines) {
            line = trim(line);
        }
    }

    void firstPass() {
        size_t address = 0;
        for (const auto& line : lines) {
            if (line.empty() || line[0] == ';') {
                continue; // Skip comments and empty lines
            }

            if (line.back() == ':') {
                std::string label = line.substr(0, line.size() - 1);
                labelMap[label] = address;
            } else {
                address += 2; // Each instruction is 2 bytes
            }
        }
    }

    void secondPass() {
        for (const auto& line : lines) {
            if (line.empty() || line[0] == ';' || line.back() == ':') {
                continue; // Skip comments, empty lines, and labels
            }

            std::istringstream iss(line);
            std::string mnemonic;
            iss >> mnemonic;

            auto opcodeIt = opcodeMap.find(mnemonic);
            if (opcodeIt == opcodeMap.end()) {
                throw std::runtime_error("Unknown instruction: " + mnemonic);
            }

            uint16_t opcode = opcodeIt->second;

            std::string operand;
            if (iss >> operand) {
                if (mnemonic == "JMP" && labelMap.find(operand) != labelMap.end()) {
                    opcode |= labelMap[operand] & 0x0FFF; // Encode address in lower 12 bits
                } else {
                    opcode |= parseOperand(operand);
                }
            }

            machineCode.push_back(opcode & 0xFF);
            machineCode.push_back((opcode >> 8) & 0xFF);
        }
    }

    static uint16_t parseOperand(const std::string& operand) {
        if (operand[0] == 'R') {
            return std::stoi(operand.substr(1));
        } else if (operand[0] >= '0' && operand[0] <= '9') {
            return std::stoi(operand);
        }
        throw std::runtime_error("Invalid operand: " + operand);
    }

    void writeOutput() {
        std::ofstream file(outputFileName, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open output file.");
        }
        for (auto byte : machineCode) {
            file.put(static_cast<char>(byte));
        }
        file.close();
    }

    static std::string trim(const std::string& str) {
        const auto strBegin = str.find_first_not_of(" \t");
        if (strBegin == std::string::npos)
            return "";

        const auto strEnd = str.find_last_not_of(" \t");
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }
};

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.bin>\n";
        std::cerr << "       " << argv[0] << " --test\n";
        return 1;
    }

    try {
        ATmega328Compiler compiler(argv[1], argv[2]);
        compiler.compile();
        std::cout << "Compilation successful. Output written to " << argv[2] << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
