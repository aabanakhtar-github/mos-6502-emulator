#ifndef MOS_6502_H
#define MOS_6502_H

#include "types.h"
#include <string>
#include <functional>
#include <vector>

#define CHECK_REGISTER(reg, val) ((reg & val) == val)




struct MOS_6502 
{
    Word program_counter; 
    Word stack_pointer;

    /* For all arithmetic operations*/
    Byte accumulator; 

    /* General Purpose Registers */
    Byte X; 
    Byte Y; 

    /* For processor state */
    Byte P = 0x0;

    /* Bit fields for the P register */
    constexpr static int P_NEGATIVE     = 0b10000000;
    constexpr static int P_OVERFLOW     = 0b01000000;
    constexpr static int P_UNUSED       = 0b00100000;  // not typically used
    constexpr static int P_BREAK        = 0b00010000;
    constexpr static int P_DECIMAL      = 0b00001000;
    constexpr static int P_INT_DISABLE  = 0b00000100;
    constexpr static int P_ZERO         = 0b00000010;
    constexpr static int P_CARRY        = 0b00000001;


    explicit MOS_6502();
};


struct Memory
{
    Byte memory[WORD_MAX];

    Byte readByte(Word address) { return *(memory + address); } 
    void writeByte(Word address, Byte value) { *(memory + address) = value; };

    // useful locations - zero page is the fastest memory
    constexpr static Word ZERO_PAGE_MAX = 0x00FF;
    constexpr static Word STACK_BASE = 0x0100; 
    constexpr static Word STACK_TOP = 0x01FF; // 256 byte stack
    constexpr static Word RAM_START = 0x0200;
    constexpr static Word RAM_END = 0x7FFF; // 32KB RAM
    constexpr static Word ROM_START = 0x8000; 
    constexpr static Word ROM_END = 0xFFFF; // 62.5 KB ROM
};

// instructions have different address modes
enum class AddressMode
{
    IMPLICIT, 
    ACCUMULATOR, 
    IMMEDIATE, 
    ZERO_PAGE, 
    ZERO_PAGE_AND_X, 
    ZERO_PAGE_AND_Y, 
    RELATIVE, 
    ABSOLUTE,
    ABSOLUTE_AND_X, 
    ABSOLUTE_AND_Y, 
    INDIRECT, // pointer to pointer
    INDEXED_INDIRECT, // pointer to pointer (8 bit) + x,
    INDIRECT_INDEXED, // pointer to pointer(16 bit) + y
};

struct Instruction
{
    std::string name;
    Byte opcode;
    std::size_t args_count;
    std::size_t cycles; 
    AddressMode addressing_mode;
    std::function<void(Byte)> implementation;
};

class Emulator 
{
public:
    void loadROM(const std::vector<Byte>& program);
    void run();

private:
    void initInstructionMap();
    void NOP(int opcode);

private:
    struct MOS_6502 cpu; 
    struct Memory mem;
    Instruction instruction_map[0xFF];
};

// 256 insturction set architecture
#endif // MOS_6502_H