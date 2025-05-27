#ifndef MOS_6502_H
#define MOS_6502_H

#include "types.h"
#include <string>
#include <functional>
#include <vector>

#define CHECK_REGISTER(reg, val) ((reg & val) == val)

#include "components.h"


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
    std::function<void(int)> implementation;
};

class Emulator 
{
public:
    void loadROM(const std::vector<Byte>& program);
    void run();

private:
    void handleArithmeticFlagChanges(Byte value);

    /* Addressing modes*/
    Byte* handleAddressing(int opcode); 
    Byte* accumulator(); 
    Byte* immediate(); 
    Byte* zeroPage(); 
    Byte* zeroPageX(); 
    Byte* zeroPageY(); 
    Byte* relative(); 
    Byte* absolute(); 
    Byte* absoluteX(); 
    Byte* absoluteY(); 
    Byte* indirect(); 
    Byte* indexedIndirect(); 
    Byte* indirectIndexed(); 


    void initInstructionMap();
    /* No operation */
    void NOP(int opcode);

    /* OR with accumulator */
    void ORA(int opcode);

    /* Load into registers */
    void LDA(int opcode);
    void LDX(int opcode);
    void LDY(int opcode);

    /* Transfer instructions */
    void TAX(int opcode);  // Transfer accumulator to X
    void TXA(int opcode);  // Transfer X to accumulator
    void TAY(int opcode);  // Transfer accumulator to Y
    void TSX(int opcode);  // Transfer stack pointer to X
    void TXS(int opcode);  // Transfer X to stack pointer
    void TYA(int opcode);  // Transfer Y to accumulator

    /* Increment and decrement */
    void INX(int opcode);
    void INY(int opcode);
    void DEX(int opcode);
    void DEY(int opcode);

    /* Break (interrupt) */
    void BRK(int opcode);

    /* Store registers to memory */
    void STA(int opcode);
    void STX(int opcode);
    void STY(int opcode);

    /* Jump and subroutine */
    void JMP(int opcode);
    void JSR(int opcode);
    void RTS(int opcode);

    /* Stack operations */
    void PHA(int opcode);
    void PLA(int opcode);
    void PHP(int opcode);
    void PLP(int opcode);

    /* Logical operations */
    void AND(int opcode);
    void EOR(int opcode);

    /* Arithmetic */
    void ADC(int opcode);
    void SBC(int opcode);

    /* Shift and rotate */
    void ASL(int opcode);
    void LSR(int opcode);
    void ROL(int opcode);
    void ROR(int opcode);

    /* Comparison */
    void CMP(int opcode);
    void CPX(int opcode);
    void CPY(int opcode);

    /* Branching */
    void BCC(int opcode);
    void BCS(int opcode);
    void BEQ(int opcode);
    void BMI(int opcode);
    void BNE(int opcode);
    void BPL(int opcode);
    void BVC(int opcode);
    void BVS(int opcode);

    /* Status flag control */
    void CLC(int opcode);
    void CLD(int opcode);
    void CLI(int opcode);
    void CLV(int opcode);
    void SEC(int opcode);
    void SED(int opcode);
    void SEI(int opcode);

    /* System */
    void RTI(int opcode);

private:
    struct MOS_6502 cpu; 
    struct Memory mem;
    Instruction instruction_map[0xFF];
};

// 256 insturction set architecture
#endif // MOS_6502_H