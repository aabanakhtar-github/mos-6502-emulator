#include "mos6502.h"


void Emulator::loadROM(const std::vector<Byte>& program) 
{

}

void Emulator::run()
{
  
  while () 
  {

  }
}

Instruction Emulator::loadNextInstruction()
{
  return Instruction{};
}

MOS_6502::MOS_6502() 
    : program_counter(Memory::ROM_START),
      stack_pointer(Memory::STACK_BASE),
      accumulator(0), 
      X(0), 
      Y(0), 
      P() // random value
{

}

// reference: https://www.pagetable.com/c64ref/6502/?tab=2
/* TODO: add all instructions*/
const Instruction instruction_map[0x100] = {
    /* name, opcode, args_count, cycles, addressing_mode, implementation */
    {"BRK", 0x00, 1, 7, AddressMode::IMPLICIT, nullptr},
    {"ORA", 0x01, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr},
    {"ORA", 0x05, 2, 3, AddressMode::ZERO_PAGE, nullptr},
    {"ORA", 0x06, 2, 5, AddressMode::ZERO_PAGE_AND_X, nullptr},
    {"ORA", 0x09, 2, 2, AddressMode::IMMEDIATE, nullptr},
    {"ORA", 0x0D, 3, 4, AddressMode::ABSOLUTE, nullptr},
    {"ORA", 0x0E, 3, 6, AddressMode::ABSOLUTE_AND_X, nullptr},

    {"LDA", 0xA9, 2, 2, AddressMode::IMMEDIATE, nullptr},
    {"LDA", 0xA5, 2, 3, AddressMode::ZERO_PAGE, nullptr},
    {"LDA", 0xB5, 2, 4, AddressMode::ZERO_PAGE_AND_X, nullptr},
    {"LDA", 0xAD, 3, 4, AddressMode::ABSOLUTE, nullptr},
    {"LDA", 0xBD, 3, 4 /* +1 */, AddressMode::ABSOLUTE_AND_X, nullptr},
    {"LDA", 0xB9, 3, 4 /* +1 */, AddressMode::ABSOLUTE_AND_Y, nullptr},
    {"LDA", 0xA1, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr},
    {"LDA", 0xB1, 2, 5 /* +1 */, AddressMode::INDIRECT_INDEXED, nullptr},

    {"STA", 0x85, 2, 3, AddressMode::ZERO_PAGE, nullptr},
    {"STA", 0x95, 2, 4, AddressMode::ZERO_PAGE_AND_X, nullptr},
    {"STA", 0x8D, 3, 4, AddressMode::ABSOLUTE, nullptr},
    {"STA", 0x9D, 3, 5, AddressMode::ABSOLUTE_AND_X, nullptr},
    {"STA", 0x99, 3, 5, AddressMode::ABSOLUTE_AND_Y, nullptr},
    {"STA", 0x81, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr},
    {"STA", 0x91, 2, 6, AddressMode::INDIRECT_INDEXED, nullptr},

    {"TAX", 0xAA, 1, 2, AddressMode::IMPLICIT, nullptr},
    {"INX", 0xE8, 1, 2, AddressMode::IMPLICIT, nullptr},
    {"JMP", 0x4C, 3, 3, AddressMode::ABSOLUTE, nullptr},
    {"JMP", 0x6C, 3, 5, AddressMode::INDIRECT, nullptr},
    {"NOP", 0xEA, 1, 2, AddressMode::IMPLICIT, nullptr},
};
