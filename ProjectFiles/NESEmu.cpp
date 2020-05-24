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

    std::cout << "Duration: " << duration << std::endl;

    std::bitset<8> stat( CPU.status );

    // Casts needed for cout to interpret correctly
    std::cout << "PC:    " << unsigned( CPU.programCounter ) << std::endl;
    std::cout << "Stack: " << unsigned (CPU.stackPointer ) << std::endl;
    std::cout << "Acc:   " << int( CPU.accumulator ) << std::endl;
    std::cout << "X:     " << int( CPU.X ) << std::endl;
    std::cout << "Y:     " << int( CPU.Y ) << std::endl;
    std::cout << "Stat:  " << stat << std::endl;
    std::cout << "Mem:   " << ( int* ) CPU.memory << std::endl;
    for( int i = 0; i < 6; i++ )
    {
        std::cout << int( CPU.memory[ i ] ) << std::endl;
    }



    return 0;
}