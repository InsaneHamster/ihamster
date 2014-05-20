#pragma once
#include <string>
#include <stdarg.h>
#include <stdexcept>
//#include "apply.hpp"

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
        
        template< typename exc_type >
        void log_and_throw( int err, char const * format, ...)
        {
                va_list vl;
                va_start( vl, format );
                std::string s = log_in_string(format, vl);
                va_end(vl);
                logs(s.c_str());
                throw exc_type(err, s);
        }

        
#if 0   //write me     
        template< typename exc_type, typename ...Z >
        static exc_type exception_builder( Z... build)
        {
                return exc_type(build...); 
        };                

        
        template< typename exc_type, typename Tuple >
        void log_and_throw( Tuple const & t, char const * format, ... )
        {
                va_list vl;
                va_start( vl, format );
                std::string s = log_in_string(format, vl);
                va_end(vl);
                logs(s.c_str());
                
                auto readyt = std::tuple_cat( t, std::make_tuple(s) );

                throw cmn::apply( exception_builder, readyt );
        }
#endif
                        
}