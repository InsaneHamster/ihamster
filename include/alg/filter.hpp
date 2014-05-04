#pragma once
#include <cmn/fwd.hpp>

namespace alg
{
        //median filter to reduce noice
        //window 3x3
        cmn::image_pt filter_median( cmn::image_pt img );
        
        
}