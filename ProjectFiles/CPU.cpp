#include "CPU.h"
#include <iostream>
#include <fstream>
#include <vector>

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

    std::ifstream log( "/home/anthony/src/NES/NESEmu/fullnestest.log" );
    std::vector<int> cycNumbers;

    std::string line;

    while( getline( log, line ) )
    {
        cycNumbers.push_back( std::stoi( line.substr( line.rfind( ":" ) + 1 ) ) );
    }

    log.close();

    auto begin = std::chrono::high_resolution_clock::now();

    int cycLine = 0;
    while( true )
    {
        if( totalCycles != cycNumbers[ cycLine ] )
        {
            break;
        }

        currentCycles = 0;

        if( PC < 0x1000 )
        {
            myFile << "0";
        }
        myFile << std::uppercase << std::hex << PC << std::endl;

        fetch();
        decodeAddr();
        decodeOP();
        execute();

        totalCycles += currentCycles;
        cyclesRemaining -= currentCycles;

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
        cycLine++;
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

    switch (*MDR)
    {
        case BRK_IMP:
            imp();
            BRK();
            /* code */
            break;

        case ORA_INX:
            inX();
            ORA();

        case STP_NON:
            non();
            STP();

        case SLO_INX:
            // not implemented

        case NOP_ZER:
            zer();
            NOP();

        default:
            break;
    }
}

/*******************************************************************************
* Status handlers
*******************************************************************************/
inline void CPUClass::updateNegative( int8_t reg )
{
    reg < 0 ? P |= SET_NEGATIVE : P &= ~SET_NEGATIVE;
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
    reg == 0 ? P |= SET_ZERO : P &= ~SET_ZERO;
}

/*******************************************************************************
* Handle addressing mode operand resolution
*******************************************************************************/
inline void CPUClass::abs(  UseRegister mode = NONE  )
{
    uint8_t offset;
    uint16_t address;

    uint8_t lo = memory[ ++PC ];
    uint8_t hi = memory[ ++PC ];

    instruction == &CPUClass::JMP ? currentCycles += 3 : currentCycles += 4;

    switch( mode )
    {
        case USE_X:
            if( instruction == &CPUClass::ASL || instruction == &CPUClass::LSR ||
                instruction == &CPUClass::ROL || instruction == &CPUClass::ROR )
            {
                currentCycles++;
            }

            offset = X;
            break;

        case USE_Y:
            offset = Y;
            break;

        default:
            offset = 0;
    }

    lo += offset;

    // Check for carry
    if( lo < offset)
    {
        hi++;
        if( !( instruction == &CPUClass::ASL || instruction == &CPUClass::LSR ||
               instruction == &CPUClass::ROL || instruction == &CPUClass::ROR ) )
        {
            currentCycles++;
        }

    }


    address = lo;
    address += hi << 8;
    // TODO: This causes issues for jumping instructions. This returns a pointer
    // to the value stored in that location. We want to jump to that location
    MDR = &memory[ address ];
}

inline void CPUClass::abX()
{
    abs( USE_X );
}

inline void CPUClass::abY()
{
    abs( USE_Y );
}

// Indirect
inline void CPUClass::ind( UseRegister mode = NONE )
{
    uint8_t lo;
    uint8_t hi;

    uint16_t address;

    switch( mode )
    {
        case USE_X:
            currentCycles += 6;

            lo = memory[ ++PC ];
            // This is supposed to just wrap
            lo += ( uint8_t )X;

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;

            break;
        case USE_Y:
            currentCycles += 5;

            lo = memory[ ++PC ];

            address = memory[ lo++ ];
            address += memory[ lo ] << 8;
            address += ( uint8_t )Y;

            if( address << 8 >> 8 > ( uint8_t )Y || instruction == &CPUClass::STA )
            {
                currentCycles++;
            }

            break;
        case NONE:
            currentCycles += 5;

            lo = memory[ ++PC ];
            hi = memory[ ++PC ];

            // We want the low byte to wrap not carry
            address = memory[ lo++ + ( hi << 8 ) ];
            address += memory[ lo + ( hi << 8 ) ] << 8;
    }

    MDR = &memory[ address ];
}

inline void CPUClass::inX()
{
    ind( USE_X );
}

inline void CPUClass::inY()
{
    ind( USE_Y );
}

