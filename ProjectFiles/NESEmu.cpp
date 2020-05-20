#include "CPU.h"
#include <iostream>

int main( int argc, char* argv[] )
{
    CPUClass CPU = CPUClass();

    // Casts needed for cout to interpret correctly
    std::cout << "PC:    " << unsigned(CPU.programCounter) << std::endl;
    std::cout << "Stack: " << unsigned(CPU.stackPointer) << std::endl;
    std::cout << "Acc:   " << int(CPU.accumulator) << std::endl;
    std::cout << "X:     " << int(CPU.X) << std::endl;
    std::cout << "Y:     " << int(CPU.Y) << std::endl;
    std::cout << "Stat:  " << unsigned(CPU.Status) << std::endl;
    std::cout << "Mem:   " << (int*) CPU.memory << std::endl;

    return 0;
}