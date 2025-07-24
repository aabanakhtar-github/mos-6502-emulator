#include "mos6502.h"
#include <fstream> 
#include <iostream> 

int main(int argc, char* argv[]) 
{
    Emulator emulator;
    Emulator::testing = false;
    
    std::string inputFile;
    std::cout << "Enter binary file: "; 
    std::cin >> inputFile; 

    /* Create the program*/
    std::ifstream infile(inputFile, std::ios_base::binary);
    if (!infile.is_open())
    {
        std::cout << "Invalid file entered!" << std::endl; 
        return 1; 
    }

    std::vector<char> cbuf{std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>()};
    std::vector<Byte> buf(begin(cbuf), end(cbuf));
    buf.push_back(0x02); // EOP

    std::cout << "=====INITIAL=====\n";
    std::cout << emulator.cpu.to_string() << std::endl;
    emulator.loadROM(buf);
    emulator.run();

    std::cout << "====FINAL=====\n";
    std::cout << emulator.cpu.to_string() << std::endl;
    return 0;    
}