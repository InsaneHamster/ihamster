#pragma once

namespace cmn 
{
        //N must be power of two
        template< typename T, int alignment > 
        T align_up( T val )
        {
                return ((val - 1) & ~(alignment - 1)) + alignment;
        }
}