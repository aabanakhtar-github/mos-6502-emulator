#include "mos6502.h"
#include <iostream>
#include <cstring>
#include "util.h"

#define DELAY_CYCLES(map, opcode)              \
  for (int i = 0; i < map[opcode].cycles; ++i) \
  delayMicros(CLOCK_uS)
#define IS_BIT_ON(n, i) ((n & (1 << i)) == (1 << i))

using namespace std::placeholders;

void Emulator::loadROM(const std::vector<Byte> &program)
{
  // define our instructions
  if (Memory::ROM_END - Memory::ROM_START < program.size())
  {
    std::cerr << "Cannot install ROM, too big." << std::endl;
  }

  std::memcpy(mem.memory + Memory::ROM_START, program.data(), program.size());
  std::cout << "Loaded ROM successfully." << std::endl;
}

void Emulator::run()
{
  std::cout << "gyatt" << std::endl;
  while (cpu.program_counter < Memory::ROM_END)
  {
    if (testing)
    {
    }

    int opcode = mem.readByte(cpu.program_counter);
    // special opcode for terminating the program (added by emulator)
    if (opcode == 0xFE)
    {
      break;
    }

    auto instruction = instruction_map[opcode];
    instruction.implementation(opcode);
    // simulate the delay
    if (!testing)
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

Byte *Emulator::handleAddressing(int opcode)
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

Byte *Emulator::accumulator()
{
  return &cpu.accumulator;
}

Byte *Emulator::immediate()
{
  Word addr = ++cpu.program_counter;
  return mem.memory + addr;
}

Byte *Emulator::zeroPage()
{
  Byte offset = mem.readByte(++cpu.program_counter);
  return mem.memory + offset;
}

Byte *Emulator::zeroPageX()
{
  Byte page_offset = mem.readByte(++cpu.program_counter);
  Byte offset = cpu.X;
  return mem.memory + ((page_offset + offset) & 0xFF);
}

Byte *Emulator::zeroPageY()
{
  Byte page_offset = mem.readByte(++cpu.program_counter);
  Byte offset = cpu.Y;
  return mem.memory + ((page_offset + offset) & 0xFF); // simulate zpg round behavior
}

Byte *Emulator::relative()
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
Byte *Emulator::indirect()
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
Byte *Emulator::indexedIndirect()
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
Byte *Emulator::indirectIndexed()
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
  ((void)sizeof(char[1 - 2 * !!(printf(""))]));
}

void Emulator::ORA(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  cpu.accumulator |= *addr;
  handleArithmeticFlagChanges(cpu.accumulator);
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

void Emulator::INC(int opcode)
{
  Byte* location = handleAddressing(opcode);
  *location += 1;
  handleArithmeticFlagChanges(*location);
}

void Emulator::BRK(int opcode)
{
  // simulate pad byte
  cpu.program_counter++; 
  mem.stackPushWord(cpu.S, cpu.program_counter);          // Push PC
  mem.stackPushByte(cpu.S, cpu.P | MOS_6502::P_BREAK | MOS_6502::P_UNUSED); // Push status

  // disable interrupts (we are one)
  cpu.P |= MOS_6502::P_INT_DISABLE;

  Byte low = mem.readByte(0xFFFE);
  Byte high = mem.readByte(0xFFFF);
  cpu.program_counter = ((Word)high << 8) | low;
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
  Byte *addr = handleAddressing(opcode);
  cpu.accumulator = *addr;
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::LDX(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  cpu.X = *addr;
  handleArithmeticFlagChanges(cpu.X);
}

void Emulator::LDY(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  cpu.Y = *addr;
  handleArithmeticFlagChanges(cpu.Y);
}

void Emulator::STA(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  *addr = cpu.accumulator;
}

void Emulator::STX(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  *addr = cpu.X;
}

void Emulator::STY(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  *addr = cpu.Y;
}

void Emulator::JMP(int opcode)
{
  Byte *location = handleAddressing(opcode);
  // jump to that memory location (waoh)
  cpu.program_counter = (Word)(location - mem.memory);
}

void Emulator::JSR(int opcode)
{
  Byte *location = handleAddressing(opcode);
  mem.stackPushWord(cpu.S, cpu.program_counter - 2); // push the return address TODO: add -1 if this doesn't work
  cpu.program_counter = Word(location - mem.memory);
}

void Emulator::RTS(int opcode)
{
  Word return_address = mem.stackPullWord(cpu.S);
  cpu.program_counter = return_address;
}

/* TODO: UPDATE THE CODE TO SET THE B FLAG once we know about register sttuff*/
void Emulator::PHA(int opcode)
{
  mem.stackPushByte(cpu.S, cpu.accumulator);
}

void Emulator::PLA(int opcode)
{
  cpu.accumulator = mem.stackPullByte(cpu.S);
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::PHP(int opcode)
{
  mem.stackPushByte(cpu.S, cpu.P);
}

void Emulator::PLP(int opcode)
{
  cpu.P = mem.stackPullByte(cpu.S);
}

void Emulator::AND(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  cpu.accumulator &= *addr;
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::EOR(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  cpu.accumulator ^= *addr;
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::initInstructionMap()
{
  for (auto &i : instruction_map)
  {
    i = {"DONE", 0xFE, 1, 1, AddressMode::IMPLICIT, [](int) {}}; // nullptr for implementation as it's a custom termination
  }

  auto MAKE_BINDING = [&](void (Emulator::*member_func_ptr)(int))
  {
    return std::function<void(Byte)>(std::bind(member_func_ptr, this, _1));
  };

  instruction_map[0x70] = {"BVS", 0x70, 2, 2, AddressMode::RELATIVE, MAKE_BINDING(&Emulator::BVS)};
  instruction_map[0x00] = {"BRK", 0x00, 1, 7, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::BRK)};
  instruction_map[0xC9] = {"CMP", 0xC9, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xC5] = {"CMP", 0xC5, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xD5] = {"CMP", 0xD5, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xCD] = {"CMP", 0xCD, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xDD] = {"CMP", 0xDD, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xD9] = {"CMP", 0xD9, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xC1] = {"CMP", 0xC1, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xD1] = {"CMP", 0xD1, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::CMP)};
  instruction_map[0xE0] = {"CPX", 0xE0, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::CPX)};
  instruction_map[0xE4] = {"CPX", 0xE4, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::CPX)};
  instruction_map[0xEC] = {"CPX", 0xEC, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::CPX)};
  instruction_map[0xC0] = {"CPY", 0xC0, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::CPY)};
  instruction_map[0xC4] = {"CPY", 0xC4, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::CPY)};
  instruction_map[0xCC] = {"CPY", 0xCC, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::CPY)};
  instruction_map[0xCA] = {"DEX", 0xCA, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::DEX)};
  instruction_map[0x88] = {"DEY", 0x88, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::DEY)};
  instruction_map[0x49] = {"EOR", 0x49, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x45] = {"EOR", 0x45, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x55] = {"EOR", 0x55, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x4D] = {"EOR", 0x4D, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x5D] = {"EOR", 0x5D, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x59] = {"EOR", 0x59, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x41] = {"EOR", 0x41, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0x51] = {"EOR", 0x51, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::EOR)};
  instruction_map[0xE8] = {"INX", 0xE8, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::INX)};
  instruction_map[0xC8] = {"INY", 0xC8, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::INY)};
  instruction_map[0x4C] = {"JMP", 0x4C, 3, 3, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::JMP)};
  instruction_map[0x6C] = {"JMP", 0x6C, 3, 5, AddressMode::INDIRECT, MAKE_BINDING(&Emulator::JMP)};
  instruction_map[0x20] = {"JSR", 0x20, 3, 6, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::JSR)};
  instruction_map[0xA9] = {"LDA", 0xA9, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xA5] = {"LDA", 0xA5, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xB5] = {"LDA", 0xB5, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xAD] = {"LDA", 0xAD, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xBD] = {"LDA", 0xBD, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xB9] = {"LDA", 0xB9, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xA1] = {"LDA", 0xA1, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xB1] = {"LDA", 0xB1, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::LDA)};
  instruction_map[0xA2] = {"LDX", 0xA2, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::LDX)};
  instruction_map[0xA6] = {"LDX", 0xA6, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::LDX)};
  instruction_map[0xB6] = {"LDX", 0xB6, 2, 4, AddressMode::ZERO_PAGE_AND_Y, MAKE_BINDING(&Emulator::LDX)};
  instruction_map[0xAE] = {"LDX", 0xAE, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::LDX)};
  instruction_map[0xBE] = {"LDX", 0xBE, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::LDX)};
  instruction_map[0xA0] = {"LDY", 0xA0, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::LDY)};
  instruction_map[0xA4] = {"LDY", 0xA4, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::LDY)};
  instruction_map[0xB4] = {"LDY", 0xB4, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::LDY)};
  instruction_map[0xAC] = {"LDY", 0xAC, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::LDY)};
  instruction_map[0xBC] = {"LDY", 0xBC, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::LDY)};
  instruction_map[0x4A] = {"LSR", 0x4A, 1, 2, AddressMode::ACCUMULATOR, MAKE_BINDING(&Emulator::LSR)};
  instruction_map[0x46] = {"LSR", 0x46, 2, 5, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::LSR)};
  instruction_map[0x56] = {"LSR", 0x56, 2, 6, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::LSR)};
  instruction_map[0x4E] = {"LSR", 0x4E, 3, 6, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::LSR)};
  instruction_map[0x5E] = {"LSR", 0x5E, 3, 7, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::LSR)};
  instruction_map[0xEA] = {"NOP", 0xEA, 1, 2, AddressMode::IMPLICIT, MAKE_BINDING(&Emulator::NOP)};
  instruction_map[0x09] = {"ORA", 0x09, 2, 2, AddressMode::IMMEDIATE, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x05] = {"ORA", 0x05, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x15] = {"ORA", 0x15, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x0D] = {"ORA", 0x0D, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x1D] = {"ORA", 0x1D, 3, 4, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x19] = {"ORA", 0x19, 3, 4, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x01] = {"ORA", 0x01, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::ORA)};
  instruction_map[0x11] = {"ORA", 0x11, 2, 5, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::ORA)};
    // STA - Store Accumulator
  instruction_map[0x85] = {"STA", 0x85, 2, 3, AddressMode::ZERO_PAGE, MAKE_BINDING(&Emulator::STA)};
  instruction_map[0x95] = {"STA", 0x95, 2, 4, AddressMode::ZERO_PAGE_AND_X, MAKE_BINDING(&Emulator::STA)};
  instruction_map[0x8D] = {"STA", 0x8D, 3, 4, AddressMode::ABSOLUTE, MAKE_BINDING(&Emulator::STA)};
  instruction_map[0x9D] = {"STA", 0x9D, 3, 5, AddressMode::ABSOLUTE_AND_X, MAKE_BINDING(&Emulator::STA)};
  instruction_map[0x99] = {"STA", 0x99, 3, 5, AddressMode::ABSOLUTE_AND_Y, MAKE_BINDING(&Emulator::STA)};
  instruction_map[0x81] = {"STA", 0x81, 2, 6, AddressMode::INDEXED_INDIRECT, MAKE_BINDING(&Emulator::STA)}; // (Indirect,X)
  instruction_map[0x91] = {"STA", 0x91, 2, 6, AddressMode::INDIRECT_INDEXED, MAKE_BINDING(&Emulator::STA)}; // (Indirect),Y

    // Custom end-of-program instruction
  instruction_map[0xFE] = {"DONE", 0xFE, 1, 1, AddressMode::IMPLICIT, [](int) {}}; // nullptr for implementation as it's a custom termination
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

