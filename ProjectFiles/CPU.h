#ifndef CPU_H
#define CPU_H

#include <stdint.h>

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

class CPUClass
{
    // TODO: Need to make the CPU keep time somehow
    // Very much subject to change
    public:

    // CPU Registers
    // These will probably be interacted with in hex
    uint16_t programCounter = 0x34;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100
    uint8_t stackPointer = 0xFF;
    int8_t accumulator = 0x0;
    int8_t X = 0x0;
    int8_t Y = 0x0;

    // This will probably be interacted with using bitwise operations
    uint8_t Status = 0xFD;

    uint8_t memory [0x10000] = {};

    // TODO: Will probably end up returning some kind of error code
    void run();

    void fetch();
    void decode();
    void execute();

    // Now the real question, are all opcodes just methods of this class?
    // Probably. That's a lot of methods. . . Also are they inline?

    // CPU OpCode methods
    // TODO: Will probably end up returning some kind of error code
    void ADC_Immediate();   // $69
    void ADC_Zero_Page();   // $65
    void ADC_Zero_Page_X(); // $75
    void ADC_Absolute();    // $6D
    void ADC_Absolute_X();  // $7D
    void ADC_Absolute_Y();  // $79
    void ADC_Indirect_X();  // $61
    void ADC_Indirect_Y();  // $71
};

#endif
