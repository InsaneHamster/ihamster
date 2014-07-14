#pragma once
#include <cmn/point.hpp>

namespace cmn 
{
        //N must be power of two
        template< typename T, int alignment > 
        T align_up( T val )
        {
                return ((val - 1) & ~(alignment - 1)) + alignment;
        }
        
        
        //gcc's std::accumulate has bug: order of input variables is incorrect
        //so its possible to sum identical types but impossible to sum different
        template<typename T, typename Q, typename Fn> static T
        accumulate( Q const * pt, int const n, Fn && fn )
        {
                Q const * end = pt + n;
                T sum = 0;
                for( ;pt != end; ++pt )
                        sum = fn( *pt, sum );
                return sum;
        }
}