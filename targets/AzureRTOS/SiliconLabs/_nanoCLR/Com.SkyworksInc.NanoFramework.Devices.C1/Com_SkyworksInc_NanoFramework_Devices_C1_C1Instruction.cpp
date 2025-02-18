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
