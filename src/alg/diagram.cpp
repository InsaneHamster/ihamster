#include <alg/diagram.hpp>
#include <limits.h>
#include <algorithm>

namespace alg 
{

static float c_take_factor = 0.2f;

void diagram_make_integral(int * dm, int const num_points)
{
         int acc = 0;
         for( int i = 0; i < num_points; ++i )
         {
                 acc += dm[i];
                 dm[i] = acc;
         }
}

int diagram_find_cutting_point(int const * dm_intergal, int const num_points)
{
        if( !num_points )
                return 0;
        
        int const max_val = dm_intergal[num_points-1];
        int const find_val = max_val - max_val * c_take_factor;
        
        int const * pvalue = std::lower_bound( dm_intergal, dm_intergal + num_points, find_val );
        if( pvalue == dm_intergal + num_points )        
                return 0;       // I can't believe
                
        return (int)(pvalue - dm_intergal);
}

}