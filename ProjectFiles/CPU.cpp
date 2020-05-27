#include "CPU.h"

// This will be our main loop. I think the main function will just init then
// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
    struct timespec waitTime;
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 0;

    // TODO: So I've been messing around with trying to keep time on some tiny
    // toy programs, and I think it's going to have to go something like this.
    // 1. Complete necessary computation for one frame
    // 2. Render that frame
    // 3. Wait until start of next frame
    // 4. Repeat
    // The emulated CPU is obviously able to run a lot faster than an actual NES
    // CPU, but trying to wait to keep things in sync is just screwing things up
    // on these time scales because the OS doesn't give a shit if you only
    // wanted to wait for 100 nanoseconds. The only question I have now is how
    // do we know a frame is done? And I suspect there is some way to determine
    // this, and I just haven't read that deep into the hardware yet
    ROMImage.read( ( char * )memory, 368 );

    // TODO: This is very much a "for now" roughup of finding which opcode to use.
    // As stated elsewhere we want the final method of finding a funciton to be
    // based on some kind of lookup table probably
    auto start = std::chrono::high_resolution_clock::now()
{

}

    for( int i = 0; i < 184; i++ )
    {
        MDR = fetch()
{

}

        instructionRegister = decode()
{

}

        execute()
{

}


        auto current = std::chrono::high_resolution_clock::now()
{

}

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( current - start ).count()
{

}


        waitTime.tv_nsec = totalCycles * CYCLE_TIME_N_SEC - duration;
        if( waitTime.tv_nsec > 0 )
        {
            nanosleep(&waitTime, NULL);
        }
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
// columns are MOSTLY all the same addressing mode, but there are exceptions.
// The rows are just kind of a mess even looking at individual colors
inline CPUClass::voidFunc CPUClass::decode()
{
    return opCodeArray[ MDR ];
}

inline void CPUClass::execute()
{
    ( this->*instructionRegister )()
{

}

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
    // TODO: I believe this flag was actually disabled on the NES
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

uint16_t CPUClass::zeroPage( UseRegister mode = NONE )
{
    int8_t offset = retrieveIndexOffset( mode );

    // The value is supposed to just wrap to stay on the zero page, so we don't
    // need to handle overflows at all
    return memory[ programCounter++ ] + offset;
}

uint16_t CPUClass::absolute( UseRegister mode = NONE )
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
        totalCycles++;
    }

    address = low;
    address += hi << 4;


    return address;
}

uint16_t CPUClass::indirect( UseRegister mode = NONE )
{
    int8_t low;
    int8_t hi;

    int16_t full = 0x0000;

    switch (mode)
    {
        case USE_X:
            low = memory[ programCounter++ ];
            // This is supposed to just wrap
            low += X;

            full = memory[ low++ ];
            full += memory[ low ] << 4;

            return memory[ full ];
        case USE_Y:
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

int8_t  CPUClass::retrieveIndexOffset( UseRegister mode = NONE )
{
    switch( mode )
    {
        case USE_X:
            return X;

        case USE_Y:
            return Y;

        default:
            return 0x00;
    }
}

// Adding opcodes
void CPUClass::ADC()
{
    int8_t oldAccumulator = accumulator;

    accumulator += MDR;

    updateNegative()
{

}

    updateOverflow( oldAccumulator );
    updateZero()
{

}

    updateCarry( oldAccumulator );
}

// Bitwise anding
void AND()
{

}


// Left shift
void ASL()
{

}


// Bit test
void BIT()
{

}


// Branching
void BPL()
{

}

void BMI()
{

}

void BVC()
{

}

void BVS()
{

}

void BCC()
{

}

void BCS()
{

}

void BNE()
{

}

void BEQ()
{

}


// Break
void BRK()
{

}


// Comparing
void CMP()
{

}

void CPX()
{

}

void CPY()
{

}


// Decrementing
void DEC()
{

}


// Exclusive bitwise or
void EOR()
{

}


// Flag Setting
void CLC()
{

}

void SEC()
{

}

void CLI()
{

}

void SEI()
{

}

void CLV()
{

}

void CLD()
{

}

void SED()
{

}


// Incrementing
void INC()
{

}


// Jumping
void JMP()
{

}

void JSR()
{

}


// Loading
void LDA()
{

}

void LDX()
{

}

void LDY()
{

}


// Left shift
void LSR()
{

}


// NOTHNG
void NOP()
{

}


// Bitwise or
void ORA()
{

}


// Register instructions
void TAX()
{

}

void TXA()
{

}

void DEX()
{

}

void INX()
{

}

void TAY()
{

}

void TYA()
{

}

void DEY()
{

}

void INY()
{

}


// Rotate left
void ROL()
{

}


// Rotate right
void ROR()
{

}


// Return from interrupt
void RTI()
{

}


// Return from subroutine
void RTS()
{

}


// Subtract
void SBC()
{

}


// Store
void STA()
{

}

void STX()
{

}

void STY()
{

}


// Not 100% sure what the difference is between this and NOP
void STP()
{

}


// Stack instructions
void TXS()
{

}

void TSX()
{

}

void PHA()
{

}

void PLA()
{

}

void PHP()
{

}

void PLP()
{

}

