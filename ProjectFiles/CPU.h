#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <fstream>

typedef int ( *voidFunc )();

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

enum IndexingMode
{
    NONE,
    WITH_X,
    WITH_Y
};

class CPUClass
{
    // TODO: Need to make the CPU keep time somehow
    // Very much subject to change
    public:

    /***************************************************************************
    * CPU Registers
    ***************************************************************************/
    // These will probably be interacted with in hex
    uint16_t programCounter = 0x0000;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100
    uint8_t stackPointer = 0xFF;
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
    voidFunc instructionRegister;

    /***************************************************************************
    * Mapped memory
    ***************************************************************************/
    uint8_t memory [0x10000] = {};

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
    void updateCarry( int8_t oldAccumulator );

    /***************************************************************************
    * Handle addressing mode operand resolution
    ***************************************************************************/
    inline int8_t immediate();
    uint16_t zeroPage( IndexingMode mode );
    uint16_t absolute( IndexingMode mode );
    uint16_t indirect( IndexingMode mode );
    // We may need one for relative, but I don't think so
    int8_t retrieveIndexOffset( IndexingMode mode );


    // Now the real question, are all opcodes just methods of this class?
    // Probably. That's a lot of methods. . . Also are they inline?

    /***************************************************************************
    * CPU OpCode methods
    ***************************************************************************/
    // TODO: Maybe these return an int indicating how many cycles they are
    // supposed to take?
    void ADCImmediate(); // $69
    void ADCZeroPage();  // $65
    void ADCZeroPageX(); // $75
    void ADCAbsolute();  // $6D
    void ADCAbsoluteX(); // $7D
    void ADCAbsoluteY(); // $79
    void ADCIndirectX(); // $61
    void ADCIndirectY(); // $71
};

#endif