void Emulator::RTI(int opcode)
{
  // get our status restored
  cpu.P = mem.stackPullByte(cpu.S);
  cpu.P &= ~MOS_6502::P_BREAK;
  // this should be set to 1 all times
  cpu.P |= MOS_6502::P_UNUSED;

  cpu.program_counter = mem.stackPullWord(cpu.S);
}

void Emulator::ADC(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte operand = *addr;

  Byte carry = (MOS_6502::P_CARRY & cpu.P) ? 1 : 0;
  Word sum = (Word)operand + (Word)cpu.accumulator + (Word)carry;

  // set carry flag
  if (sum > 0xFF)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  // set the overflow flag (if its humongous)
  int is_overflow = (~(cpu.accumulator ^ operand) & (cpu.accumulator ^ (Byte)sum)) & 0x80;
  if (is_overflow)
  {
    cpu.P |= MOS_6502::P_OVERFLOW;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_OVERFLOW;
  }

  cpu.accumulator = sum;
  // set other flags
  handleArithmeticFlagChanges(cpu.accumulator);
}

void Emulator::SBC(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte operand = *addr;

  Byte carry = (cpu.P & MOS_6502::P_CARRY) ? 1 : 0;
  Byte acc_before = cpu.accumulator;

  // Invert operand to simulate two's complement subtraction
  // this is some weird bit magic thing that does subtraction
  Word sum = (Word)acc_before + (Word)(~operand) + (Word)carry;
  Byte result = (Byte)sum;

  cpu.accumulator = result;

  // Carry flag = no borrow
  if (sum < 0x100)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  // Overflow: check signed overflow
  bool is_overflow = ((acc_before ^ result) & (~operand ^ result)) & 0x80;
  if (is_overflow)
  {
    cpu.P |= MOS_6502::P_OVERFLOW;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_OVERFLOW;
  }

  handleArithmeticFlagChanges(result);
}

