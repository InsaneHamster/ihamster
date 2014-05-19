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
        fputc('\n', stderr);
}

void logs(char const * str)
{
        fputs(str, stderr);
}

std::string log_in_string(char const * format, ...)
{
        va_list list;
        va_start( list, format );
        std::string ret = log_in_string_v( format, list );
        va_end( list );
        return ret;
}

std::string log_in_string_v( const char* format, va_list vl )
{
        int n = vsnprintf( 0, 0, format, vl );
        std::string ret(n,' ');
		vsnprintf( &ret[0], n, format, vl );
        return ret;
}

}