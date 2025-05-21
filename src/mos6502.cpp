#include "mos6502.h"
#include <iostream>
#include <cstring>
#include "util.h"

#define DELAY_CYCLES(map, opcode) for (int i = 0; i < map[opcode].cycles; ++i) delayMicros(CLOCK_uS)
#define IS_BIT_ON(n, i) (n & (1 << i) == (1 << i))

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
    // simulate the delay
    DELAY_CYCLES(instruction_map, opcode);
    cpu.program_counter++;
  }
}

Byte* Emulator::handleAddressing(int opcode)
{
    auto mode = instruction_map[opcode].addressing_mode;
    switch (mode)
    {
    case AddressMode::IMMEDIATE:
        return immediate();
    case AddressMode::ZERO_PAGE:
        return zeroPage();
    case AddressMode::ZERO_PAGE_AND_X:
        return zeroPageX();
    case AddressMode::ZERO_PAGE_AND_Y:
        return zeroPageY();
    case AddressMode::ABSOLUTE:
        return absolute();
    case AddressMode::ABSOLUTE_AND_X:
        return absoluteX();
    case AddressMode::ABSOLUTE_AND_Y:
        return absoluteY();
    case AddressMode::INDIRECT:
        return indirect();
    case AddressMode::INDEXED_INDIRECT:
        return indexedIndirect();
    case AddressMode::INDIRECT_INDEXED:
        return indirectIndexed();
    case AddressMode::ACCUMULATOR:
        return accumulator();
    case AddressMode::IMPLICIT:
        // No operand, return 0 or a dummy value
        return nullptr;
    case AddressMode::RELATIVE:
        return relative();
    default:
        std::cerr << "Unhandled addressing mode" << std::endl;
        return nullptr;
    }
}

Byte* Emulator::accumulator() 
{
  return &cpu.accumulator;
}

Byte* Emulator::zeroPage()
{
  Byte offset = mem.readByte(++cpu.program_counter);
  return mem.memory + offset;
}

Byte* Emulator::zeroPageX()
{
  Byte page_offset = mem.readByte(++cpu.program_counter); 
  std::size_t offset = cpu.X; 
  return mem.memory + page_offset + offset; 
}

Byte* Emulator::zeroPageY() 
{
  Byte page_offset = mem.readByte(++cpu.program_counter); 
  std::size_t offset = cpu.Y; 
  return mem.memory + page_offset + offset;
}

Byte* Emulator::relative() 
{
  // make sure to range limit the offset
  SignedByte offset = (SignedByte)mem.readByte(++cpu.program_counter);
  // skip over one extra to hit the target instruction
  return mem.memory + (cpu.program_counter + offset + 1);  
}

/* pointer to pointer */
Byte* Emulator::indirect() 
{
  Byte lower_byte = mem.readByte(++cpu.program_counter); 
  Byte higher_byte = mem.readByte(++cpu.program_counter);
  Word location = ((Word)higher_byte << 8) | (Word)lower_byte;

  // just crash because this is weird
  if (location + 1 >= Memory::ROM_END)
  {
    std::exit(1);
  }

  Byte pointer_low = mem.readByte(location);
  Byte pointer_high = mem.readByte(location + 1);
  Word actual_location = ((Word)pointer_high << 8) | (Word)pointer_low;

  return mem.memory + actual_location;
}

/* (zp + x) */
Byte* Emulator::indexedIndirect()
{
  Byte offset = cpu.X; 
  /* location in the zero page of the initial address*/
  Byte location = mem.readByte(++cpu.program_counter); 
  /* The zero page address offsetted*/
  Byte zp_address = (Byte)(offset + location); // simulates a bug, making this effective only in the zero page
  Byte lower_byte = mem.readByte(zp_address); 
  Byte higher_byte = mem.readByte((Byte)(zp_address + 1));
  Word target_address = ((Word)lower_byte | ((Word)higher_byte << 8)); 
  // return (zp + x)
  return mem.memory + target_address;
}

/* (zp) + y  */
Byte* Emulator::indirectIndexed()
{
  Byte offset = cpu.Y; 
  Byte location = mem.readByte(++cpu.program_counter); // location in the zero page
  // get the address from the zero page
  Byte lower_byte = mem.readByte(location); 
  Byte higher_byte = mem.readByte((Byte)(location + 1)); // implement wrap around bug for 6502
  Word target_address = ((Word)lower_byte | (Word)(higher_byte << 8)); 
  // add offset to it
  return mem.memory + target_address + offset;
}

/* do nothing */
void Emulator::NOP(int opcode)
{
}

void Emulator::ORA(int opcode)
{
  Byte* addr = handleAddressing(opcode);
  Byte& target = *addr; // wrap around bultin :)
  target |= cpu.accumulator; 

  if (target == 0) {
    cpu.P |= MOS_6502::P_ZERO;
  }

  if (IS_BIT_ON(target, 7)) // is negative bit sign on
  {
    cpu.P |= MOS_6502::P_NEGATIVE;
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