#ifndef CPU_H
#define CPU_H

#include <stdint.h>

class CPUClass
{
    // Very much subject to change
    public:

    // CPU Registers
    // These will probably be interacted with in hex
    uint16_t programCounter = 0;
    uint8_t stackPointer = 0xFF;
    uint8_t Accumulator = 0;
    uint8_t X = 0;
    uint8_t Y = 0;

    // This will probably be interacted with using bitwise operations
    uint8_t Status = 0;

    uint8_t Memory [8192] = {};

    // Now the real question, are all opcodes just methods of this class?
    // Probably. That's a lot of methods. . . Also are they inline?
};

#endif
