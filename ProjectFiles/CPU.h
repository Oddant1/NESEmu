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

    static const int cyclesPerFrame = 1790000 / 60;

    // Function pointer for opcode methods
    typedef void ( CPUClass::*voidFunc )();

    // Counts total number of CPU cycles we've run since starting the emulator.
    // An unsigned long long int is guaranteed to be at least 64 bits which at
    // 1.79 million cycles a second gives us a VERY long time before we need to
    // worry about this thing wrapping around. Like several hundred thousand
    // years
    unsigned long long int totalCycles;
    int cyclesRemaining;

    /***************************************************************************
    * CPU Registers
    ***************************************************************************/
    // These will probably be interacted with in hex
    // For test load all data into 0xC000 on. Should fit. Start PC at
    // 0xC000
    uint16_t programCounter = 0x0000;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100. It's initialzed to FD though... For some reason. This is based
    // on this source https://wiki.nesdev.com/w/index.php/CPU_ALL
    uint8_t stackPointer = 0xFD;
    int8_t accumulator = 0x00;
    int8_t X = 0x00;
    int8_t Y = 0x00;

    // This will probably be interacted with using bitwise operations
    uint8_t status = 0x00; //0x34;

    // I think we're just going to leave the opcode here when we decode it
    // instead of shunting it of to another "register" because at this high of a
    // level that serves no purpose
    uint8_t* MDR; // Memory Data Register

    // We can just think of this as where the opcode goes to be decoded, works
    // well enough since this is the decoded opcode
    voidFunc instruction;
    voidFunc addressMode;

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
    inline void fetch();
    inline void decodeAddr();
    inline void decodeOP();
    inline void execute();

    /***************************************************************************
    * Status handlers
    ***************************************************************************/
    void updateNegative();
    // TODO: This has opcodes. This may also need to be opcode specific
    void updateOverflow( int8_t oldAccumulator );
    // TODO: This has opcodes
    void updateBreak();
    // TODO: This has opcodes
    void updateDecimal();
    void updateInterruptDisable();
    void updateZero();
    // TODO: This has opcodes. This will probably need to be more opcode
    // specific
    void updateCarry( int8_t oldAccumulator );

    /***************************************************************************
    * Handle addressing mode operand resolution
    ***************************************************************************/
    // Absolute
    void absolute( UseRegister mode );
    void abs();
    void abX();
    void abY();

    // Indirect
    void indirect( UseRegister mode );
    void ind();
    void inX();
    void inY();

    // Zero Page
    void zeroPage( UseRegister mode );
    void zer();
    void zeX();
    void zeY();

    // Immediate: Stupid simple
    void imm();

    // Relative
    void rel();

    // These three are formalities
    // Implied: This does nothing
    void imp();

    // Accumulator: This does nothing
    void acc();

    // None: This literally does nothing
    void non();

    /***************************************************************************
    * CPU OpCode methods
    ***************************************************************************/
    // These are roughly alphabetical by type as laid out here
    // http://www.6502.org/tutorials/6502opcodes.html

    // Adding
    void ADC();

    // Bitwise anding
    void AND();

    // Left shift
    void ASL();

    // Bit test
    void BIT();

    // Branching
    void Branch();

    void BPL();
    void BMI();
    void BVC();
    void BVS();
    void BCC();
    void BCS();
    void BNE();
    void BEQ();

    // Break
    void BRK();

    // Comparing
    void CMP();
    void CPX();
    void CPY();

    // Decrementing
    void DEC();

    // Exclusive bitwise or
    void EOR();

    // Flag Setting
    void CLC();
    void SEC();
    void CLI();
    void SEI();
    void CLV();
    void CLD();
    void SED();

    // Incrementing
    void INC();

    // Jumping
    void JMP();
    void JSR();

    // Loading
    void LDA();
    void LDX();
    void LDY();

    // Left shift
    void LSR();

    // NOTHNG
    void NOP();

    // Bitwise or
    void ORA();

    // Register instructions
    void TAX();
    void TXA();
    void DEX();
    void INX();
    void TAY();
    void TYA();
    void DEY();
    void INY();

    // Rotate left
    void ROL();

    // Rotate right
    void ROR();

    // Return from interrupt
    void RTI();

    // Return from subroutine
    void RTS();

    // Subtract
    void SBC();

    // Store
    void STA();
    void STX();
    void STY();

    // Not 100% sure what the difference is between this and NOP. NOP seems to
    // be a formalized opcode where this is a placeholder
    void STP();

    // Stack instructions
    void TXS();
    void TSX();
    void PHA();
    void PLA();
    void PHP();
    void PLP();

    voidFunc opCodeArray[ 256 ] =
    {
        &CPUClass::BRK, &CPUClass::ORA, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::ORA, &CPUClass::ASL, nullptr,
        &CPUClass::PHP, &CPUClass::ORA, &CPUClass::ASL, nullptr,
        &CPUClass::NOP, &CPUClass::ORA, &CPUClass::ASL, nullptr,
        &CPUClass::BPL, &CPUClass::ORA, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::ORA, &CPUClass::ASL, nullptr,
        &CPUClass::CLC, &CPUClass::ORA, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::ORA, &CPUClass::ASL, nullptr,
        &CPUClass::JSR, &CPUClass::AND, &CPUClass::STP, nullptr,
        &CPUClass::BIT, &CPUClass::AND, &CPUClass::ROL, nullptr,
        &CPUClass::PLP, &CPUClass::AND, &CPUClass::ROL, nullptr,
        &CPUClass::BIT, &CPUClass::AND, &CPUClass::ROL, nullptr,
        &CPUClass::BMI, &CPUClass::AND, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::AND, &CPUClass::ROL, nullptr,
        &CPUClass::SEC, &CPUClass::AND, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::AND, &CPUClass::ROL, nullptr,
        &CPUClass::RTI, &CPUClass::EOR, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::EOR, &CPUClass::LSR, nullptr,
        &CPUClass::PHA, &CPUClass::EOR, &CPUClass::LSR, nullptr,
        &CPUClass::JMP, &CPUClass::EOR, &CPUClass::LSR, nullptr,
        &CPUClass::BVC, &CPUClass::EOR, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::EOR, &CPUClass::LSR, nullptr,
        &CPUClass::CLI, &CPUClass::EOR, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::EOR, &CPUClass::LSR, nullptr,
        &CPUClass::RTS, &CPUClass::ADC, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::ADC, &CPUClass::ROR, nullptr,
        &CPUClass::PLA, &CPUClass::ADC, &CPUClass::ROR, nullptr,
        &CPUClass::JMP, &CPUClass::ADC, &CPUClass::ROR, nullptr,
        &CPUClass::BVS, &CPUClass::ADC, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::ADC, &CPUClass::ROR, nullptr,
        &CPUClass::SEI, &CPUClass::ADC, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::ADC, &CPUClass::ROR, nullptr,
        &CPUClass::NOP, &CPUClass::STA, &CPUClass::NOP, nullptr,
        &CPUClass::STY, &CPUClass::STA, &CPUClass::STX, nullptr,
        &CPUClass::DEY, &CPUClass::NOP, &CPUClass::TXA, nullptr,
        &CPUClass::STY, &CPUClass::STA, &CPUClass::STX, nullptr,
        &CPUClass::BCC, &CPUClass::STA, &CPUClass::STP, nullptr,
        &CPUClass::STY, &CPUClass::STA, &CPUClass::STX, nullptr,
        &CPUClass::TYA, &CPUClass::STA, &CPUClass::TXS, nullptr,
        nullptr       , &CPUClass::STA, nullptr       , nullptr,
        &CPUClass::LDY, &CPUClass::LDA, &CPUClass::LDX, nullptr,
        &CPUClass::LDY, &CPUClass::LDA, &CPUClass::LDX, nullptr,
        &CPUClass::TAY, &CPUClass::LDA, &CPUClass::TAX, nullptr,
        &CPUClass::LDY, &CPUClass::LDA, &CPUClass::LDX, nullptr,
        &CPUClass::BCS, &CPUClass::LDA, &CPUClass::STP, nullptr,
        &CPUClass::LDY, &CPUClass::LDA, &CPUClass::LDX, nullptr,
        &CPUClass::CLV, &CPUClass::LDA, &CPUClass::TSX, nullptr,
        &CPUClass::LDY, &CPUClass::LDA, &CPUClass::LDX, nullptr,
        &CPUClass::CPY, &CPUClass::CMP, &CPUClass::NOP, nullptr,
        &CPUClass::CPY, &CPUClass::CMP, &CPUClass::DEC, nullptr,
        &CPUClass::INY, &CPUClass::CMP, &CPUClass::DEX, nullptr,
        &CPUClass::CPY, &CPUClass::CMP, &CPUClass::DEC, nullptr,
        &CPUClass::BNE, &CPUClass::CMP, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::CMP, &CPUClass::DEC, nullptr,
        &CPUClass::CLD, &CPUClass::CMP, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::CMP, &CPUClass::DEC, nullptr,
        &CPUClass::CPX, &CPUClass::SBC, &CPUClass::NOP, nullptr,
        &CPUClass::CPX, &CPUClass::SBC, &CPUClass::INC, nullptr,
        &CPUClass::INX, &CPUClass::SBC, &CPUClass::NOP, nullptr,
        &CPUClass::CPX, &CPUClass::SBC, &CPUClass::INC, nullptr,
        &CPUClass::BEQ, &CPUClass::SBC, &CPUClass::STP, nullptr,
        &CPUClass::NOP, &CPUClass::SBC, &CPUClass::INC, nullptr,
        &CPUClass::SED, &CPUClass::SBC, &CPUClass::NOP, nullptr,
        &CPUClass::NOP, &CPUClass::SBC, &CPUClass::INC, nullptr
    };

    voidFunc memoryAccessArray[ 256 ] =
    {
        &CPUClass::imp, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::acc, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::abs, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imp, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imp, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::ind, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::inY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imm, &CPUClass::inX, &CPUClass::imm, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeY, &CPUClass::zeY,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imm, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abY, &CPUClass::abY,
        &CPUClass::imm, &CPUClass::inX, &CPUClass::imm, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeY, &CPUClass::zeY,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abY, &CPUClass::abY,
        &CPUClass::imm, &CPUClass::inX, &CPUClass::imm, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imm, &CPUClass::inX, &CPUClass::imm, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::imp, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX
    };
};

#endif
