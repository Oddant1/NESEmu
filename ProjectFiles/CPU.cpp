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
    auto start = std::chrono::high_resolution_clock::now();
    for( int i = 0; i < 184; i++ )
    {
        MDR = fetch();
        instructionRegister = decode();
        execute();

        auto current = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( current - start ).count();

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

    updateNegative();
    updateOverflow( oldAccumulator );
    updateZero();
    updateCarry( oldAccumulator );
}

void CPUClass::ADCImmediate()
{
    totalCycles += 2;
    MDR = immediate();
    ADC();
}

void CPUClass::ADCZeroPage()
{
    totalCycles += 3;
    MDR = memory[ zeroPage() ];
    ADC();
}

void CPUClass::ADCZeroPageX()
{
    totalCycles += 4;
    MDR = memory[ zeroPage( USE_X ) ];
    ADC();
}

void CPUClass::ADCAbsolute()
{
    totalCycles += 4;
    MDR = memory[ absolute() ];
    ADC();
}

void CPUClass::ADCAbsoluteX()
{
    totalCycles += 4;
    MDR = memory[ absolute( USE_X ) ];
    ADC();
}

void CPUClass::ADCAbsoluteY()
{
    totalCycles += 4;
    MDR = memory[ absolute( USE_Y ) ];
    ADC();
}

void CPUClass::ADCIndirectX()
{
    totalCycles += 6;
    MDR = memory[ indirect( USE_X ) ];
    ADC();
}

void CPUClass::ADCIndirectY()
{
    totalCycles += 5;
    MDR = memory[ indirect( USE_Y ) ];
    ADC();
}

// Bitwise anding opcodes
void CPUClass::AND()
{
    accumulator &= MDR;

    updateNegative();
    updateZero();
}

void CPUClass::ANDImmediate()
{
    totalCycles += 2;
    MDR = immediate();
    AND();
}

void CPUClass::ANDZeroPage()
{
    totalCycles += 3;
    MDR = memory[ zeroPage() ];
    AND();
}

void CPUClass::ANDZeroPageX()
{
    totalCycles += 4;
    MDR = memory[ zeroPage( USE_X ) ];
    AND();
}

void CPUClass::ANDAbsolute()
{
    totalCycles += 4;
    MDR = memory[ absolute() ];
    AND();
}

void CPUClass::ANDAbsoluteX()
{
    totalCycles += 4;
    MDR = memory[ absolute( USE_X ) ];
    AND();
}

void CPUClass::ANDAbsoluteY()
{
    totalCycles += 4;
    MDR = memory[ absolute( USE_Y ) ];
    AND();
}

void CPUClass::ANDIndirectX()
{
    totalCycles += 6;
    MDR = memory[ indirect( USE_X ) ];
    AND();
}

void CPUClass::ANDIndirectY()
{
    totalCycles += 5;
    MDR = memory[ indirect( USE_Y ) ];
    AND();
}

// Left shift opcodes
void CPUClass::ASL()
{
    int8_t oldMDR = MDR;

    MDR << 1;

    updateNegative();
    updateZero();
    updateCarry( oldMDR );
}

// TODO: This is a bit gross
void CPUClass::ASLAcc()
{
    int8_t oldAccumulator = accumulator;

    totalCycles += 2;
    accumulator << 1;

    updateNegative();
    updateZero();
    updateCarry( oldAccumulator );
}

void CPUClass::ASLZeroPage()
{
    uint16_t address = zeroPage();

    totalCycles += 5;
    MDR = memory[ address ];
    ASL();
    memory[ address ] = MDR;
}

void CPUClass::ASLZeroPageX()
{
    uint16_t address = zeroPage( USE_X );

    totalCycles += 6;
    MDR = memory[ address ];
    ASL();
    memory[ address ] = MDR;
}

void CPUClass::ASLAbsolute()
{
    uint16_t address = absolute();

    totalCycles += 6;
    MDR = memory[ address ];
    ASL();
    memory[ address ] = MDR;
}

void CPUClass::ASLAbsoluteX()
{
    uint16_t address = zeroPage( USE_X );

    totalCycles += 7;
    MDR = memory[ address ];
    ASL();
    memory[ address ] = MDR;
}

// Bit test opcodes
void CPUClass::BIT()
{

}

void CPUClass::BITZeroPage()
{

}

void CPUClass::BITAbsolute()
{

}

// Branching opcodes
void CPUClass::BRANCH()
{

}

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

// Break opcodes
void CPUClass::BRK()
{

}

// Comparing opcodes
void CPUClass::CMP( UseRegister compare )
{

}

void CPUClass::CMPImmediate()
{

}

void CPUClass::CMPZeroPage()
{

}

void CPUClass::CMPZeroPageX()
{

}

void CPUClass::CMPAbsolute()
{

}

void CPUClass::CMPAbsoluteX()
{

}

void CPUClass::CMPAbsoluteY()
{

}

void CPUClass::CMPIndirectX()
{

}

void CPUClass::CMPIndirectY()
{

}

void CPUClass::CPXImmediate()
{

}

void CPUClass::CPXZeroPage()
{

}

void CPUClass::CPXAbsolute()
{

}

void CPUClass::CPYImmediate()
{

}

void CPUClass::CPYZeroPage()
{

}

void CPUClass::CPYAbsolute()
{

}

// Decrementing opcodes
void CPUClass::DEC()
{

}

void CPUClass::DECZeroPage()
{

}

void CPUClass::DECZeroPageX()
{

}

void CPUClass::DECAbsolute()
{

}

void CPUClass::DECAbsoluteX()
{

}

// Exclusive bitwise or opcodes
void CPUClass::EOR()
{

}

