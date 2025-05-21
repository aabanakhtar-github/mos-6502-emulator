#include "mos6502.h"

int main(int argc, char* argv[]) 
{
    Emulator emulator;
    std::vector<Byte> rom = {
        0xA9, 0xFF,  //LDA 0xFF
        0xAA, // TAX 
        0xE8,
        0xE8,
        0xE8,
        0xFE
    };
    rom.push_back(0xFF - 1);

    emulator.loadROM(rom);

    emulator.run();
    return 0;    
}