#include "ATmega328Compiler.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.bin>`\n";
        return 0;
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