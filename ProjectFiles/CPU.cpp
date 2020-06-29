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

    while( true )
    {
        if( PC == 0xCEFC )
        {
            // break;
            std::cout << "here" << std::endl;
        }
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
        // behavior in mind.
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
inline void CPUClass::updateNegative( int8_t reg )
{
    if( reg < 0 )
    {
        P |= SET_NEGATIVE;
    }
    else
    {
        P &= ~SET_NEGATIVE;
    }
}

inline void CPUClass::updateBreak()
{

}

inline void CPUClass::updateDecimal()
{
    // TODO: I believe this flag was actually disabled on the NES
}

inline void CPUClass::updateInterruptDisable()
{

}

inline void CPUClass::updateZero( int8_t reg )
{
    if( reg == 0 )
    {
        P |= SET_ZERO;
    }
    else
    {
        P &= ~SET_ZERO;
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
    MDR = &memory[ memory[ ++PC + offset ] ];
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
    uint16_t temp = A;

    temp += *MDR;
    // Add the carry bit
    temp += ( P & 0b00000001 );

    A = ( int8_t )temp;

    updateNegative( A );

    if( ( A < 0 && oldA > 0 && ( int8_t )*MDR > 0 ) ||
        ( A > 0 && oldA < 0 && ( int8_t )*MDR < 0 ) )
    {
        P |= SET_OVERFLOW;
    }
    else
    {
        P &= ~SET_OVERFLOW;
    }

    updateZero( A );

    if( temp > 0xFF )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
    }
}

// Subtract
void CPUClass::SBC()
{
    int8_t oldA =  A;

    A -= *MDR;
    // Add the carry
    A -= ( 1 - ( P & 0b00000001 ) );

    updateNegative( A );

    if( ( A < 0 && oldA > 0 && ( int8_t )*MDR < 0 ) ||
        ( A > 0 && oldA < 0 && ( int8_t )*MDR > 0 ) )
    {
        P |= SET_OVERFLOW;
    }
    else
    {
        P &= ~SET_OVERFLOW;
    }

    updateZero( A );

    // If we are subtracting a value larger than the value in the accumulator
    // we will need to borrow which for SBC and Compare means set carry
    ( uint8_t)oldA >= *MDR ? P |= SET_CARRY : P &= ~SET_CARRY;
}

// Bitwise and
void CPUClass::AND()
{
     A &= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Exclusive bitwise or
void CPUClass::EOR()
{
     A ^= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Bitwise or
void CPUClass::ORA()
{
     A |= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Left shift
void CPUClass::ASL()
{

    if( ( *MDR & 0b10000000 ) == 0b10000000 )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
    }

    *MDR <<= 1;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Left shift
void CPUClass::LSR()
{
    if( ( *MDR  & 0b00000001 ) == 0b00000001 )
    {
        P |= SET_CARRY;
    }
    else
    {
        P &= ~SET_CARRY;
    }

    *MDR >>= 1;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Bit test
void CPUClass::BIT()
{
    if( ( A & *MDR ) == 0b00000000 )
    {
        P |= SET_ZERO;
    }
    else
    {
        P &= ~SET_ZERO;
    }

    if( ( *MDR & 0b10000000 ) == 0b10000000 )
    {
        P |= SET_NEGATIVE;
    }
    else
    {
        P &= ~SET_NEGATIVE;
    }

    if( ( *MDR  & 0b01000000 ) == 0b01000000 )
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
void CPUClass::Compare( int8_t reg )
{
    // If we are comparing a value larger than the value in the register
    // we will need to borrow which for SBC and Compare means set carry
    ( uint8_t )reg >= *MDR ? P |= SET_CARRY : P &= ~SET_CARRY;

    reg -= *MDR;

    updateNegative( reg );
    updateZero( reg );
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

// Incrementing
void CPUClass::INC()
{
    *MDR++;

    updateNegative( *MDR );
    updateZero( *MDR );
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
    stack[ SP-- ] = hi;
    stack[ SP-- ] = lo;

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

// NOTHNG
void CPUClass::NOP()
{

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
    int8_t reg;

    bool carry = ( ( *MDR ) & 0b10000000 ) == 0b10000000;

    *MDR << 1;

    *MDR |= P & SET_CARRY;
    reg = *MDR;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( reg );
    updateZero( reg );
}

// Rotate right
void CPUClass::ROR()
{
    int8_t reg;

    bool carry = ( ( *MDR ) & 0b00000001 ) == 0b00000001;

    ( *MDR ) >> 1;

    *MDR |= ( P & SET_CARRY ) << 7;
    reg = *MDR;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( reg );
    updateZero( reg );
}

// Return from interrupt
void CPUClass::RTI()
{
    P = stack[ ++SP ];

    PC = stack[ ++SP ];
    PC += ( ( uint16_t )stack[ ++SP ] ) << 8;
    // Decrement here because we increment at the end of run
    PC--;
}

// Return from subroutine
void CPUClass::RTS()
{
    // TODO: Something is going wrong attempting to return
    PC = stack[ ++SP ];
    PC += ( ( uint16_t )stack[ ++SP ] ) << 8;
}

// Store
void CPUClass::STA()
{
    *MDR =  A;
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
    SP = X;
}

void CPUClass::TSX()
{
    X = SP;

    updateNegative( X );
    updateZero( X );
}

void CPUClass::PHA()
{
    stack[ SP-- ] =  A;
}

void CPUClass::PLA()
{
    A = stack[ ++SP ];

    updateNegative( A );
    updateZero( A );
}

void CPUClass::PHP()
{
    stack[ SP-- ] = P;
}

void CPUClass::PLP()
{
    P = stack[ ++SP  ];
}
