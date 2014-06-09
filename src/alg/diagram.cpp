#include <alg/diagram.hpp>
#include <limits.h>

namespace alg 
{
static int const c_window_size = 4;
        
        
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
         int min_index = 0;
         int min_grow = INT_MAX;
         
         for( int i = 0; i < num_points - c_window_size; ++i )
         {
                 int grow = dm_intergal[i + c_window_size] - dm_intergal[i];
                 if( grow < min_grow ) min_index = i, min_grow = grow;
                 else break;
         }         
         
         return min_index + c_window_size/2;
 }

}