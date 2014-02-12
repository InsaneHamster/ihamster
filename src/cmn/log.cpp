#include <cmn/log.hpp>
#include <stdarg.h>
#include <stdio.h>

namespace cmn
{

void log(char const * format, ...)
{
        va_list list;
        va_start( list, format );
        vfprintf( stderr, format, list );        
        va_end( list );
}

}