void Emulator::ASL(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte should_carry = 0x80 & *addr;
  *addr = ((*addr << 1) & 0xFF); // shift one right and maintain 8 bits

  if (should_carry)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  handleArithmeticFlagChanges(*addr);
}

void Emulator::LSR(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte should_carry = *addr & 1; // least significant bit
  *addr = ((*addr >> 1) & 0xFF);

  if (should_carry)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  handleArithmeticFlagChanges(*addr);
}

void Emulator::ROL(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte should_carry = *addr & 0x80;
  Byte _0_bit_val = (cpu.P & MOS_6502::P_CARRY) ? 1 : 0;
  *addr = (*addr << 1) | (_0_bit_val);

  if (should_carry)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  handleArithmeticFlagChanges(*addr);
}

void Emulator::ROR(int opcode)
{
  Byte *addr = handleAddressing(opcode);
  Byte original_bit0 = *addr & 0x01;
  Byte carry_in = (cpu.P & MOS_6502::P_CARRY) ? 0x80 : 0;
  *addr = (*addr >> 1) | carry_in; // 7th bit

  if (original_bit0)
  {
    cpu.P |= MOS_6502::P_CARRY;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_CARRY;
  }

  handleArithmeticFlagChanges(*addr);
}

void Emulator::CMP(int opcode)
{
  Byte *addr = handleAddressing(opcode);

  // Compare: A - M
  Byte A = cpu.accumulator;
  Byte M = *addr;
  Byte result = A - M;

  if (A == M)
    cpu.P |= MOS_6502::P_ZERO;
  else
    cpu.P &= ~MOS_6502::P_ZERO;

  if (A >= M)
    cpu.P |= MOS_6502::P_CARRY;
  else
    cpu.P &= ~MOS_6502::P_CARRY;

  // Set Negative flag if bit 7 of (A - M) is set
  if (result & 0x80)
    cpu.P |= MOS_6502::P_NEGATIVE;
  else
    cpu.P &= ~MOS_6502::P_NEGATIVE;
}