// Zero Page
inline void CPUClass::zer( UseRegister mode = NONE )
{
    uint8_t address = memory[ ++PC ];

    currentCycles += 3;

    switch( mode )
    {
        case USE_X:
            currentCycles++;

            address += ( uint8_t )X;
            break;

        case USE_Y:
            currentCycles++;

            address += ( uint8_t )Y;
            break;
    }

    // The value is supposed to just wrap to stay on the zero page, so we don't
    // need to handle overflows at all (if it didn't wrap it would hit the
    // stack)
    MDR = &memory[ address ];
}

inline void CPUClass::zeX()
{
    zer( USE_X );
}

inline void CPUClass::zeY()
{
    zer( USE_Y );
}

// This is kind of a formality so there is an entry into the addressmode decode
// table for immediate opcodes
inline void CPUClass::imm()
{
    currentCycles += 2;

    MDR = &memory[ ++PC ];
}

inline void CPUClass::rel()
{
    currentCycles += 2;

    MDR = &memory[ ++PC ];
}

inline void CPUClass::imp()
{
    currentCycles += 2;
}

inline void CPUClass::acc()
{
    MDR = ( uint8_t* )&A;
}

inline void CPUClass::non()
{

}

// Add
inline void CPUClass::ADC()
{
    int8_t oldA =  A;
    // To check for overflow
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

    // If our addition went over FF we carried from bit 7
    temp > 0xFF ? P |= SET_CARRY : P &= ~SET_CARRY;
}

