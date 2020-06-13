#include "CPU.h"

// TODO: I need to start thinking very seriously about how the program counter
// and cycle counts are to be managed. I'm not sure my previous solutions are as
// good as I previously thought

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
    // TODO: How to I get this to read into memory where I want it
    ROMImage.seekg( 0x10, ROMImage.beg );
    ROMImage.read( ( char* )memory, 0x6000 );

    for( int i = 0; i < 184; i++ )
    {
        // auto begin = std::chrono::high_resolution_clock::now()
        fetch();
        // Decoding needs to be split into two stages
        decodeAddr();
        decodeOP();
        execute();

        // The PC is incremented here to more closely reflect the behavior of
        // the hardware. According to http://www.6502.org/tutorials/6502opcodes.html#PC
        // the PC should be on the last operand of the prior opcode until it is
        // ready for the next opcode. We can't increment it at the top of the
        // loop or we increment off of the first opcode, so we do it here. This
        // helps with JSR and RTS behavior as it is dependent on pushing and
        // popping the address of the last operand of the previous opcode. To do
        // this differently would break jump tables that are designed with that
        // behavior in mind
        programCounter++;
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

    uint8_t lo = memory[ programCounter++ ];
    uint8_t hi = memory[ programCounter ];

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

    lo += offset;

    // Check for carry
    if( lo < offset )
    {
        hi++;
        totalCycles++;
    }

    address = lo;
    address += hi << 8;

    // TODO: This causes issues for jumping instructions. This returns a pointer
    // to the value stored in that location. We want to jump to that location
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
    int8_t lo;
    int8_t hi;

    uint16_t address;

    switch( mode )
    {
        case USE_X:
            lo = memory[ programCounter ];
            // This is supposed to just wrap
            lo += X;

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;

        case USE_Y:
            lo = memory[ programCounter ];

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;
            address += Y;

        case NONE:
            address = memory[ programCounter++ ];
            address += memory[ programCounter ] << 8;

            lo = memory[ address++ ];
            hi = memory[ address ];

            address = hi << 8;
            address += lo;
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
    // need to handle overflows at all (if it didn't wrap it would hit the
    // stack)
    MDR = &( memory[ memory[ programCounter ] + offset ] );
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
    MDR = &memory[ programCounter ];
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
    // This could be turned into a ternary, but I think it would be a gross one
    if( ( ( *MDR ) & 0b10000000 ) == 0b10000000 )
    {
        status |= SET_CARRY;
    }
    else
    {
        status &= ~SET_CARRY;
    }

    ( *MDR ) << 1;

    updateNegative();
    updateZero();
}

// Bit test
void CPUClass::BIT()
{
    if( accumulator & *MDR == 0b00000000 )
    {
        status |= SET_ZERO;
    }
    else
    {
        status &= ~SET_ZERO;
    }

    if( *MDR & 0b10000000 == 0b10000000 )
    {
        status |= SET_NEGATIVE;
    }

    if( *MDR & 0b01000000 == 0b01000000 )
    {
        status |= SET_OVERFLOW;
    }
}

// Branching
void CPUClass::Branch()
{
    // We need to caste the branch offset to a signed int
    int8_t offset = ( int8_t )*MDR;

    programCounter += offset;
}

void CPUClass::BPL()
{
    if( status & 0b10000000 == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BMI()
{
    if( status & 0b10000000 == 0b10000000 )
    {
        Branch();
    }
}

void CPUClass::BVC()
{
    if( status & 0b01000000 == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BVS()
{
    if( status & 0b01000000 == 0b01000000 )
    {
        Branch();
    }
}

void CPUClass::BCC()
{
    if( status & 0b00000001 == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BCS()
{
    if( status & 0b00000001 == 0b00000001 )
    {
        Branch();
    }
}

void CPUClass::BNE()
{
    if( status & 0b00000010 == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BEQ()
{
    if( status & 0b00000010 == 0b00000010 )
    {
        Branch();
    }
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
    ( *MDR )--;

    updateNegative();
    updateZero();
}

// Exclusive bitwise or
void CPUClass::EOR()
{
    accumulator ^= *MDR;

    updateNegative();
    updateZero();
}

// Flag Setting
void CPUClass::CLC()
{
    status &= ~SET_CARRY;
}

void CPUClass::SEC()
{
    status |= SET_CARRY;
}

void CPUClass::CLI()
{
    status &= ~SET_INTERRUPT_DISABLE;
}

void CPUClass::SEI()
{
    status |= SET_INTERRUPT_DISABLE;
}

void CPUClass::CLV()
{
    status &= ~SET_OVERFLOW;
}

/*
* NOTE: Based on what I've read these don't work on the NES. Nintendo disabled
* Binary Coded Decimal mode so they wouldn't have to pay licensing fees based on
* what I've read. So I suppose they would probably work and set the flag, it
* just wouldn't do anythin
*/
void CPUClass::CLD()
{
    status &= ~SET_DECIMAL;
}

void CPUClass::SED()
{
    status |= SET_DECIMAL;
}

// Incrementing
void CPUClass::INC()
{
    ( *MDR )++;

    updateNegative();
    updateZero();
}

// Jumping
void CPUClass::JMP()
{
    programCounter = *MDR;
}

/*
* NOTE: On the real NES JSR pushes the address of the last byte of the JSR to
* the stack. In other words, because JSR uses the Absolute addressing mode it
* pushes the address of its own second operand to the stack. This is treated in
* documentation as the address of the next operation - 1.
*
* In my code, I've already incremented the program counter past the end of the
* JSR and onto the next operation at this point. For the sake of consistency
* with the hardware I push programCounter - 1 to the stack.
*
* I have no idea why this address is pushed to the stack on a JSR instead of the
* actual address of the next operation. Especially given the fact that
* interrupts do push the address of the next operation. Seriously, look at this
* source.
*
* http://www.6502.org/tutorials/6502opcodes.html#RTS
*
* They talk about JSR pushing the address of the next operation - 1 while
* they say interrupts just push the program counter. So when a JSR is executed
* is it pushing the program counter which hasn't been incremented past its last
* operand yet for some reason? Is it even more bafflingly pushing the program
* counter - 1? Why is it pushing the address - 1 and where is it getting that
* from? I can only imagine it's from a yet to be incrememented program counter
* but like... Why hasn't it been incremented yet? This has turned into quite
* the rant for a simple note. Maybe I just need to modify my code so it
* increments the PC before retrieving the next opcode and not after? But I'm not
* doing that right now because then surely it moves you off the first desired
* opcode? Ah well. I'm sure there's a reason they push the address - 1. I just
* wish this source addressed it. I can't find any that really do.
*/
void CPUClass::JSR()
{
    uint8_t lo = ( uint8_t )programCounter & 0b00001111;
    uint8_t hi = ( uint8_t )programCounter >> 4;

    // hi then lo so lo comes off first
    memory[ stackPointer++ ] = hi;
    memory[ stackPointer++ ] = lo;

    programCounter = *MDR;
}

// Loading
void CPUClass::LDA()
{
    accumulator = *MDR;
}

void CPUClass::LDX()
{
    X = *MDR;
}

void CPUClass::LDY()
{
    Y = *MDR;
}

// Left shift
void CPUClass::LSR()
{
    // This could be turned into a ternary, but I think it would be a gross one
    if( ( ( *MDR ) & 0b00000001 ) == 0b00000001 )
    {
        status |= SET_CARRY;
    }
    else
    {
        status &= ~SET_CARRY;
    }

    ( *MDR ) >> 1;

    updateNegative();
    updateZero();
}

// NOTHNG
void CPUClass::NOP()
{

}

// Bitwise or
void CPUClass::ORA()
{
    accumulator |= *MDR;

    updateNegative();
    updateZero();
}

// Register instructions
void CPUClass::TAX()
{
    X = accumulator;
}

void CPUClass::TXA()
{
    accumulator = X;
}

void CPUClass::DEX()
{
    X--;
}

void CPUClass::INX()
{
    X++;
}

void CPUClass::TAY()
{
    Y = accumulator;
}

void CPUClass::TYA()
{
    accumulator = Y;
}

void CPUClass::DEY()
{
    Y--;
}

void CPUClass::INY()
{
    Y++;
}

/*
* NOTE: The NES's CPU performs rotations using the carry. For instance:
*
* If we perform a rotate left on the value 0b10000000 with the carry not set
* we will get 0b00000000 with a set carry. The 1 got rotated off the left into
* the carry, not bit 0, and the carry got rotated into bit 0.
*/
// Rotate left
void CPUClass::ROL()
{
    // If the high bit is set it needs to be shifted into the carry later
    bool carry = ( ( *MDR ) & 0b10000000 ) == 0b10000000;

    ( *MDR ) << 1;

    // The carry needs to be moved into the low bit
    *MDR |= status & SET_CARRY;

    if( carry )
    {
        status |= SET_CARRY;
    }

    updateNegative();
    updateZero();
}

// Rotate right
void CPUClass::ROR()
{
    // If the low bit is set it needs to be shifted into the carry later
    bool carry = ( ( *MDR ) & 0b00000001 ) == 0b00000001;

    ( *MDR ) >> 1;

    // The carry needs to be moved into the high bit
    *MDR |= ( status & SET_CARRY ) << 7;

    if( carry )
    {
        status |= SET_CARRY;
    }

    updateNegative();
    updateZero();
}

// Return from interrupt
void CPUClass::RTI()
{
    status = memory[ --stackPointer ];

    programCounter = memory[ --stackPointer ];
    programCounter += ( ( uint16_t )memory[ --stackPointer ] ) << 8;
}

// Return from subroutine
void CPUClass::RTS()
{
    programCounter = memory[ --stackPointer ];
    programCounter += ( ( uint16_t )memory[ --stackPointer ] ) << 8;

    programCounter++;
}

// Subtract
void CPUClass::SBC()
{
    int8_t oldAccumulator = accumulator;

    accumulator -= *MDR;

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

// Store
void CPUClass::STA()
{
    *MDR = accumulator;
}

void CPUClass::STX()
{
    *MDR = X;
}

void CPUClass::STY()
{
    *MDR = Y;
}

// Not 100% sure what the difference is between this and NOP
void CPUClass::STP()
{

}

// Stack instructions
void CPUClass::TXS()
{
    memory[ stackPointer++ ] = X;
}

void CPUClass::TSX()
{
    X = memory[ --stackPointer ];
}

void CPUClass::PHA()
{
    memory[ stackPointer++ ] = accumulator;
}

void CPUClass::PLA()
{
    accumulator = memory[ --stackPointer ];
}

void CPUClass::PHP()
{
    memory[ stackPointer++ ] = status;
}

void CPUClass::PLP()
{
    status = memory[ --stackPointer ];
}
