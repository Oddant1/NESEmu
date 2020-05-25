#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <fstream>
#include <chrono>
#include <time.h>

const long CYCLE_TIME_N_SEC = 558L;

// TODO: Verify correct endianness
enum SetStatus
{
    SET_NEGATIVE          = 0b10000000,
    SET_OVERFLOW          = 0b01000000,
    // Bit 5 is unused
    SET_BREAK             = 0b00010000,
    SET_DECIMAL           = 0b00001000,
    SET_INTERRUPT_DISABLE = 0b00000100,
    SET_ZERO              = 0b00000010,
    SET_CARRY             = 0b00000001
};

enum UseRegister
{
    NONE,
    USE_ACC,
    USE_X,
    USE_Y
};

class CPUClass
{
    // TODO: Need to make the CPU keep time somehow
    // Very much subject to change
    public:

    // Function pointer for opcode methods
    typedef void ( CPUClass::*voidFunc )();

    // Counts total number of CPU cycles we've run since starting the emulator.
    // An unsigned long long int is guaranteed to be at least 64 bits which at
    // 1.79 million cycles a second gives us a VERY long time before we need to
    // worry about this thing wrapping around. Like several hundred thousand
    // years
    unsigned long long int totalCycles;

    /***************************************************************************
    * CPU Registers
    ***************************************************************************/
    // These will probably be interacted with in hex
    uint16_t programCounter = 0x0000;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100. It starts at FD though... For some reason
    uint8_t stackPointer = 0xFD;
    int8_t accumulator = 0x00;
    int8_t X = 0x00;
    int8_t Y = 0x00;

    // This will probably be interacted with using bitwise operations
    uint8_t status = 0x00;//0x34;

    // I think we're just going to leave the opcode here when we decode it
    // instead of shunting it of to another "register" because at this high of a
    // level that serves no purpose
    uint8_t MDR; // Memory Data Register

    // We can just think of this as where the opcode goes to be decoded, works
    // well enough since this is the decoded opcode
    voidFunc instructionRegister = &CPUClass::DECAbsolute;

    // void ( CPUClass::*foo )() = DECAbsolute;

    /***************************************************************************
    * Mapped memory
    ***************************************************************************/
    uint8_t memory [ 0x10000 ] = {};

    /***************************************************************************
    * Runs the emulated CPU
    ***************************************************************************/
    void run( std::ifstream &ROMImage );

    /***************************************************************************
    * CPU cycle
    ***************************************************************************/
    inline int8_t fetch();
    inline voidFunc decode();
    inline void execute();

    /***************************************************************************
    * Status handlers
    ***************************************************************************/
    void updateNegative();
    // TODO: This has opcodes
    void updateOverflow( int8_t oldAccumulator );
    // TODO: This has opcodes
    void updateBreak();
    // TODO: This has opcodes
    void updateDecimal();
    void updateInterruptDisable();
    void updateZero();
    // TODO: This has opcodes
    void updateCarry( int8_t oldAccumulator );

    /***************************************************************************
    * Handle addressing mode operand resolution
    ***************************************************************************/
    inline int8_t immediate();
    uint16_t zeroPage( UseRegister mode );
    uint16_t absolute( UseRegister mode );
    uint16_t indirect( UseRegister mode );
    // We may need one for relative, but I don't think so
    int8_t retrieveIndexOffset( UseRegister mode );


    // Now the real question, are all opcodes just methods of this class?
    // Probably. That's a lot of methods. . . Also are they inline?

    /***************************************************************************
    * CPU OpCode methods
    ***************************************************************************/
    // These are roughly alphabetical by type as laid out here
    // http://www.6502.org/tutorials/6502opcodes.html

    // Adding opcodes
    void ADC();

    void ADCImmediate(); // $69
    void ADCZeroPage();  // $65
    void ADCZeroPageX(); // $75
    void ADCAbsolute();  // $6D
    void ADCAbsoluteX(); // $7D
    void ADCAbsoluteY(); // $79
    void ADCIndirectX(); // $61
    void ADCIndirectY(); // $71

    // Bitwise anding opcodes
    void AND();

    void ANDImmediate(); // $29
    void ANDZeroPage();  // $25
    void ANDZeroPageX(); // $35
    void ANDAbsolute();  // $2D
    void ANDAbsoluteX(); // $3D
    void ANDAbsoluteY(); // $39
    void ANDIndirectX(); // $21
    void ANDIndirectY(); // $31

    // Left shift opcodes
    void ASL();

