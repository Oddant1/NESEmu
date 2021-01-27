#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <fstream>
#include <chrono>
#include <time.h>

const long CYCLE_TIME_N_SEC = 558L;

// TODO: Verify correct endianness
enum SetPdecodeAddr
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
    // In the logs this starts at 7 idk why
    unsigned long long int totalCycles = 7;
    int cyclesRemaining;
    int currentCycles;

    /***************************************************************************
    * CPU Registers
    ***************************************************************************/
    // These will probably be interacted with in hex
    // For test load all data into 0xC000 on. Should fit. Start PC at
    // 0xC000
    uint16_t PC = 0xC000;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100. It's initialzed to FD though... For some reason. This is based
    // on this source https://wiki.nesdev.com/w/index.php/CPU_ALL
    uint8_t SP = 0xFD;
    int8_t A = 0x00;
    int8_t X = 0x00;
    int8_t Y = 0x00;

    // This will probably be interacted with using bitwise operations
    uint8_t P = 0b00110100;

    // I think we're just going to leave the opcode here when we decode it
    // instead of shunting it of to another "register" because at this high of a
    // level that serves no purpose
    uint8_t* MDR; // Memory Data Register

    // We can just think of this as where the opcode goes to be decoded, works
    // well enough since this is the decoded opcode
    voidFunc addressMode;
    voidFunc instruction;

    /***************************************************************************
    * Mapped memory
    ***************************************************************************/
    // The last address is the accumulator
    uint8_t memory[ 0x10000 ] = {};
    // The stack is the second page in memory
    uint8_t* stack = &memory[ 0x100 ];

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
    * P handlers
    ***************************************************************************/
    inline void updateNegative( int8_t reg );
    // TODO: This has opcodes
    inline void updateBreak();
    // TODO: This has opcodes
    inline void updateDecimal();
    inline void updateInterruptDisable();
    inline void updateZero( int8_t reg );

    /***************************************************************************
    * Handle addressing mode operand resolution
    ***************************************************************************/
    // Absolute
    inline void abs( UseRegister mode );
    inline void abX();
    inline void abY();

    // Indirect
    inline void ind( UseRegister mode );
    inline void inX();
    inline void inY();

    // Zero Page
    inline void zer( UseRegister mode );
    inline void zeX();
    inline void zeY();

    // Immediate: Stupid simple
    inline void imm();

    // Relative
    inline void rel();

    // These three are formalities
    // imp: This does nothing
    inline void imp();

    // A: This does nothing
    inline void acc();

    // None: This literally does nothing
    inline void non();

    /***************************************************************************
    * CPU OpCode methods
    ***************************************************************************/
    // These are roughly alphabetical by type as laid out here
    // http://www.6502.org/tutorials/6502opcodes.html

    // Adding
    inline void ADC();
    // Subtract
    inline void SBC();

    // Bitwise and
    inline void AND();
    // Exclusive bitwise or
    inline void EOR();
    // Bitwise or
    inline void ORA();

    // Left shift
    inline void ASL();
    // Left shift
    inline void LSR();

    // Bit test
    inline void BIT();

    // Branching
    inline void Branch();
    inline void BPL();
    inline void BMI();
    inline void BVC();
    inline void BVS();
    inline void BCC();
    inline void BCS();
    inline void BNE();
    inline void BEQ();

    // Break
    inline void BRK();

    // Comparing
    inline void Compare( int8_t reg );
    inline void CMP();
    inline void CPX();
    inline void CPY();

    // Decrementing
    inline void DEC();
    // Incrementing
    inline void INC();

    // Flag Setting
    inline void CLC();
    inline void SEC();
    inline void CLI();
    inline void SEI();
    inline void CLV();
    inline void CLD();
    inline void SED();

    // Jumping
    inline void JMP();
    inline void JSR();

    // Loading
    inline void LDA();
    inline void LDX();
    inline void LDY();

    // NOTHNG
    inline void NOP();

    // Register instructions
    inline void TAX();
    inline void TXA();
    inline void DEX();
    inline void INX();
    inline void TAY();
    inline void TYA();
    inline void DEY();
    inline void INY();

    // Rotate left
    inline void ROL();
    // Rotate right
    inline void ROR();

    // Return from interrupt
    inline void RTI();
    // Return from subroutine
    inline void RTS();

    // Store
    inline void STA();
    inline void STX();
    inline void STY();

    // Not 100% sure what the difference is between this and NOP. NOP seems to
    // be a formalized opcode where this is a placeholder
    inline void STP();

    // Stack instructions
    inline void TXS();
    inline void TSX();
    inline void PHA();
    inline void PLA();
    inline void PHP();
    inline void PLP();

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
        &CPUClass::imp, &CPUClass::imm, &CPUClass::acc, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imp, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::acc, &CPUClass::imm,
        &CPUClass::abs, &CPUClass::abs, &CPUClass::abs, &CPUClass::abs,
        &CPUClass::rel, &CPUClass::inY, &CPUClass::non, &CPUClass::inY,
        &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX, &CPUClass::zeX,
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
        &CPUClass::abX, &CPUClass::abX, &CPUClass::abX, &CPUClass::abX,
        &CPUClass::imp, &CPUClass::inX, &CPUClass::non, &CPUClass::inX,
        &CPUClass::zer, &CPUClass::zer, &CPUClass::zer, &CPUClass::zer,
        &CPUClass::imp, &CPUClass::imm, &CPUClass::acc, &CPUClass::imm,
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
        &CPUClass::imp, &CPUClass::abY, &CPUClass::imp, &CPUClass::abY,
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

