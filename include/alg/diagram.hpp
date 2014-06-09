#pragma once

namespace alg
{

        //makes accumulative diagram, that is dm_intergal[1] = dm[1] + dm[1], dm_intergal[2] = dm[2] + dm_intergal[1] so on
        void diagram_make_integral(int * dm, int const num_points);

        //returns point which can be used to separate background from foreground (used in sobel/scharr algorithms)
        int diagram_find_cutting_point(int const * dm_intergal, int const num_points);
        
        
}