    void ASLAcc();       // $0A
    void ASLZeroPage();  // $06
    void ASLZeroPageX(); // $16
    void ASLAbsolute();  // $0E
    void ASLAbsoluteX(); // $1E

    // Bit test opcodes
    void BIT();

    void BITZeroPage();  // $23
    void BITAbsolute();  // $2C

    // Branching opcodes
    void BRANCH();

    void BPL();          // $10
    void BMI();          // $30
    void BVC();          // $50
    void BVS();          // $70
    void BCC();          // $90
    void BCS();          // $B0
    void BNE();          // $D0
    void BEQ();          // $F0

    // Break opcodes
    void BRK();          // $00

    // Comparing opcodes
    void CMP( UseRegister compare );

    void CMPImmediate(); // $C9
    void CMPZeroPage();  // $C5
    void CMPZeroPageX(); // $D5
    void CMPAbsolute();  // $CD
    void CMPAbsoluteX(); // $DD
    void CMPAbsoluteY(); // $D9
    void CMPIndirectX(); // $C1
    void CMPIndirectY(); // $D1

    void CPXImmediate(); // $E0
    void CPXZeroPage();  // $E4
    void CPXAbsolute();  // $EC

    void CPYImmediate(); // $C0
    void CPYZeroPage();  // $C4
    void CPYAbsolute();  // $CC

    // Decrementing opcodes
    void DEC();

    void DECZeroPage();  // $C6
    void DECZeroPageX(); // $D6
    void DECAbsolute();  // $CE
    void DECAbsoluteX(); // $DE

    // Exclusive bitwise or opcodes
    void EOR();

    void EORImmediate(); // $C6
    void EORZeroPage();  // $45
    void EORZeroPageX(); // $55
    void EORAbsolute();  // $4D
    void EORAbsoluteX(); // $5D
    void EORAbsoluteY(); // $59
    void EORIndirectX(); // $41
    void EORIndirectY(); // $51

    // Flag Setting Opcodes

    void CLC();          // $18
    void SEC();          // $38
    void CLI();          // $58
    void SEI();          // $78
    void CLV();          // $B8
    void CLD();          // $D8
    void SED();          // $F8

    // Incrementing opcodes
    void INC();

    void INCZeroPage();  // $E6
    void INCZeroPageX(); // $F6
    void INCAbsolute();  // $EE
    void INCAbsoluteX(); // $FE

    // Jumping opcodes
    void JMP();

    void JMPAbsolute();  // $4C
    void JMPIndirect();  // $6C

    void JSR();          // $20

    // Loading opcodes
    void LOAD( UseRegister destination );

    void LDAImmediate(); // $A9
    void LDAZeroPage();  // $A5
    void LDAZeroPageX(); // $B5
    void LDAAbsolute();  // $AD
    void LDAAbsoluteX(); // $BD
    void LDAAbsoluteY(); // $B9
    void LDAIndirectX(); // $A1
    void LDAIndirectY(); // $B1

    void LDXImmediate(); // $A2
    void LDXZeroPage();  // $A6
    void LDXZeroPageY(); // $B6
    void LDXAbsolute();  // $AE
    void LDXAbsoluteY(); // $BE

    void LDYImmediate(); // $A0
    void LDYZeroPage();  // $A4
    void LDYZeroPageX(); // $B4
    void LDYAbsolute();  // $AC
    void LDYAbsoluteX(); // $BC

    // Right shift instructions
    void LSR();

    void LSRAcc();       // $4A
    void LSRZeroPage();  // $46
    void LSRZeroPageX(); // $56
    void LSRAbsolute();  // $4E
    void LSRAbsoluteX(); // $5E

    // NOTHNG NOP has so many slots
    void NOP(); // $04 $0C $14 $1A $1C $34 $3A $3C $44 $54 $5A $5C $64 $74 $7A
                // $7C $80 $82 $89 $C2 $D4 $DA $DC $E2 $EA $F4 $FA $FC

    // Bitwise or opcodes
    void ORA();

    void ORAImmediate(); // $09
    void ORAZeroPage();  // $05
    void ORAZeroPageX(); // $15
    void ORAAbsolute();  // $0D
    void ORAAbsoluteX(); // $1D
    void ORAAbsoluteY(); // $19
    void ORAIndirectX(); // $01
    void ORAIndirectY(); // $11

    // Register instruction opcodes

    void TAX();          // $AA
    void TXA();          // $8A
    void DEX();          // $CA
    void INX();          // $E8
    void TAY();          // $A8
    void TYA();          // $98
    void DEY();          // $88
    void INY();          // $C8

    // Rotate left opcodes
    void ROL();