enum Instructions
{
    BRK_IMP, ORA_INX, STP_NON, SLO_INX, NOP_ZER, ORA_ZER, ASL_ZER, SLO_ZER, PHP_IMP, ORA_IMM, ASL_ACC, ANC_IMM, NOP_ABS, ORA_ABS, ASL_ABS, SLO_ABS, BPL_REL, ORA_INY, STP_NON, SLO_INY, NOP_ZEX, ORA_ZEX, ASL_ZEX, SLO_ZEX, CLC_IMP, ORA_ABY, NOP_NON, SLO_ABY, NOP_ABX, ORA_ABX, ASL_ABX, SLO_ABX,
    JSR_ABS, AND_INX, STP_NON, RLA_INX, BIT_IMM, AND_IMM, ROL_IMM, RLA_IMM, PLP_IMP, AND_IMM, ROL_ACC, ANC_IMM, BIT_ABS, AND_ABS, ROL_ABS, RLA_ABS, BMI_REL, AND_INY, STP_NON, RLA_INY, NOP_ZEX, AND_ZEX, ROL_ZEX, RLA_ZEX, SEC_IMP, AND_ABY, NOP_NON, RLA_ABY, NOP_ABX, AND_ABX, ROL_ABX, RLA_ABX,
    RTI_IMP, EOR_INX, STP_NON, SRE_INX, NOP_ZER, EOR_ZER, LSR_ZER, SRE_ZER, PHA_IMP, EOR_IMM, LSR_ACC, ALR_IMM, JMP_ABS, EOR_ABS, LSR_ABS, SRE_ABS, BVC_REL, EOR_INY, STP_NON, SRE_INY, NOP_ZEX, EOR_ZEX, LSR_ZEX, SRE_ZEX, CLI_IMP, EOR_ABY, NOP_NON, SRE_ABY, NOP_ABX, EOR_ABX, LSR_ABX, SRE_ABX,
    RTS_IMM, ADC_INX, STP_NON, RRA_INX, NOP_ZER, ADC_ZER, ROR_ZER, RRA_ZER, PLA_IMP, ADC_IMM, ROR_ACC, ARR_IMM, JMP_IND, ADC_ABS, ROR_ABS, RRA_ABS, BVS_REL, ADC_INY, STP_NON, RRA_INY, NOP_ZEX, ADC_ZEX, ROR_ZEX, RRA_ZEX, SEI_IMP, ADC_ZEY, NOP_NON, RRA_ABY, NOP_ABX, ADC_ABX, ROR_ABX, RRA_ABX,
    NOP_IMM, STA_INX, NOP_IMM, SAX_INX, STY_ZER, STA_ZER, STX_ZER, SAX_ZER, DEY_IMP, NOP_IMM, TXA_IMP, XAA_IMM, STY_ABS, STA_ABS, STX_ABS, SAX_ABS, BCC_REL, STA_INY, STP_NON, AHX_INY, STY_ZEX, STA_ZEX, STX_ZEY, SAX_ZEY, TYA_IMP, STA_ABY, TXS_IMP, TAS_ABY, SHY_ABX, STA_ABX, SHX_ABY, AHX_ABY,
    LDY_IMM, LDA_INX, LDX_IMM, LAX_INX, LDY_ZER, LDA_ZER, LDX_ZER, LAX_ZER, TAY_IMP, LDA_IMM, TAX_IMP, LAX_IMM, LDY_ABS, LDA_ABS, LDX_ABS, LAX_ABS, BCS_REL, LDA_INY, STP_NON, LAX_INY, LDY_ZEX, LDA_ZEX, LDX_ZEY, LAX_ZEY, CLV_IMP, LDA_ABY, TSX_IMP, LAS_ABY, LDY_ABX, LDA_ABX, LDX_ABY, LAX_ABY,
    CPY_IMM, CMP_INX, NOP_IMM, DCP_INX, CPY_ZER, CMP_ZER, DEC_ZER, DCP_ZER, INY_IMP, CMP_IMM, DEX_IMP, AXS_IMM, CPY_ABS, CMP_ABS, DEC_ABS, DCP_ABS, BNE_REL, CMP_INY, STP_NON, NOP_ZEX, CMP_ZEX, DEC_ZEX, DEC_ZEX, DCP_ZEX, CLD_IMP, CMP_ABY, NOP_NON, DCP_ABY, NOP_ABX, CMP_ABX, DEC_ABX, DCP_ABX,
    CPX_IMM, SBC_INX, NOP_IMM, ISC_INX, CPX_ZER, SBC_ZER, INC_ZER, ISC_ZER, INX_IMP, SBC_IMM, NOP_NON, SBC_IMM, CPX_ABS, SBC_ABS, INC_ABS, ISC_ABS, BEQ_REL, SBC_INY, STP_NON, ISC_INY, NOP_ZEX, SBC_ZEX, INC_ZEX, ISC_ZEX, SED_IMP, SBC_ABY, NOP_NON, ISC_ABY, NOP_ABX, SBC_ABX, INC_ABX, ISC_ABX
}

#endif
