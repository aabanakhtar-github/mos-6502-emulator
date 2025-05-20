#include "mos6502.h"
#include <iostream>
#include <cstring>
#include "util.h"

using namespace std::placeholders;

void Emulator::loadROM(const std::vector<Byte>& program) 
{
  // define our instructions
  initInstructionMap();
  if (Memory::ROM_END - Memory::ROM_START < program.size()) 
  {
    std::cerr << "Cannot install ROM, too big." << std::endl;
  }

  std::memcpy(mem.memory + Memory::ROM_START, program.data(), program.size());
  std::cout << "Loaded ROM successfully." << std::endl;
}

void Emulator::run()
{
  while (cpu.program_counter < Memory::ROM_END) 
  {
    int opcode = mem.readByte(cpu.program_counter);
    // special opcode for terminating the program (added by emulator) 
    if (opcode == 0xFF - 1)
    {
      break;
    }

    auto instruction = instruction_map[opcode];
    instruction.implementation(opcode);
    cpu.program_counter++;
  }
}

void Emulator::NOP(int opcode)
{
  for (int i = 0; i < instruction_map[opcode].cycles; ++i) 
  {
    delayMicros(CLOCK_uS);
  }
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
void Emulator::initInstructionMap()
{
  instruction_map[0x00] = {"BRK", 0x00, 1, 7, AddressMode::IMPLICIT, nullptr};

  instruction_map[0x01] = {"ORA", 0x01, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr};
  instruction_map[0x05] = {"ORA", 0x05, 2, 3, AddressMode::ZERO_PAGE, nullptr};
  instruction_map[0x06] = {"ORA", 0x06, 2, 5, AddressMode::ZERO_PAGE_AND_X, nullptr};
  instruction_map[0x09] = {"ORA", 0x09, 2, 2, AddressMode::IMMEDIATE, nullptr};
  instruction_map[0x0D] = {"ORA", 0x0D, 3, 4, AddressMode::ABSOLUTE, nullptr};
  instruction_map[0x0E] = {"ORA", 0x0E, 3, 6, AddressMode::ABSOLUTE_AND_X, nullptr};

  instruction_map[0xA9] = {"LDA", 0xA9, 2, 2, AddressMode::IMMEDIATE, nullptr};
  instruction_map[0xA5] = {"LDA", 0xA5, 2, 3, AddressMode::ZERO_PAGE, nullptr};
  instruction_map[0xB5] = {"LDA", 0xB5, 2, 4, AddressMode::ZERO_PAGE_AND_X, nullptr};
  instruction_map[0xAD] = {"LDA", 0xAD, 3, 4, AddressMode::ABSOLUTE, nullptr};
  instruction_map[0xBD] = {"LDA", 0xBD, 3, 4 /* +1 if page crossed */, AddressMode::ABSOLUTE_AND_X, nullptr};
  instruction_map[0xB9] = {"LDA", 0xB9, 3, 4 /* +1 if page crossed */, AddressMode::ABSOLUTE_AND_Y, nullptr};
  instruction_map[0xA1] = {"LDA", 0xA1, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr};
  instruction_map[0xB1] = {"LDA", 0xB1, 2, 5 /* +1 if page crossed */, AddressMode::INDIRECT_INDEXED, nullptr};

  instruction_map[0x85] = {"STA", 0x85, 2, 3, AddressMode::ZERO_PAGE, nullptr};
  instruction_map[0x95] = {"STA", 0x95, 2, 4, AddressMode::ZERO_PAGE_AND_X, nullptr};
  instruction_map[0x8D] = {"STA", 0x8D, 3, 4, AddressMode::ABSOLUTE, nullptr};
  instruction_map[0x9D] = {"STA", 0x9D, 3, 5, AddressMode::ABSOLUTE_AND_X, nullptr};
  instruction_map[0x99] = {"STA", 0x99, 3, 5, AddressMode::ABSOLUTE_AND_Y, nullptr};
  instruction_map[0x81] = {"STA", 0x81, 2, 6, AddressMode::INDEXED_INDIRECT, nullptr};
  instruction_map[0x91] = {"STA", 0x91, 2, 6, AddressMode::INDIRECT_INDEXED, nullptr};

  instruction_map[0xAA] = {"TAX", 0xAA, 1, 2, AddressMode::IMPLICIT, nullptr};
  instruction_map[0xE8] = {"INX", 0xE8, 1, 2, AddressMode::IMPLICIT, nullptr};

  instruction_map[0x4C] = {"JMP", 0x4C, 3, 3, AddressMode::ABSOLUTE, nullptr};
  instruction_map[0x6C] = {"JMP", 0x6C, 3, 5, AddressMode::INDIRECT, nullptr};

  instruction_map[0xEA] = {"NOP", 0xEA, 1, 2, AddressMode::IMPLICIT, std::bind(&Emulator::NOP, this, _1)};
  // custom instruction to end the program cleanly
  instruction_map[0xFF - 1] = {"DONE", 0xFF-1, 0, 0, AddressMode::IMPLICIT, nullptr};
}