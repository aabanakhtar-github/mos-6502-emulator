#include "types.h"
#include "cstddef"

struct MOS_6502 
{
    bool operator==(const MOS_6502& rhs) const 
    {
        auto& lhs = *this; 
        return lhs.program_counter == rhs.program_counter &&
        lhs.accumulator     == rhs.accumulator &&
        lhs.X               == rhs.X &&
        lhs.Y               == rhs.Y &&
        lhs.S               == rhs.S &&
        lhs.P               == rhs.P;
    }

    bool operator!=(const MOS_6502& rhs) const 
    {
        return this->operator==(rhs);
    }

    Word program_counter = 0x8000; 

    /* For all arithmetic operations*/
    Byte accumulator = 0; 

    /* General Purpose Registers */
    Byte X = 0; 
    Byte Y = 0;
    
    /* Stack pointer starts at 0xFD for some 6502 specific things*/
    Byte S = 0xFD;

    /* For processor state */
    Byte P = P_UNUSED;

    /* Bit fields for the P register */
    constexpr static Byte P_NEGATIVE    =  0b10000000;
    constexpr static Byte P_OVERFLOW     = 0b01000000;
    constexpr static Byte P_UNUSED       = 0b00100000;  // not typically used
    constexpr static Byte P_BREAK        = 0b00010000;
    constexpr static Byte P_DECIMAL      = 0b00001000;
    constexpr static Byte P_INT_DISABLE  = 0b00000100;
    constexpr static Byte P_ZERO         = 0b00000010;
    constexpr static Byte P_CARRY        = 0b00000001;

};


/* Struct to handle addressing and memory stuff */
struct Memory
{
    Byte memory[WORD_MAX + 1];
    bool did_write = false; // used in test suite

    Memory()
    {
       for (size_t i = 0x8000; i < sizeof(memory); ++i) 
       {
            memory[i] = 0xFE; // to terminate the program asap for testing
       } 
    }

    Byte readByte(Word address) { return *(memory + address); } 
    void writeByte(Word address, Byte value) { *(memory + address) = value; };

    void stackPushByte(Byte& stack_register, Byte value); 
    void stackPushWord(Byte& stack_register, Word value); 

    Byte stackPullByte(Byte& stack_register);
    Word stackPullWord(Byte& stack_register);

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