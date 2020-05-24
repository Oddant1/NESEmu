#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
    // struct timespec waitTime;
    // waitTime.tv_sec = 0;
    // waitTime.tv_nsec = 0;

    // TODO: Need some kind of wait code to make sure the processor stays in
    // sync. Leverington's code could be some inspiration for this
    ROMImage.read( ( char * )memory, 6 );

    // TODO: This is very much a "for now" roughup of finding which opcode to use.
    // As stated elsewhere we want the final method of finding a funciton to be
    // based on some kind of lookup table probably
    auto start = std::chrono::high_resolution_clock::now();
    for( int i = 0; i < 2; i++ )
    {
        // auto begin = std::chrono::high_resolution_clock::now();

        MDR = fetch();
        instructionRegister = decode();
        execute();

        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

        // waitTime.tv_nsec = numCycles * CYCLE_TIME_N_SEC - duration;
        // if( waitTime.tv_nsec > 0 )
        // {
        //     nanosleep(&waitTime, NULL);
        // }
    }
}

// I feel like this doesn't require seperate functions, but we'll see
inline int8_t CPUClass::fetch()
{
    return memory[ programCounter++ ];
}

// I thought a lot about better ways of decoding the opcode based on looking at
// this source https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
// however, they all seemed like they would get complicated and messy. The
// colmns are MOSTLY all the same addressing mode, but there are exceptions.
// The rows are just kind of a mess even looking at individual colors
inline CPUClass::voidFunc CPUClass::decode()
{
    return opCodeArray[ MDR ];
}

inline void CPUClass::execute()
{
    ( this->*instructionRegister )();
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
    uint8_t hi = memory[ programCounter++ ];

    low += offset;

    // Check for carry
    if( low < offset )
    {
        hi++;
        numCycles++;
    }

    address = low;
    address += hi << 4;


    return address;
}

uint16_t CPUClass::indirect( IndexingMode mode )
{
    int8_t low;
    int8_t hi;

    int16_t full = 0x0000;

    switch (mode)
    {
        case WITH_X:
            low = memory[ programCounter++ ];
            // This is supposed to just wrap
            low += X;

            full = memory[ low++ ];
            full += memory[ low ] << 4;

            return memory[ full ];
        case WITH_Y:
            low = memory[ programCounter++ ];

            full = memory[ low++ ];
            full += memory[ low ] << 4;
            full += Y;

            return memory[ full ];
        case NONE:
            full = memory[ programCounter++ ];
            full += memory[ programCounter++ ] << 4;

            low = memory[ full++ ];
            hi = memory[ full ];

            full = hi << 4;
            full += low;

            return memory[ full ];
    }
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


void CPUClass::ADC()
{
    int8_t oldAccumulator = accumulator;

    accumulator += MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// $69
void CPUClass::ADCImmediate()
{
    numCycles = 2;
    MDR = immediate();
    ADC();
}

// $ 65
void CPUClass::ADCZeroPage()
{
    numCycles = 3;
    MDR = memory[ zeroPage( NONE ) ];
    ADC();
}

// $75
void CPUClass::ADCZeroPageX()
{
    numCycles = 4;
    MDR = memory[ zeroPage( WITH_X ) ];
    ADC();
}

// $6D
void CPUClass::ADCAbsolute()
{
    numCycles = 4;
    MDR = memory[ absolute( NONE ) ];
    ADC();
}

// $7D
void CPUClass::ADCAbsoluteX()
{
    numCycles = 4;
    MDR = memory[ absolute( WITH_X ) ];
    ADC();
}

// $79
void CPUClass::ADCAbsoluteY()
{
    numCycles = 4;
    MDR = memory[ absolute( WITH_Y ) ];
    ADC();
}

// $61
void CPUClass::ADCIndirectX()
{
    numCycles = 6;
    MDR = memory[ indirect( WITH_X ) ];
    ADC();
}

// $71
void CPUClass::ADCIndirectY()
{
    numCycles = 5;
    MDR = memory[ indirect( WITH_Y ) ];
    ADC();
}
