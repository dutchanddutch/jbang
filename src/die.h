#pragma once
#include "defs.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

[[ noreturn ]]
let static die( char const *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	exit( EXIT_FAILURE );
}
