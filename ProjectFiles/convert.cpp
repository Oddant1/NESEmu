#include "convert.h"

int8_t convert( int8_t toConvert )
{
    if( toConvert < 0 )
    {
        return ( ( toConvert - 1 ) ^ 0b11111111 | 0b10000000 );
    }

    return toConvert;
}
