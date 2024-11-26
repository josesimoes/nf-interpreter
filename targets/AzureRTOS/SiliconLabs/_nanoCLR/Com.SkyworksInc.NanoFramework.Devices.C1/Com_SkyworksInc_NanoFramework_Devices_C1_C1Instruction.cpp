#include "Com_SkyworksInc_NanoFramework_Devices_C1_C1Instruction.h"

C1Instruction::C1Instruction()
{

}

C1Instruction::C1Instruction(C1InstructionName t, int s1, int s2) 
{
    instructionType = t;
    states[0] = s1;
    states[1] = s2;
}

C1Instruction* fetchInstruction(std::map<C1InstructionName, C1Instruction>& instructions, C1InstructionName name) 
{
    auto it = instructions.find(name);
    if (it != instructions.end()) {
        return &(it->second);
    }
    return nullptr; // Return nullptr if the instruction is not found
}
