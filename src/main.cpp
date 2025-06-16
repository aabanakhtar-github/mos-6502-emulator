#include "mos6502.h"
#include <fstream> 
#include <iostream> 

int main(int argc, char* argv[]) 
{
    Emulator emulator;
    const std::string inputFile = "../testing/assembly/lda_test.bin";
    std::ifstream infile(inputFile, std::ios_base::binary);
    std::vector<char> cbuf{std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>()};

    std::vector<Byte> buf(begin(cbuf), end(cbuf));
    buf.push_back(0xFF - 1);


    for (auto byte : buf) 
    {
        std::cout << std::hex << byte << " ";
    }

    emulator.loadROM(buf);

    emulator.run();
    return 0;    
}