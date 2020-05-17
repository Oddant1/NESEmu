#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running
void CPUClass::run()
{
    // TODO: We need some kind of wait code to make sure the processor stays in
    // sync. Leverington's code could be some inspiration for this
}

// Do these need to be seperate functions? Fundamentally what will this look like?
//  - Look at memory[programCounter]
//  - Probably have a vector/array of opcode functions set up to index into based
//    on this value
//  - Execute the function identified
//  - Increment counter appropriately (i.e move it over operands to next opcode)
//      - This could also involve non-incrementing movement of counter like a return to stack
//      - This means stack movement probably needs to be done in the function actually handling
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

// TODO: Do we make different functions for the different addressing modes?
// ANSWER: Probably? Not like any of these operations are particularly complicated
void CPUClass::ADC_Immediate()
{
    // TODO: Set status bits appropriately
    // TODO: Is immediate just add to acc?
    accumulator += memory[ programCounter ];
}