void Emulator::CPX(int opcode)
{
  Byte *addr = handleAddressing(opcode);

  // Compare: X - M
  Byte X = cpu.X;
  Byte M = *addr;
  Byte result = X - M;

  if (X == M)
    cpu.P |= MOS_6502::P_ZERO;
  else
    cpu.P &= ~MOS_6502::P_ZERO;

  if (X >= M)
    cpu.P |= MOS_6502::P_CARRY;
  else
    cpu.P &= ~MOS_6502::P_CARRY;

  // Set Negative flag if bit 7 of (A - M) is set
  if (result & 0x80)
    cpu.P |= MOS_6502::P_NEGATIVE;
  else
    cpu.P &= ~MOS_6502::P_NEGATIVE;
}

void Emulator::CPY(int opcode)
{
  Byte *addr = handleAddressing(opcode);

  Byte Y = cpu.Y;
  Byte M = *addr;
  Byte result = Y - M;

  if (Y == M)
    cpu.P |= MOS_6502::P_ZERO;
  else
    cpu.P &= ~MOS_6502::P_ZERO;

  if (Y >= M)
    cpu.P |= MOS_6502::P_CARRY;
  else
    cpu.P &= ~MOS_6502::P_CARRY;

  // Set Negative flag if bit 7 of (A - M) is set
  if (result & 0x80)
    cpu.P |= MOS_6502::P_NEGATIVE;
  else
    cpu.P &= ~MOS_6502::P_NEGATIVE;
}

