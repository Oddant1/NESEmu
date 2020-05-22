#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
    // TODO: Need some kind of wait code to make sure the processor stays in
    // sync. Leverington's code could be some inspiration for this
    ROMImage.read( ( char * )memory, 6 );

    // TODO: This is very much a "for now" roughup of finding which opcode to use.
    // As stated elsewhere we want the final method of finding a funciton to be
    // based on some kind of lookup table probably
    for( int i = 0; i < 2; i++ )
    {
        if( memory[ programCounter ] == 0x69 )
        {
            ADCImmediate();
        }
        else if( memory[ programCounter ] == 0x65 )
        {
            ADCZeroPage();
        }
        else
        {
            ADCZeroPageX();
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
inline int8_t CPUClass::fetch()
{
    return memory[ programCounter ];
}

inline voidFunc CPUClass::decode()
{
    // TODO: This will index into our func array once we have one
    return NULL;
}

inline void CPUClass::execute()
{
    instructionRegister();
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
    if( accumulator < 0 && oldAccumulator > 0 && MDR > 0 )
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
    if( ( accumulator > oldAccumulator && MDR < 0 ) ||
        ( accumulator < oldAccumulator && MDR > 0 ) )
    {
        status |= SET_CARRY;
    }
    else
    {
        status &= ~SET_CARRY;
    }
}

inline int8_t CPUClass::immediate()
{
    return memory[ programCounter++ ];
}

uint16_t CPUClass::zeroPage( IndexingMode mode )
{
    int8_t offset = retrieveIndexOffset( mode );

    // The value is supposed to just wrap to stay on the zero page, so we don't
    // need to handle overflows at all
    return memory[ programCounter++ ] + offset;
}

uint16_t CPUClass::absolute( IndexingMode mode )
{
    int16_t address;

    int8_t offset = retrieveIndexOffset( mode );

    uint8_t low = memory[ programCounter++ ];
    uint8_t high = memory[ programCounter++ ];

    low += offset;

    // Check for carry
    if( low < offset )
    {
        high++;
    }

    address = high;
    address << 4;

    return address += low;
}

uint16_t CPUClass::indirect( IndexingMode mode )
{
    return 0;
}

int8_t  CPUClass::retrieveIndexOffset( IndexingMode mode )
{
    switch( mode )
    {
        case WITH_X:
            return X;

        case WITH_Y:
            return Y;

        default:
            return 0x00;
    }
}

// TODO: This is looking more and more like I should make a single ADC function
// with some kind of flag for what type to use. . . That's gets hairy when
// decoding though
// $69
void CPUClass::ADCImmediate()
{
    int8_t oldAccumulator = accumulator;

    // TODO: This will probably get moved to fetch
    programCounter++;

    MDR = immediate();
    accumulator += MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// $ 65
void CPUClass::ADCZeroPage()
{
    int8_t oldAccumulator = accumulator;

    programCounter++;

    MDR = memory[ zeroPage( NONE ) ];
    accumulator += MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// $75
void CPUClass::ADCZeroPageX()
{
    int8_t oldAccumulator = accumulator;

    programCounter++;

    MDR = memory[ zeroPage( WITH_X ) ];
    accumulator += MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// $6D
void CPUClass::ADCAbsolute()
{
    int8_t oldAccumulator = accumulator;

    programCounter++;

    MDR = memory[ absolute( NONE ) ];
    accumulator += MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// $7D
void CPUClass::ADCAbsoluteX()
{

}

// $79
void CPUClass::ADCAbsoluteY()
{

}

// $61
void CPUClass::ADCIndirectX()
{

}

// $71
void CPUClass::ADCIndirectY()
{

}