    void ROLAcc();       // $2A
    void ROLZeroPage();  // $26
    void ROLZeroPageX(); // $36
    void ROLAbsolute();  // $2E
    void ROLAbsoluteX(); // $3E

    // Rotate right opcodes
    void ROR();

    void RORAcc();       // $6A
    void RORZeroPage();  // $66
    void RORZeroPageX(); // $76
    void RORAbsolute();  // $6E
    void RORAbsoluteX(); // $7E

    // Return from interrupt
    void RTI();          // $40

    // Return from subroutine
    void RTS();          // $60

    // Subtract opcodes
    void SBC();

    void SBCImmediate(); // $E9
    void SBCZeroPage();  // $E5
    void SBCZeroPageX(); // $F5
    void SBCAbsolute();  // $ED
    void SBCAbsoluteX(); // $FD
    void SBCAbsoluteY(); // $F9
    void SBCIndirectX(); // $E1
    void SBCIndirectY(); // $F1

    // Store opcodes
    void STORE( UseRegister storeFrom );

    void STAZeroPage();  // $85
    void STAZeroPageX(); // $95
    void STAAbsolute();  // $8D
    void STAAbsoluteX(); // $9D
    void STAAbsoluteY(); // $99
    void STAIndirectX(); // $81
    void STAIndirectY(); // $91

    void STXZeroPage();  // $86
    void STXZeroPageY(); // $96
    void STXAbsolute();  // $8E

    void STYZeroPage();  // $84
    void STYZeroPageX(); // $94
    void STYAbsolute();  // $8C

    // Stack instruction opcodes

    void TXS();          // $9A
    void TSX();          // $BA
    void PHA();          // $48
    void PLA();          // $68
    void PHP();          // $08
    void PLP();          // $28