void Emulator::branchIf(bool condition, SignedByte offset)
{
  if (condition) 
  {
    cpu.program_counter += offset;
  }
}

void Emulator::BCC(int opcode)
{
  SignedByte offset = *handleAddressing(opcode);
  branchIf((cpu.P & MOS_6502::P_CARRY) == 0, offset); 
}

void Emulator::BCS(int opcode) 
{
  SignedByte offset = *handleAddressing(opcode); 
  branchIf(cpu.P & MOS_6502::P_CARRY, offset);
}

void Emulator::BEQ(int opcode) 
{
  SignedByte offset = *handleAddressing(opcode); 
  branchIf(cpu.P & MOS_6502::P_ZERO, offset);
}

void Emulator::BMI(int opcode)
{
  SignedByte offset = *handleAddressing(opcode); 
  branchIf(cpu.P & MOS_6502::P_NEGATIVE, offset);
}

void Emulator::BNE(int opcode)
{
  SignedByte offset = *handleAddressing(opcode); 
  branchIf((cpu.P & MOS_6502::P_ZERO) == 0, offset);
}

void Emulator::BPL(int opcode)
{
  SignedByte offset = *handleAddressing(opcode); 
  branchIf((cpu.P & MOS_6502::P_NEGATIVE) == 0, offset);
}

void Emulator::BVC(int opcode)
{
  SignedByte offset = *handleAddressing(opcode);
  branchIf((cpu.P & MOS_6502::P_OVERFLOW) == 0, offset);
}

void Emulator::BVS(int opcode)
{
  SignedByte offset = *handleAddressing(opcode);
  branchIf(cpu.P & MOS_6502::P_OVERFLOW, offset);
}

void Emulator::BIT(int opcode)
{
  Byte* addr = handleAddressing(opcode);
  Byte value = *addr;
  Byte result = cpu.accumulator & value;

  // Set or clear zero flag based on accumulator & memory
  if (result == 0)
  {
    cpu.P |= MOS_6502::P_ZERO;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_ZERO;
  }

  // Set Negative flag to bit 7 of memory operand
  if (value & 0x80)
  {
    cpu.P |= MOS_6502::P_NEGATIVE;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_NEGATIVE;
  }

  // Set Overflow flag to bit 6 of memory operand
  if (value & 0x40)
  {
    cpu.P |= MOS_6502::P_OVERFLOW;
  }
  else
  {
    cpu.P &= ~MOS_6502::P_OVERFLOW;
  }
}

