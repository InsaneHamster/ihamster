#pragma once
#include <string>
#include <stdarg.h>
#include <stdexcept>

namespace cmn
{
        void log(char const * format, ...);
        void logs(char const * str);
        std::string log_in_string(char const * format, ...);
        std::string log_in_string_v( const char* format, va_list vl );

        
        template< typename exc_type = std::runtime_error >
        void log_and_throw(char const * format, ...)
        {
                va_list vl;
                va_start( vl, format );
                std::string s = log_in_string(format, vl);
                va_end(vl);
                logs(s.c_str());
                throw exc_type(s);
        }
}