void CPUClass::EORImmediate()
{

}

void CPUClass::EORZeroPage()
{

}

void CPUClass::EORZeroPageX()
{

}

void CPUClass::EORAbsolute()
{

}

void CPUClass::EORAbsoluteX()
{

}

void CPUClass::EORAbsoluteY()
{

}

void CPUClass::EORIndirectX()
{

}

void CPUClass::EORIndirectY()
{

}

// Flag Setting Opcodes
void CPUClass::CLC()
{
    totalCycles += 2;
    status &= ~SET_CARRY;
}

void CPUClass::SEC()
{
    totalCycles += 2;
    status |= SET_CARRY;
}

void CPUClass::CLI()
{
    totalCycles += 2;
    status &= ~SET_INTERRUPT_DISABLE;
}

void CPUClass::SEI()
{
    totalCycles += 2;
    status |= SET_INTERRUPT_DISABLE;
}

void CPUClass::CLV()
{
    totalCycles += 2;
    status &= ~SET_OVERFLOW;
}

void CPUClass::CLD()
{
    totalCycles += 2;
    status &= ~SET_DECIMAL;
}

void CPUClass::SED()
{
    totalCycles += 2;
    status |= SET_DECIMAL;
}

// Incrementing opcodes
void CPUClass::INC()
{

}

void CPUClass::INCZeroPage()
{

}

void CPUClass::INCZeroPageX()
{

}

void CPUClass::INCAbsolute()
{

}

void CPUClass::INCAbsoluteX()
{

}

// Jumping opcodes
void CPUClass::JMP()
{

}

void CPUClass::JMPAbsolute()
{

}

void CPUClass::JMPIndirect()
{

}

void CPUClass::JSR()
{

}

// Loading opcodes
void CPUClass::LOAD( UseRegister destination )
{

}

void CPUClass::LDAImmediate()
{

}

void CPUClass::LDAZeroPage()
{

}

void CPUClass::LDAZeroPageX()
{

}

void CPUClass::LDAAbsolute()
{

}

void CPUClass::LDAAbsoluteX()
{

}

void CPUClass::LDAAbsoluteY()
{

}

void CPUClass::LDAIndirectX()
{

}

void CPUClass::LDAIndirectY()
{

}

void CPUClass::LDXImmediate()
{

}

void CPUClass::LDXZeroPage()
{

}

void CPUClass::LDXZeroPageY()
{

}

void CPUClass::LDXAbsolute()
{

}

void CPUClass::LDXAbsoluteY()
{

}

void CPUClass::LDYImmediate()
{

}

void CPUClass::LDYZeroPage()
{

}

void CPUClass::LDYZeroPageX()
{

}

void CPUClass::LDYAbsolute()
{

}

void CPUClass::LDYAbsoluteX()
{

}

// Right shift instructions
void CPUClass::LSR()
{

}

void CPUClass::LSRAcc()
{

}

void CPUClass::LSRZeroPage()
{

}

void CPUClass::LSRZeroPageX()
{

}

void CPUClass::LSRAbsolute()
{

}

void CPUClass::LSRAbsoluteX()
{

}

// NOTHNG NOP has so many slots
void CPUClass::NOP()
{

}

// Bitwise or opcodes
void CPUClass::ORA()
{

}

void CPUClass::ORAImmediate()
{

}

void CPUClass::ORAZeroPage()
{

}

void CPUClass::ORAZeroPageX()
{

}

void CPUClass::ORAAbsolute()
{

}

void CPUClass::ORAAbsoluteX()
{

}

void CPUClass::ORAAbsoluteY()
{

}

void CPUClass::ORAIndirectX()
{

}

void CPUClass::ORAIndirectY()
{

}

// Register instruction opcodes
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

// Rotate left opcodes
void CPUClass::ROL()
{

}

void CPUClass::ROLAcc()
{

}

void CPUClass::ROLZeroPage()
{

}

void CPUClass::ROLZeroPageX()
{

}

void CPUClass::ROLAbsolute()
{

}

void CPUClass::ROLAbsoluteX()
{

}

// Rotate right opcodes
void CPUClass::ROR()
{

}

void CPUClass::RORAcc()
{

}

void CPUClass::RORZeroPage()
{

}

void CPUClass::RORZeroPageX()
{

}

void CPUClass::RORAbsolute()
{

}

void CPUClass::RORAbsoluteX()
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

// Subtract opcodes
void CPUClass::SBC()
{

}

void CPUClass::SBCImmediate()
{

}

void CPUClass::SBCZeroPage()
{

}

void CPUClass::SBCZeroPageX()
{

}

void CPUClass::SBCAbsolute()
{

}

void CPUClass::SBCAbsoluteX()
{

}

void CPUClass::SBCAbsoluteY()
{

}

void CPUClass::SBCIndirectX()
{

}

void CPUClass::SBCIndirectY()
{

}

// Store opcodes
void CPUClass::STORE( UseRegister storeFrom )
{

}

void CPUClass::STAZeroPage()
{

}

void CPUClass::STAZeroPageX()
{

}

void CPUClass::STAAbsolute()
{

}

void CPUClass::STAAbsoluteX()
{

}

void CPUClass::STAAbsoluteY()
{

}

void CPUClass::STAIndirectX()
{

}

void CPUClass::STAIndirectY()
{

}

void CPUClass::STXZeroPage()
{

}

void CPUClass::STXZeroPageY()
{

}

void CPUClass::STXAbsolute()
{

}

void CPUClass::STYZeroPage()
{

}

void CPUClass::STYZeroPageX()
{

}

void CPUClass::STYAbsolute()
{

}

// Stack instruction opcodes
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
