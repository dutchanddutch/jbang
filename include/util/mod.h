#include "defs.h"

// euclidean remainder, i.e. mod( -1, 10 ) == 9

let inline mod( uint n, uint d ) -> uint
{
	return n % d;
}

let inline mod( int n, uint d ) -> uint
{
	if( n >= 0 )
		return (uint)n % d;
	return d + ~( ~(uint)n % d );
}
