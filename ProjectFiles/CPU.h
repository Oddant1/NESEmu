#ifndef CPU_H
#define CPU_H

#include <stdint.h>

class CPUClass
{
    // TODO: Need to make the CPU keep time somehow
    // Very much subject to change
    public:

    // CPU Registers
    // These will probably be interacted with in hex
    // TODO: Figure out where/how to initialize program counter
    uint16_t programCounter = 0;
    // Stack starts at 0x01FF and goes down to 0x0100. Pointer is offset from
    // 0x0100
    uint8_t stackPointer = 0xFF;
    uint8_t accumulator = 0;
    uint8_t X = 0;
    uint8_t Y = 0;

    // This will probably be interacted with using bitwise operations
    uint8_t Status = 0;

    uint8_t memory [8192] = {};

    // TODO: Will probably end up returning some kind of error code
    void run();

    void fetch();
    void decode();
    void execute();

    // Now the real question, are all opcodes just methods of this class?
    // Probably. That's a lot of methods. . . Also are they inline?

    // CPU OpCode methods
    // TODO: Will probably end up returning some kind of error code
    void CPUClass::ADC_Immediate();
};


#endif
