#include "vrandn.h"

// Doesn't matter what this initializes to, as long as it is not zero
uint32_t lfsr;

void vsrand( uint32_t seed )
{
    if( seed )
    {
        lfsr = seed;
    }
}

uint32_t vrand( void )
{
    // Pseudo random number algorithm
	lfsr = ( ( 0x01 & lfsr>>1 ) ^ ( 0x01 & lfsr ) )<<31 | lfsr>>1;

	return lfsr;
}
