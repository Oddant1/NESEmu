#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running here
void CPUClass::run()
{
    // TODO: Need some kind of wait code to make sure the processor stays in
    // sync. Leverington's code could be some inspiration for this
}

// Do these need to be seperate functions? Fundamentally what will this look like?
//  FETCH:
//      - Look at memory[programCounter]
//  DECODE:
//      - Probably have a vector/array of opcode functions set up to index into based
//        on this value
//  EXECUTE:
//      - Execute the function identified
//  - Increment counter appropriately (i.e move it over operands to next opcode)
//      - This could also involve non-incrementing movement of counter like a return to stack
//      - This means stack/pc movement probably needs to be done in the function actually handling
//        the opcode or helpers called from there
// I feel like this doesn't require seperate functions, but we'll see
void CPUClass::fetch()
{

}

void CPUClass::decode()
{

}

void CPUClass::execute()
{

}

// TODO: Are different functions necessary for the different addressing modes?
// ANSWER: Probably? Not like any of these operations are particularly complicated
// $69
void CPUClass::ADC_Immediate()
{
    // TODO: Set status bits appropriately
    // TODO: Is immediate just add to acc?
    // TODO: NES uses signed magnitude (for some reason. . .) This is a problem
    // because g++ uses 2's compliment. Need to translate
    accumulator += memory[ programCounter ];
}

// $ 65
void CPUClass::ADC_Zero_Page()
{

}

// $75
void CPUClass::ADC_Zero_Page_X()
{

}

// $6D
void CPUClass::ADC_Absolute()
{

}

// $7D
void CPUClass::ADC_Absolute_X()
{

}

// $79
void CPUClass::ADC_Absolute_Y()
{

}

// $61
void CPUClass::ADC_Indirect_X()
{

}

// $71
void CPUClass::ADC_Indirect_Y()
{

}
