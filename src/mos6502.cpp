#include "mos6502.h"
#include <iostream>
#include <cstring>
#include "util.h"

#define DELAY_CYCLES(map, opcode) for (int i = 0; i < map[opcode].cycles; ++i) delayMicros(CLOCK_uS)
#define IS_BIT_ON(n, i) ((n & (1 << i)) == (1 << i))

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

void Emulator::handleArithmeticFlagChanges(Byte value)
{
  cpu.P &= ~MOS_6502::P_ZERO; 
  cpu.P &= ~MOS_6502::P_NEGATIVE;

  if (value == 0) 
  {
    cpu.P |= MOS_6502::P_ZERO;
  }

  if (IS_BIT_ON(value, 7)) 
  {
    cpu.P |= MOS_6502::P_NEGATIVE;
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

Byte *Emulator::immediate()
{
  Word addr = ++cpu.program_counter;
  return mem.memory + addr;
}

Byte* Emulator::zeroPage()
{
  Byte offset = mem.readByte(++cpu.program_counter);
  return mem.memory + offset;
}

Byte* Emulator::zeroPageX()
{
  Byte page_offset = mem.readByte(++cpu.program_counter); 
  Byte offset = cpu.X; 
  return mem.memory + ((page_offset + offset) & 0xFF); 
}

Byte* Emulator::zeroPageY() 
{
  Byte page_offset = mem.readByte(++cpu.program_counter); 
  Byte offset = cpu.Y; 
  return mem.memory + ((page_offset + offset) & 0xFF); // simulate zpg round behavior
}

Byte* Emulator::relative() 
{
  // make sure to range limit the offset
  SignedByte offset = (SignedByte)mem.readByte(++cpu.program_counter);
  return mem.memory + (cpu.program_counter + offset);  
}

/* spit out an address */
Byte *Emulator::absolute()
{
  Byte low = mem.readByte(++cpu.program_counter); 
  Byte high = mem.readByte(++cpu.program_counter); 
  Word addr = ((Word)low | ((Word)high << 8)); 
  return mem.memory + addr; 
}

Byte *Emulator::absoluteX() 
{
  int offset = cpu.X; 
  Byte low = mem.readByte(++cpu.program_counter); 
  Byte high = mem.readByte(++cpu.program_counter); 
  Word addr = ((Word)low | ((Word)high << 8)); 

  // if these are different pages, incur page penalty
  if ((addr & 0xFF00) != ((addr + offset) & 0xFF00))
  {
    delayMicros(CLOCK_uS);
  }

  return mem.memory + addr + offset; 
}

Byte *Emulator::absoluteY() 
{
  int offset = cpu.Y; 
  Byte low = mem.readByte(++cpu.program_counter); 
  Byte high = mem.readByte(++cpu.program_counter); 
  Word addr = ((Word)low | ((Word)high << 8)); 

  // if these are different pages, incur page penalty
  if ((addr & 0xFF00) != ((addr + offset) & 0xFF00))
  {
    delayMicros(CLOCK_uS);
  }

  return mem.memory + addr + offset; 
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
  Byte pointer_high = mem.readByte((location & 0xFF00) & ((location + 1) & 0x00FF)); // simulate the lovely bugs created by them 6502 developers
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

  // incur page crossing penalty
  if ((target_address & 0xFF00) != ((target_address + offset) & 0xFF00))
  {
    delayMicros(CLOCK_uS);
  }

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
  cpu.accumulator |= *addr; 

  cpu.P &= ~MOS_6502::P_ZERO; 
  cpu.P &= ~MOS_6502::P_NEGATIVE;
  
  if (*addr == 0) 
  {
    cpu.P |= MOS_6502::P_ZERO;
  }

  if (IS_BIT_ON(*addr, 7)) // is negative bit sign on
  {
    cpu.P |= MOS_6502::P_NEGATIVE;
  }
}

void Emulator::INX(int opcode) 
{
  cpu.X++; 
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::INY(int opcode)
{
  cpu.Y++; 
  handleArithmeticFlagChanges(cpu.Y);
}

void Emulator::DEX(int opcode)
{
  cpu.X--;
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::DEY(int opcode) 
{
  cpu.Y--; 
  handleArithmeticFlagChanges(cpu.Y);
}

void Emulator::TXA(int opcode)
{
  cpu.accumulator = cpu.X;
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::TAY(int opcode)
{
  cpu.Y = cpu.accumulator;
  handleArithmeticFlagChanges(cpu.Y);
}

void Emulator::TSX(int opcode)
{
  cpu.X = cpu.S; 
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::TXS(int opcode)
{
  cpu.S = cpu.X; 
  handleArithmeticFlagChanges(cpu.S);
}

void Emulator::TYA(int opcode)
{
  cpu.accumulator = cpu.Y; 
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::TAX(int opcode) 
{
  cpu.X = cpu.accumulator;
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::LDA(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  cpu.accumulator = *addr; 
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::LDX(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  cpu.X = *addr; 
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::LDY(int opcode) 
{
  Byte* addr = handleAddressing(opcode); 
  cpu.Y = *addr; 
  handleArithmeticFlagChanges(cpu.Y);
}

void Emulator::STA(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  *addr = cpu.accumulator;
}

void Emulator::STX(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  *addr = cpu.X;
}

void Emulator::STY(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  *addr = cpu.Y;
}

void Emulator::JMP(int opcode)
{
  Byte* location = handleAddressing(opcode);
  // jump to that memory location (waoh)
  cpu.program_counter = (Word)(location - mem.memory);
}

MOS_6502::MOS_6502() 
    : program_counter(Memory::ROM_START),
      stack_pointer(Memory::STACK_BASE),
      accumulator(0), 
      X(0), 
      Y(0), 
      P(0x34)
{

}

void Emulator::initInstructionMap()
{
    auto MAKE_BINDING = [&](void (Emulator::*member_func_ptr)(int)) {
        return std::function<void(Byte)>(std::bind(member_func_ptr, this, _1));
    };

    // BRK - Break
    instruction_map[0x00] = {"BRK", 0x00, 1, 7, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::BRK)};

    // ORA - Logical Inclusive OR
    instruction_map[0x01] = Instruction{"ORA", 0x01, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::ORA)}; // (Indirect,X)
    instruction_map[0x05] = {"ORA", 0x05, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::ORA)};
    instruction_map[0x15] = {"ORA", 0x15, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::ORA)};
    instruction_map[0x09] = {"ORA", 0x09, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::ORA)};
    instruction_map[0x0D] = {"ORA", 0x0D, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::ORA)};
    instruction_map[0x1D] = {"ORA", 0x1D, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::ORA)}; // Cycles typically 4+ if page boundary crossed
    instruction_map[0x19] = {"ORA", 0x19, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::ORA)}; // Cycles typically 4+ if page boundary crossed
    instruction_map[0x11] = {"ORA", 0x11, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::ORA)}; // (Indirect),Y, cycles typically 5+ if page boundary crossed

    // LDA - Load Accumulator
    instruction_map[0xA9] = {"LDA", 0xA9, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::LDA)};
    instruction_map[0xA5] = {"LDA", 0xA5, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::LDA)};
    instruction_map[0xB5] = {"LDA", 0xB5, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::LDA)};
    instruction_map[0xAD] = {"LDA", 0xAD, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::LDA)};
    instruction_map[0xBD] = {"LDA", 0xBD, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::LDA)}; // Cycles typically 4+ if page boundary crossed
    instruction_map[0xB9] = {"LDA", 0xB9, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::LDA)}; // Cycles typically 4+ if page boundary crossed
    instruction_map[0xA1] = {"LDA", 0xA1, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::LDA)}; // (Indirect,X)
    instruction_map[0xB1] = {"LDA", 0xB1, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::LDA)}; // (Indirect),Y, cycles typically 5+ if page boundary crossed

    // STA - Store Accumulator
    instruction_map[0x85] = {"STA", 0x85, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::STA)};
    instruction_map[0x95] = {"STA", 0x95, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::STA)};
    instruction_map[0x8D] = {"STA", 0x8D, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::STA)};
    instruction_map[0x9D] = {"STA", 0x9D, 3, 5, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::STA)}; // Store instructions don't have page cross penalty
    instruction_map[0x99] = {"STA", 0x99, 3, 5, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::STA)}; // Store instructions don't have page cross penalty
    instruction_map[0x81] = {"STA", 0x81, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::STA)}; // (Indirect,X)
    instruction_map[0x91] = {"STA", 0x91, 2, 6, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::STA)}; // (Indirect),Y

    // Miscellaneous
    instruction_map[0xAA] = {"TAX", 0xAA, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::TAX)};
    instruction_map[0xE8] = {"INX", 0xE8, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::INX)};

    instruction_map[0x4C] = {"JMP", 0x4C, 3, 3, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::JMP)};
    instruction_map[0x6C] = {"JMP", 0x6C, 3, 5, AddressMode::INDIRECT, MAKE_BINDING(&Emulator::JMP)};

    instruction_map[0xEA] = {"NOP", 0xEA, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::NOP)};

    // Custom end-of-program instruction
    instruction_map[0xFE] = {"DONE", 0xFE, 1, 1, AddressMode::IMPLICIT, nullptr}; // nullptr for implementation as it's a custom termination
}

void Emulator::CLC(int opcode)
{
  cpu.P &= ~MOS_6502::P_CARRY;
}

void Emulator::CLD(int opcode)
{
  cpu.P &= ~MOS_6502::P_DECIMAL;
}

void Emulator::CLI(int opcode)
{
  cpu.P &= ~MOS_6502::P_INT_DISABLE; 
}


void Emulator::CLV(int opcode) 
{
  cpu.P &= ~MOS_6502::P_OVERFLOW; 
}

void Emulator::SEC(int opcode)
{
  cpu.P |= MOS_6502::P_CARRY;
}

void Emulator::SED(int opcode)
{
  cpu.P |= MOS_6502::P_DECIMAL;
}

void Emulator::SEI(int opcode)
{
  cpu.P |= MOS_6502::P_INT_DISABLE;
}

void Emulator::ADC(int opcode)
{
  Byte* addr = handleAddressing(opcode); 
  Byte operand = *addr; 
  
  cpu.accumulator += operand; 
}
