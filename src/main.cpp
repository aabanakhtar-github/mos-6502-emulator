#include "mos6502.h"

int main(int argc, char* argv[]) 
{
    Emulator emulator;
    std::vector<Byte> rom = {
        0xEA,
        0xEA,
        0x01,
        
        0xFF-1
    };
    rom.push_back(0xFF - 1);

    emulator.loadROM(rom);

    emulator.run();
    return 0;    
}