#include "CPU.h"
#include <iostream>

// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
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

    for( int i = 0; i < 184; i++ )
    {
        // auto begin = std::chrono::high_resolution_clock::now()
        fetch();
        // Decoding needs to be split into two stages
        decodeAddr();
        decodeOP();
        execute();
    }
}

/*******************************************************************************
* CPU cycle
*******************************************************************************/
inline void CPUClass::fetch()
{
    MDR = &( memory[ programCounter++ ] );
}

inline void CPUClass::decodeAddr()
{
    addressMode = memoryAccessArray[ *MDR ];
}

inline void CPUClass::decodeOP()
{
    instruction = opCodeArray[ *MDR ];
}

inline void CPUClass::execute()
{
    ( this->*addressMode )();
    ( this->*instruction )();
}

/*******************************************************************************
* Status handlers
*******************************************************************************/
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

/*******************************************************************************
* Handle addressing mode operand resolution
*******************************************************************************/
// Absolute
void CPUClass::absolute( UseRegister mode = NONE )
{
    uint8_t offset;
    uint16_t address;

    uint8_t low = memory[ programCounter++ ];
    uint8_t hi = memory[ programCounter++ ];

    totalCycles += 4;
    cyclesRemaining -= 4;

    switch( mode )
    {
        case USE_X:
            offset = X;
            break;

        case USE_Y:
            offset += Y;
            break;

        default:
            offset = 0;
    }

    low += offset;

    // Check for carry
    if( low < offset )
    {
        hi++;
        totalCycles++;
    }

    address = low;
    address += hi << 4;

    MDR = &( memory[ address ] );
}

void CPUClass::abs()
{
    absolute();
}

void CPUClass::abX()
{
    absolute( USE_X );
}

void CPUClass::abY()
{
    absolute( USE_Y );
}

// Indirect
void CPUClass::indirect( UseRegister mode = NONE )
{
    int8_t low;
    int8_t hi;

    uint16_t address;

    switch( mode )
    {
        case USE_X:
            low = memory[ programCounter++ ];
            // This is supposed to just wrap
            low += X;

            address = memory[ low++ ];
            address += memory[ low ] << 4;

        case USE_Y:
            low = memory[ programCounter++ ];

            address = memory[ low++ ];
            address += memory[ low ] << 4;
            address += Y;

        case NONE:
            address = memory[ programCounter++ ];
            address += memory[ programCounter++ ] << 4;

            low = memory[ address++ ];
            hi = memory[ address ];

            address = hi << 4;
            address += low;
    }

    MDR = &( memory[ address ] );
}

void CPUClass::ind()
{
    indirect();
}

void CPUClass::inX()
{
    indirect( USE_X );
}

void CPUClass::inY()
{
    indirect( USE_Y );
}

// Zero Page
void CPUClass::zeroPage( UseRegister mode = NONE )
{
    int8_t offset;

    switch( mode )
    {
        case USE_X:
            offset = X;
            break;

        case USE_Y:
            offset = Y;
            break;

        default:
            offset = 0;
    }

    // The value is supposed to just wrap to stay on the zero page, so we don't
    // need to handle overflows at all
    MDR = &( memory[ memory[ programCounter++ ] + offset ] );
}

void CPUClass::zer()
{
    zeroPage();
}

void CPUClass::zeX()
{
    zeroPage( USE_X );
}

void CPUClass::zeY()
{
    zeroPage( USE_Y );
}

// TODO: INVESTIDATE RETURN TYPES OF MEMORY RESOLVERS
// This is kind of a formality so there is an entry into the addressmode decode
// table for immediate opcodes
void CPUClass::imm()
{
    MDR = &memory[ programCounter++ ];
}

void CPUClass::rel()
{

}

void CPUClass::imp()
{

}

void CPUClass::acc()
{
    // The operations that use the accumulator as an operand don't care if it's
    // signed anyway
    MDR = ( uint8_t* )&accumulator;
}

void CPUClass::non()
{

}

// Adding opcodes
void CPUClass::ADC()
{
    int8_t oldAccumulator = accumulator;

    accumulator += *MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// Bitwise anding
void CPUClass::AND()
{
    accumulator &= *MDR;

    updateNegative();
    updateZero();
}

// Left shift
void CPUClass::ASL()
{
    if( ( *MDR & 0x1000000 ) == 0x10000000 )
    {
        status |= SET_CARRY;
    }
    else
    {
        status &= ~SET_CARRY;
    }

    *MDR << 1;

    updateNegative();
    updateZero();
}

// Bit test
void CPUClass::BIT()
{

}

// Branching
void CPUClass::BPL()
{

}

void CPUClass::BMI()
{

}

void CPUClass::BVC()
{

}

void CPUClass::BVS()
{

}

void CPUClass::BCC()
{

}

void CPUClass::BCS()
{

}

void CPUClass::BNE()
{

}

void CPUClass::BEQ()
{

}

// Break
void CPUClass::BRK()
{

}

// Comparing
void CPUClass::CMP()
{

}

void CPUClass::CPX()
{

}

void CPUClass::CPY()
{

}

// Decrementing
void CPUClass::DEC()
{

}

// Exclusive bitwise or
void CPUClass::EOR()
{

}

// Flag Setting
void CPUClass::CLC()
{

}

void CPUClass::SEC()
{

}

void CPUClass::CLI()
{

}

void CPUClass::SEI()
{

}

void CPUClass::CLV()
{

}

void CPUClass::CLD()
{

}

void CPUClass::SED()
{

}

// Incrementing
void CPUClass::INC()
{

}

// Jumping
void CPUClass::JMP()
{

}

void CPUClass::JSR()
{

}

// Loading
void CPUClass::LDA()
{

}

void CPUClass::LDX()
{

}

void CPUClass::LDY()
{

}

// Left shift
void CPUClass::LSR()
{

}

// NOTHNG
void CPUClass::NOP()
{

}

// Bitwise or
void CPUClass::ORA()
{

}

// Register instructions
void CPUClass::TAX()
{

}

void CPUClass::TXA()
{

}

void CPUClass::DEX()
{

}

void CPUClass::INX()
{

}

void CPUClass::TAY()
{

}

void CPUClass::TYA()
{

}

void CPUClass::DEY()
{

}

void CPUClass::INY()
{

}

// Rotate left
void CPUClass::ROL()
{

}

// Rotate right
void CPUClass::ROR()
{

}

// Return from interrupt
void CPUClass::RTI()
{

}

// Return from subroutine
void CPUClass::RTS()
{

}

// Subtract
void CPUClass::SBC()
{

}

// Store
void CPUClass::STA()
{

}

void CPUClass::STX()
{

}

void CPUClass::STY()
{

}

// Not 100% sure what the difference is between this and NOP
void CPUClass::STP()
{

}

// Stack instructions
void CPUClass::TXS()
{

}

void CPUClass::TSX()
{

}

void CPUClass::PHA()
{

}

void CPUClass::PLA()
{

}

void CPUClass::PHP()
{

}

void CPUClass::PLP()
{

}
