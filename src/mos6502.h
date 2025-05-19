#ifndef MOS_6502_H
#define MOS_6502_H

#include "types.h"

struct MOS_6502 
{
    Word program_counter; 
    Word stack_pointer;

    /* For all arithmetic operations*/
    Word accumulator; 

    /* General Purpose Registers */
    Byte X; 
    Byte Y; 

    /* For processor state*/
    Byte processor_status = 0x0;

};

#endif // MOS_6502_H