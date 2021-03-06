#include <iostream>
#include <fstream>
#include <bitset>

#include "CPU.h"

int main( int argc, char* argv[] )
{
    auto begin = std::chrono::high_resolution_clock::now();
    CPUClass CPU = CPUClass();

    std::ifstream ROMImage( argv[1], std::ios::binary );
    CPU.run( ROMImage );

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();

    // std::cout << "Duration: " << duration << std::endl;

    std::bitset<8> stat( CPU.P );

    // Casts needed for cout to interpret correctly
    std::cout << "Cycles:   " << CPU.totalCycles << std::endl;
    std::cout << "PC:       " << unsigned( CPU.PC ) << std::endl;
    std::cout << "Stack:    " << unsigned (CPU.SP ) << std::endl;
    std::cout << "Acc:      " << int( CPU.A ) << std::endl;
    std::cout << "X:        " << int( CPU.X ) << std::endl;
    std::cout << "Y:        " << int( CPU.Y ) << std::endl;
    std::cout << "Stat:     " << stat << std::endl;
    std::cout << "Mem:      " << ( int* ) CPU.memory << std::endl;

    return 0;
}