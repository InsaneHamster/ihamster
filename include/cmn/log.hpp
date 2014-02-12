#pragma once

namespace cmn
{
        void log(char const * format, ...);
        
        //template< typename exc_type=std::runtime_error> 
        //void log_and_throw(char const * format, ...);
}