// Subtract
inline void CPUClass::SBC()
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
inline void CPUClass::AND()
{
     A &= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Exclusive bitwise or
inline void CPUClass::EOR()
{
     A ^= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Bitwise or
inline void CPUClass::ORA()
{
     A |= *MDR;

    updateNegative( A );
    updateZero( A );
}

// Left shift
inline void CPUClass::ASL()
{
    currentCycles += 2;

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
inline void CPUClass::LSR()
{
    currentCycles += 2;

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
inline void CPUClass::BIT()
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
inline void CPUClass::Branch()
{
    uint16_t oldPC = PC;

    currentCycles++;

    // We need to caste the branch offset to a signed int
    PC += ( int8_t )*MDR;

    // if( oldPC >> 8 != PC >> 8 )
    // {
    //     currentCycles++;
    // }
}

inline void CPUClass::BPL()
{
    if( ( P & 0b10000000 ) == 0b00000000 )
    {
        Branch();
    }
}

inline void CPUClass::BMI()
{
    if( ( P & 0b10000000 ) == 0b10000000 )
    {
        Branch();
    }
}

inline void CPUClass::BVC()
{
    if( ( P & 0b01000000 ) == 0b00000000 )
    {
        Branch();
    }
}

inline void CPUClass::BVS()
{
    if( ( P & 0b01000000 ) == 0b01000000 )
    {
        Branch();
    }
}

inline void CPUClass::BCC()
{
    if( ( P & 0b00000001 ) == 0b00000000 )
    {
        Branch();
    }
}

inline void CPUClass::BCS()
{
    if( ( P & 0b00000001 ) == 0b00000001 )
    {
        Branch();
    }
}

inline void CPUClass::BNE()
{
    if( ( P & 0b00000010 ) == 0b00000000 )
    {
        Branch();
    }
}

inline void CPUClass::BEQ()
{
    if( ( P & 0b00000010 ) == 0b00000010 )
    {
        Branch();
    }
}

// Break
inline void CPUClass::BRK()
{
    totalCycles += 5;
}

// Comparing
inline void CPUClass::Compare( int8_t reg )
{
    // If we are comparing a value larger than the value in the register
    // we will need to borrow which for SBC and Compare means set carry
    ( uint8_t )reg >= *MDR ? P |= SET_CARRY : P &= ~SET_CARRY;

    reg -= *MDR;

    updateNegative( reg );
    updateZero( reg );
}

inline void CPUClass::CMP()
{
    Compare( A );
}

inline void CPUClass::CPX()
{
    Compare( X );
}

inline void CPUClass::CPY()
{
    Compare( Y );
}

// Decrementing
inline void CPUClass::DEC()
{
    currentCycles += 2;

    ( *MDR )--;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Incrementing
inline void CPUClass::INC()
{
    currentCycles += 2;

    ( *MDR )++;

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Flag Setting
inline void CPUClass::CLC()
{
    P &= ~SET_CARRY;
}

inline void CPUClass::SEC()
{
    P |= SET_CARRY;
}

inline void CPUClass::CLI()
{
    P &= ~SET_INTERRUPT_DISABLE;
}

inline void CPUClass::SEI()
{
    P |= SET_INTERRUPT_DISABLE;
}

inline void CPUClass::CLV()
{
    P &= ~SET_OVERFLOW;
}

/*
* NOTE: Based on what I've read these don't work on the NES. Nintendo disabled
* Binary Coded Decimal mode so they wouldn't have to pay licensing fees based on
* what I've read. So I suppose they would probably work and set the flag, it
* just wouldn't do anythin
*/
inline void CPUClass::CLD()
{
    P &= ~SET_DECIMAL;
}

inline void CPUClass::SED()
{
    P |= SET_DECIMAL;
}

// Jumping
inline void CPUClass::JMP()
{
    // Note: -1 because we increment the PC at the end of run
    PC = MDR - memory - 1;
}

inline void CPUClass::JSR()
{
    currentCycles += 2;

    uint8_t lo = PC & 0b0000000011111111;
    uint8_t hi = PC >> 8;

    // hi then lo so lo comes off first
    stack[ SP-- ] = hi;
    stack[ SP-- ] = lo;

    // Note: -1 because we increment the PC at the end of run
    PC = MDR - memory - 1;
}

// Loading
inline void CPUClass::LDA()
{
     A = *MDR;

    updateNegative( A );
    updateZero( A );
}

inline void CPUClass::LDX()
{
    X = *MDR;

    updateNegative( X );
    updateZero( X );
}

inline void CPUClass::LDY()
{
    Y = *MDR;

    updateNegative( Y );
    updateZero( Y );
}

// NOTHNG
inline void CPUClass::NOP()
{

}

// Register instructions
inline void CPUClass::TAX()
{
    X =  A;

    updateNegative( X );
    updateZero( X );
}

inline void CPUClass::TXA()
{
     A = X;

    updateNegative( A );
    updateZero( A );
}

inline void CPUClass::DEX()
{
    X--;

    updateNegative( X );
    updateZero( X );
}

inline void CPUClass::INX()
{
    X++;

    updateNegative( X );
    updateZero( X );
}

inline void CPUClass::TAY()
{
    Y =  A;

    updateNegative( Y );
    updateZero( Y );
}

inline void CPUClass::TYA()
{
     A = Y;

    updateNegative( Y );
    updateZero( Y );
}

inline void CPUClass::DEY()
{
    Y--;

    updateNegative( Y );
    updateZero( Y );
}

inline void CPUClass::INY()
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
inline void CPUClass::ROL()
{
    bool carry = ( ( *MDR ) & 0b10000000 ) == 0b10000000;

    currentCycles += 2;

    *MDR <<= 1;
    *MDR |= P & SET_CARRY;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Rotate right
inline void CPUClass::ROR()
{
    bool carry = ( ( *MDR ) & 0b00000001 ) == 0b00000001;

    currentCycles += 2;

    *MDR >>= 1;
    *MDR |= ( P & SET_CARRY ) << 7;

    if( carry )
    {
        P |= SET_CARRY;
    }

    updateNegative( *MDR );
    updateZero( *MDR );
}

// Return from interrupt
inline void CPUClass::RTI()
{
    currentCycles += 4;

    P = stack[ ++SP ];

    PC = stack[ ++SP ];
    PC += ( ( uint16_t )stack[ ++SP ] ) << 8;
    // Decrement here because we increment at the end of run
    PC--;
}

// Return from subroutine
inline void CPUClass::RTS()
{
    currentCycles += 4;

    // TODO: Something is going wrong attempting to return
    PC = stack[ ++SP ];
    PC += ( ( uint16_t )stack[ ++SP ] ) << 8;
}

// Store
inline void CPUClass::STA()
{
    *MDR =  A;
}

inline void CPUClass::STX()
{
    *MDR = X;
}

inline void CPUClass::STY()
{
    *MDR = Y;
}

// Not 100% sure what the difference is between this and NOP
inline void CPUClass::STP()
{

}

// Stack instructions
inline void CPUClass::TXS()
{
    SP = X;
}

inline void CPUClass::TSX()
{
    X = SP;

    updateNegative( X );
    updateZero( X );
}

inline void CPUClass::PHA()
{
    currentCycles++;

    stack[ SP-- ] =  A;
}

inline void CPUClass::PLA()
{
    currentCycles += 2;

    A = stack[ ++SP ];

    updateNegative( A );
    updateZero( A );
}

inline void CPUClass::PHP()
{
    currentCycles++;

    stack[ SP-- ] = P;
}

inline void CPUClass::PLP()
{
    currentCycles += 2;

    P = stack[ ++SP  ];
}
