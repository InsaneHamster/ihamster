#pragma once
#include <cmn/fwd.hpp>

namespace alg
{
        //intended as part of sobel chain
        //divide/paint image into parts     
        //img_bw - image in format_bw gotten as sobel output (see cmn::image_utils how to get bw from grey)
        //return: image in format_g16 - where each value represents color;
        image_pt paint( image_pt const & img_bw );
        
}
