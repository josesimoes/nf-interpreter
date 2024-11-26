// Instruction.h
#ifndef C1INSTRUCTION_H
#define C1INSTRUCTION_H

#include <map>

// Enum for instruction names
enum C1InstructionName {
    DATA_WRITE,
    DATA_READ,
    ADDRESS_WRITE,
    ADDRESS_READ
};

// Instruction class definition
class C1Instruction {
public:
    C1InstructionName instructionType;
    int states[2];

    C1Instruction();
    C1Instruction(C1InstructionName t, int s1, int s2);
};

// Function to fetch instruction from map (dictionary)
C1Instruction* fetchInstruction(std::map<C1InstructionName, C1Instruction>& instructions, C1InstructionName name);

#endif // C1INSTRUCTION_H