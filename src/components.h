#include "types.h"

struct MOS_6502 
{
    Word program_counter; 
    Word stack_pointer;

    /* For all arithmetic operations*/
    Byte accumulator; 

    /* General Purpose Registers */
    Byte X; 
    Byte Y;
    
    /* Stack pointer */
    Byte S;

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


/* Struct to handle addressing and memory stuff */
struct Memory
{
    Byte memory[WORD_MAX + 1];

    Byte readByte(Word address) { return *(memory + address); } 
    void writeByte(Word address, Byte value) { *(memory + address) = value; };

    // useful locations - zero page is the fastest memory
    constexpr static Word ZERO_PAGE_MAX = 0x00FF;
    constexpr static Word STACK_BASE = 0x0100; 
    constexpr static Word STACK_TOP = 0x01FF; // 256 byte stack
    constexpr static Word RAM_START = 0x0200;
    constexpr static Word RAM_END = 0x7FFF; // 32KB RAM
    constexpr static Word ROM_START = 0x8000; 
    constexpr static Word ROM_END = 0xFFFD; // 62.5 KB ROM
    constexpr static Word BRK_INT = 0xFFFE; 
    constexpr static Word BRK_INT_HI = 0xFFFF;
};