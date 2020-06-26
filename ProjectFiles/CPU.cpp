#include "CPU.h"
#include <iostream>
#include <fstream>

// start running here
void CPUClass::run( std::ifstream &ROMImage )
{
    std::ofstream myFile;
    myFile.open("test.txt");

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
    ROMImage.seekg( 0x10, ROMImage.beg );
    ROMImage.read( ( char* )( &memory[ 0xC000 ] ), 0x4000 );

    auto begin = std::chrono::high_resolution_clock::now();

    // for( int i = 0; i < 184; i++ )?
    while( true )
    {
        // if( PC == 0xC953 )
        // {
        //     break;
        //     std::cout << "here" << std::endl;
        // }
        myFile << std::uppercase << std::hex << PC << std::endl;
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
        // behavior in mind. I don't think we always want to inc though do we.
        // Length one opcodes
        PC++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();
    std::cout << "Duration: " << duration << std::endl;
}

/*******************************************************************************
* CPU cycle
*******************************************************************************/
inline void CPUClass::fetch()
{
    MDR = &memory[ PC ];
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
void CPUClass::updateNegative( int8_t val )
{
    if( val < 0 )
    {
        P |= SET_NEGATIVE;
    }
    else
    {
        P &= ~SET_NEGATIVE;
    }
}

void CPUClass::updateOverflow( int8_t newVal, int8_t oldval )
{
    if( ( newVal < 0 && oldval > 0 && *MDR > 0 ) ||
        ( newVal > 0 && oldval < 0 && *MDR < 0 ) )
    {
        P |= SET_OVERFLOW;
    }
    else
    {
        P &= ~SET_OVERFLOW;
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

void CPUClass::updateZero( int8_t val )
{
    if( val == 0 )
    {
        P |= SET_ZERO;
    }
    else
    {
        P &= ~SET_ZERO;
    }
}

void CPUClass::updateCarry( int8_t newVal, int8_t oldVal )
{
    if( ( newVal < oldVal && oldVal > 0 && *MDR > 0 ) ||
        ( newVal > oldVal && oldVal < 0 && *MDR > 0 ) )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
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

    uint8_t lo = memory[ ++PC ];
    uint8_t hi = memory[ ++PC ];

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
    MDR = &memory[ address ];
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
            lo = memory[ ++PC ];
            // This is supposed to just wrap
            lo += X;

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;

        case USE_Y:
            lo = memory[ ++PC ];

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;
            address += Y;

        case NONE:
            address = memory[ ++PC ];
            address += memory[ ++PC ] << 8;

            lo = memory[ address++ ];
            hi = memory[ address ];

            address = hi << 8;
            address += lo;
    }

    MDR = &memory[ address ];
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
    MDR = &memory[ ++PC + offset ];
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

// This is kind of a formality so there is an entry into the addressmode decode
// table for immediate opcodes
void CPUClass::imm()
{
    MDR = &memory[ ++PC ];
}

void CPUClass::rel()
{
    MDR = &memory[ ++PC ];
}

void CPUClass::imp()
{

}

void CPUClass::acc()
{
    MDR = ( uint8_t* )&A;
}

void CPUClass::non()
{

}

// Adding opcodes
void CPUClass::ADC()
{
    int8_t oldA =  A;

    A += *MDR;
    // Add the carry bit
    // TODO: If we add this carry bit are we guaranteed to come out with a clear
    // carry?
    A += ( P & 0b00000001 );

    updateNegative( A );
    updateOverflow( A, oldA );
    updateZero( A );
    updateCarry( A, oldA );
}

// Bitwise anding
void CPUClass::AND()
{
     A &= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Left shift
void CPUClass::ASL()
{
    int8_t val;

    if( ( *MDR & 0b10000000 ) == 0b10000000 )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
    }

    *MDR << 1;
    val = *MDR;

    updateNegative( val );
    updateZero( val );
}

// Bit test
void CPUClass::BIT()
{
    if( ( A & memory[ *MDR ] ) == 0b00000000 )
    {
        P |= SET_ZERO;
    }
    else
    {
        P &= ~SET_ZERO;
    }

    if( ( memory[ *MDR ] & 0b10000000 ) == 0b10000000 )
    {
        P |= SET_NEGATIVE;
    }
    else
    {
        P &= ~SET_NEGATIVE;
    }

    if( ( memory [ *MDR ] & 0b01000000 ) == 0b01000000 )
    {
        P |= SET_OVERFLOW;
    }
    else
    {
        P &= ~SET_OVERFLOW;
    }
}

// Branching
void CPUClass::Branch()
{
    // We need to caste the branch offset to a signed int
    PC += ( int8_t )*MDR;
}

void CPUClass::BPL()
{
    if( ( P & 0b10000000 ) == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BMI()
{
    if( ( P & 0b10000000 ) == 0b10000000 )
    {
        Branch();
    }
}

void CPUClass::BVC()
{
    if( ( P & 0b01000000 ) == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BVS()
{
    if( ( P & 0b01000000 ) == 0b01000000 )
    {
        Branch();
    }
}

void CPUClass::BCC()
{
    if( ( P & 0b00000001 ) == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BCS()
{
    if( ( P & 0b00000001 ) == 0b00000001 )
    {
        Branch();
    }
}

void CPUClass::BNE()
{
    if( ( P & 0b00000010 ) == 0b00000000 )
    {
        Branch();
    }
}

void CPUClass::BEQ()
{
    if( ( P & 0b00000010 ) == 0b00000010 )
    {
        Branch();
    }
}

// Break
void CPUClass::BRK()
{

}

// Comparing
void CPUClass::Compare( int8_t val )
{
    int8_t oldVal = val;
    val -= *MDR;

    updateNegative( val );
    updateZero( val );
    updateCarry( val, oldVal );
}

void CPUClass::CMP()
{
    Compare( A );
}

void CPUClass::CPX()
{
    Compare( X );
}

void CPUClass::CPY()
{
    Compare( Y );
}

// Decrementing
void CPUClass::DEC()
{
    *MDR--;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Exclusive bitwise or
void CPUClass::EOR()
{
     A ^= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Flag Setting
void CPUClass::CLC()
{
    P &= ~SET_CARRY;
}

void CPUClass::SEC()
{
    P |= SET_CARRY;
}

void CPUClass::CLI()
{
    P &= ~SET_INTERRUPT_DISABLE;
}

void CPUClass::SEI()
{
    P |= SET_INTERRUPT_DISABLE;
}

void CPUClass::CLV()
{
    P &= ~SET_OVERFLOW;
}

/*
* NOTE: Based on what I've read these don't work on the NES. Nintendo disabled
* Binary Coded Decimal mode so they wouldn't have to pay licensing fees based on
* what I've read. So I suppose they would probably work and set the flag, it
* just wouldn't do anythin
*/
void CPUClass::CLD()
{
    P &= ~SET_DECIMAL;
}

void CPUClass::SED()
{
    P |= SET_DECIMAL;
}

// Incrementing
void CPUClass::INC()
{
    *MDR++;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Jumping
void CPUClass::JMP()
{
    // Note: -1 because we increment the PC at the end of run
    PC = MDR - memory - 1;
}

void CPUClass::JSR()
{
    uint8_t lo = PC & 0b0000000011111111;
    uint8_t hi = PC >> 8;

    // hi then lo so lo comes off first
    memory[ SP-- ] = hi;
    memory[ SP-- ] = lo;

    // Note: -1 because we increment the PC at the end of run
    PC = MDR - memory - 1;
}

// Loading
void CPUClass::LDA()
{
     A = *MDR;

    updateNegative( A );
    updateZero( A );
}

void CPUClass::LDX()
{
    X = *MDR;

    updateNegative( X );
    updateZero( X );
}

void CPUClass::LDY()
{
    Y = *MDR;

    updateNegative( Y );
    updateZero( Y );
}

// Left shift
void CPUClass::LSR()
{
    int8_t val;

    if( ( ( *MDR ) & 0b00000001 ) == 0b00000001 )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
    }

    *MDR >> 1;
    val = *MDR;

    updateNegative( val );
    updateZero( val );
}

// NOTHNG
void CPUClass::NOP()
{

}

// Bitwise or
void CPUClass::ORA()
{
     A |= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Register instructions
void CPUClass::TAX()
{
    X =  A;

    updateNegative( X );
    updateZero( X );
}

void CPUClass::TXA()
{
     A = X;

    updateNegative( A );
    updateZero( A );
}

void CPUClass::DEX()
{
    X--;

    updateNegative( X );
    updateZero( X );
}

void CPUClass::INX()
{
    X++;

    updateNegative( X );
    updateZero( X );
}

void CPUClass::TAY()
{
    Y =  A;

    updateNegative( Y );
    updateZero( Y );
}

void CPUClass::TYA()
{
     A = Y;

    updateNegative( Y );
    updateZero( Y );
}

void CPUClass::DEY()
{
    Y--;

    updateNegative( Y );
    updateZero( Y );
}

void CPUClass::INY()
{
    Y++;

    updateNegative( Y );
    updateZero( Y );
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
    int8_t val;

    bool carry = ( ( *MDR ) & 0b10000000 ) == 0b10000000;

    *MDR << 1;

    *MDR |= P & SET_CARRY;
    val = *MDR;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( val );
    updateZero( val );
}

// Rotate right
void CPUClass::ROR()
{
    int8_t val;

    bool carry = ( ( *MDR ) & 0b00000001 ) == 0b00000001;

    ( *MDR ) >> 1;

    *MDR |= ( P & SET_CARRY ) << 7;
    val = *MDR;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( val );
    updateZero( val );
}

// Return from interrupt
void CPUClass::RTI()
{
    P = memory[ ++SP ];

    PC = memory[ ++SP ];
    PC += ( ( uint16_t )memory[ ++SP ] ) << 8;
}

// Return from subroutine
void CPUClass::RTS()
{
    // TODO: Something is going wrong attempting to return
    PC = memory[ ++SP ];
    PC += ( ( uint16_t )memory[ ++SP ] ) << 8;
}

// Subtract
void CPUClass::SBC()
{
    int8_t oldA =  A;

     A -= *MDR;

    updateNegative( A );
    updateOverflow( A, oldA );
    updateZero( A );
    updateCarry( A, oldA );
}

// Store
void CPUClass::STA()
{
    memory[ *MDR ] =  A;
}

void CPUClass::STX()
{
    memory[ *MDR ] = X;
}

void CPUClass::STY()
{
    memory[ *MDR ] = Y;
}

// Not 100% sure what the difference is between this and NOP
void CPUClass::STP()
{

}

// Stack instructions
void CPUClass::TXS()
{
    memory[ SP--] = X;
}

void CPUClass::TSX()
{
    X = memory[ ++SP ];

    updateNegative( X );
    updateZero( X );
}

void CPUClass::PHA()
{
    memory[ SP-- ] =  A;
}

void CPUClass::PLA()
{
     A = memory[ ++SP ];

    updateNegative( A );
    updateZero( A );
}

void CPUClass::PHP()
{
    memory[ SP-- ] = P;
}

void CPUClass::PLP()
{
    P = memory[ ++SP ];
}
