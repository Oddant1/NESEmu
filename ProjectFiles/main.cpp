#include "CPU.h"
#include <iostream>

int main( int argc, char* argv[] )
{
    CPUClass CPU = CPUClass();

    // Casts needed for cout to interpret correctly
    std::cout << "PC:    " << unsigned(CPU.programCounter) << std::endl;
    std::cout << "Stack: " << unsigned(CPU.stackPointer) << std::endl;
    std::cout << "Acc:   " << unsigned(CPU.Accumulator) << std::endl;
    std::cout << "X:     " << unsigned(CPU.X) << std::endl;
    std::cout << "Y:     " << unsigned(CPU.Y) << std::endl;
    std::cout << "Stat:  " << unsigned(CPU.Status) << std::endl;
    std::cout << "Mem:   " << (int*) CPU.Memory << std::endl;

    return 0;
}