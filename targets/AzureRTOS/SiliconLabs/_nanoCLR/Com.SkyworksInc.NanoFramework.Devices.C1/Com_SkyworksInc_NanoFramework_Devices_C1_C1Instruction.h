// Instruction.h
#ifndef C1INSTRUCTION_H
#define C1INSTRUCTION_H

// Enum for instruction names
enum C1InstructionName {
    DATA_WRITE = 0,
    DATA_READ = 1,
    ADDRESS_WRITE = 2,
    ADDRESS_READ = 3
};

// Instruction class definition
class C1Instruction {
public:
    C1InstructionName instructionType;
    int states[2];

    C1Instruction();
    C1Instruction(C1InstructionName t, int s1, int s2);
};

#endif // C1INSTRUCTION_H