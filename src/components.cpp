#include "components.h"

// apparently the stack grows right to left
void Memory::stackPushByte(Byte& stack_register, Byte value)
{
    memory[STACK_BASE + stack_register] = value; 
    did_write = true;
    stack_register--; 
}

// grows downward
Byte Memory::stackPullByte(Byte& stack_register) 
{
    ++stack_register;
    Byte value = memory[STACK_BASE + stack_register]; 
    did_write = false;
    return value;
}

void Memory::stackPushWord(Byte& stack_register, Word value) 
{
    Byte lower_byte = (Byte)(value & 0xFF); 
    Byte higher_byte = (Byte)(value >> 8); 
    stackPushByte(stack_register, higher_byte);  // push high byte second (on lower address)
    stackPushByte(stack_register, lower_byte);   // push low byte first (on higher address)
    did_write = true;
}

Word Memory::stackPullWord(Byte& stack_register) 
{
    Byte lower_byte = stackPullByte(stack_register);   // pulled later = was pushed first
    Byte higher_byte = stackPullByte(stack_register);  // pulled first = was pushed second
    did_write = false;
    return (Word)lower_byte | (((Word)higher_byte) << 8);
}