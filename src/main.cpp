#include "ATmega328Compiler.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << "<hex/bin> <input.asm> <output.bin>`\n";
        return 0;
    }

    try {
        ATmega328Compiler compiler(argv[1], argv[2], argv[3]);
        compiler.compile();
        std::cout << "Compilation successful. Output written to " << argv[3] << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}