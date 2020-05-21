#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
    // TODO: Need some kind of wait code to make sure the processor stays in
    // sync. Leverington's code could be some inspiration for this
    ROMImage.read( ( char * )memory, 4 );

    // TODO: This is very much a "for now" roughup of finding which opcode to use.
    // As stated elsewhere we want the final method of finding a funciton to be
    // based on some kind of lookup table probably
    for( int i = 0; i < 4; i++ )
    {
        if( memory[ programCounter ] == 0x69 )
        {
            ADC_Immediate();
        }
    }
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

void CPUClass::updateNegative()
{
    if( accumulator < 0 )
    {
        status |= SET_NEGATIVE;
    }
    else
    {
        status &= ~SET_NEGATIVE;
    }
}

void CPUClass::updateOverflow( int8_t oldAccumulator )
{
    if( accumulator < 0 && oldAccumulator > 0 && memory[ programCounter ] > 0 )
    {
        status |= SET_OVERFLOW;
    }
    else
    {
        status &= ~SET_OVERFLOW;
    }
}

void CPUClass::updateBreak()
{

}

void CPUClass::updateDecimal()
{

}

void CPUClass::updateInterruptDisable()
{

}

void CPUClass::updateZero()
{
    if( accumulator == 0 )
    {
        status |= SET_ZERO;
    }
    else
    {
        status &= ~SET_ZERO;
    }
}

void CPUClass::updateCarry( int8_t oldAccumulator )
{
    if( ( accumulator > oldAccumulator && memory[ programCounter ] < 0 ) ||
        ( accumulator < oldAccumulator && memory[ programCounter ] > 0 ) )
    {
        status |= SET_CARRY;
    }
    else
    {
        status &= ~SET_CARRY;
    }
}

// TODO: Are different functions necessary for the different addressing modes?
// ANSWER: Probably? Not like any of these operations are particularly complicated
// $69
void CPUClass::ADC_Immediate()
{
    // TODO: Set status bits appropriately
    // TODO: Is immediate just add to acc?
    int8_t oldAccumulator = accumulator;

    programCounter++;

    accumulator += memory[ programCounter ];

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );

    programCounter++;
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