    voidFunc opCodeArray[ 256 ] =
    {
        &CPUClass::NOP,
        &CPUClass::ORAIndirectX,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ORAZeroPage,
        &CPUClass::ASLZeroPage,
        nullptr, // unofficial
        &CPUClass::PHP,
        &CPUClass::ORAImmediate,
        &CPUClass::ASLAcc,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ORAAbsolute,
        &CPUClass::ASLAbsolute,
        nullptr, // unofficial
        &CPUClass::BPL,
        &CPUClass::ORAIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ORAZeroPageX,
        &CPUClass::ASLZeroPageX,
        nullptr, // unofficial
        &CPUClass::CLC,
        &CPUClass::ORAAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ORAAbsoluteX,
        &CPUClass::ASLAbsoluteX,
        nullptr, // unofficial
        &CPUClass::JSR,
        &CPUClass::ANDIndirectX,
        nullptr,
        nullptr, // unofficial
        &CPUClass::BITZeroPage,
        &CPUClass::ANDZeroPage,
        &CPUClass::ROLZeroPage,
        nullptr, // unofficial
        &CPUClass::PLP,
        &CPUClass::ANDImmediate,
        &CPUClass::ROLAcc,
        nullptr, // unofficial
        &CPUClass::BITAbsolute,
        &CPUClass::ANDAbsolute,
        &CPUClass::ROLAbsolute,
        nullptr, // unofficial
        &CPUClass::BMI,
        &CPUClass::ANDIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ANDZeroPageX,
        &CPUClass::ROLZeroPageX,
        nullptr, // unofficial
        &CPUClass::SEC,
        &CPUClass::ANDAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ANDAbsoluteX,
        &CPUClass::ROLAbsoluteX,
        nullptr, // unofficial
        &CPUClass::RTI,
        &CPUClass::EORIndirectX,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::EORZeroPage,
        &CPUClass::LSRZeroPage,
        nullptr, // unofficial
        &CPUClass::PHA,
        &CPUClass::EORImmediate,
        &CPUClass::LSRAcc,
        nullptr, // uonfficial
        &CPUClass::JMPAbsolute,
        &CPUClass::EORAbsolute,
        &CPUClass::LSRAbsolute,
        nullptr, // unofficial
        &CPUClass::BVC,
        &CPUClass::EORIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::EORZeroPageX,
        &CPUClass::LSRZeroPageX,
        nullptr, // unofficial
        &CPUClass::CLI,
        &CPUClass::EORAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::EORAbsoluteX,
        &CPUClass::LSRAbsoluteX,
        nullptr, // unofficial
        &CPUClass::RTS,
        &CPUClass::ADCIndirectX,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ADCZeroPage,
        &CPUClass::RORZeroPage,
        nullptr, // unofficial
        &CPUClass::PLA,
        &CPUClass::ADCImmediate,
        &CPUClass::RORAcc,
        nullptr, // unofficial
        &CPUClass::JMPIndirect,
        &CPUClass::ADCAbsolute,
        &CPUClass::RORAbsolute,
        nullptr, // unofficial
        &CPUClass::BVS,
        &CPUClass::ADCIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ADCZeroPageX,
        &CPUClass::RORZeroPageX,
        nullptr, // unofficial
        &CPUClass::SEI,
        &CPUClass::ADCAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::ADCAbsoluteX,
        &CPUClass::RORAbsoluteX,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::STAIndirectY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::STYZeroPage,
        &CPUClass::STAZeroPage,
        &CPUClass::STXZeroPage,
        nullptr, // unofficial
        &CPUClass::DEY,
        &CPUClass::NOP,
        &CPUClass::TXA,
        nullptr, // unofficial
        &CPUClass::STYAbsolute,
        &CPUClass::STAAbsolute,
        &CPUClass::STXAbsolute,
        nullptr, // unofficial
        &CPUClass::BCC,
        &CPUClass::STAIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::STYZeroPageX,
        &CPUClass::STAZeroPageX,
        &CPUClass::STXZeroPageY,
        nullptr, // unofficial
        &CPUClass::TYA,
        &CPUClass::STAAbsoluteY,
        &CPUClass::TXS,
        nullptr, // unofficial
        nullptr, // unofficial
        &CPUClass::STAAbsoluteX,
        nullptr, // unofficial
        &CPUClass::LDYImmediate,
        &CPUClass::LDAIndirectX,
        &CPUClass::LDXImmediate,
        nullptr, // unofficial
        &CPUClass::LDYZeroPage,
        &CPUClass::LDAZeroPage,
        &CPUClass::LDXZeroPage,
        nullptr, // unofficial
        &CPUClass::TAY,
        &CPUClass::LDAImmediate,
        &CPUClass::TAX,
        nullptr, // unofficial
        &CPUClass::LDYAbsolute,
        &CPUClass::LDAAbsolute,
        &CPUClass::LDXAbsolute,
        nullptr, // unofficial
        &CPUClass::BCS,
        &CPUClass::LDAIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::LDYZeroPageX,
        &CPUClass::LDAZeroPageX,
        &CPUClass::LDXZeroPageY,
        nullptr, // unofficial
        &CPUClass::CLV,
        &CPUClass::LDAAbsoluteY,
        &CPUClass::TSX,
        nullptr, // unofficial
        &CPUClass::LDYAbsoluteX,
        &CPUClass::LDAAbsoluteX,
        &CPUClass::LDXAbsoluteY,
        nullptr, // unofficial
        &CPUClass::CPYImmediate,
        &CPUClass::CMPIndirectX,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::CPYZeroPage,
        &CPUClass::CMPZeroPage,
        &CPUClass::DECZeroPage,
        nullptr, // unofficial,
        &CPUClass::INY,
        &CPUClass::CMPImmediate,
        &CPUClass::DEX,
        nullptr, // unofficial
        &CPUClass::CPYAbsolute,
        &CPUClass::CMPAbsolute,
        &CPUClass::DECAbsolute,
        nullptr, // unofficial
        &CPUClass::BNE,
        &CPUClass::CMPIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::CMPZeroPageX,
        &CPUClass::DECZeroPageX,
        nullptr, // unofficial
        &CPUClass::CLD,
        &CPUClass::CMPAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::CMPAbsoluteX,
        &CPUClass::DECAbsoluteX,
        nullptr, // unofficial
        &CPUClass::CPXImmediate,
        &CPUClass::SBCIndirectX,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::CPXZeroPage,
        &CPUClass::SBCZeroPage,
        &CPUClass::INCZeroPage,
        nullptr, // unofficial
        &CPUClass::INX,
        &CPUClass::SBCImmediate,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::CPXAbsolute,
        &CPUClass::SBCAbsolute,
        &CPUClass::INCAbsolute,
        nullptr, // unofficial
        &CPUClass::BEQ,
        &CPUClass::SBCIndirectY,
        nullptr,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::SBCZeroPageX,
        &CPUClass::INCZeroPageX,
        nullptr, // unofficial
        &CPUClass::SED,
        &CPUClass::SBCAbsoluteY,
        &CPUClass::NOP,
        nullptr, // unofficial
        &CPUClass::NOP,
        &CPUClass::SBCAbsoluteX,
        &CPUClass::INCAbsoluteX,
        nullptr  // unofficial
    };
};

#endif
