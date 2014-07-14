#pragma once
#include <cmn/fwd.hpp>

namespace alg
{
        //intended as part of sobel chain
        //divide/paint image into parts     
        //img_bw - image in format_bw gotten as sobel output (see cmn::image_utils how to get bw from grey)
        //return: image in format_g16 - where each value represents color;
        cmn::image_pt image_paint( cmn::image_pt const & img_bw );
                
        //tolerance - 0..1 the larger the bigger regions will be colored/painted
        //return: image in format_g16 - where each value represents color;
        cmn::image_pt image_paint_with_hint( cmn::image_pt const & img_sobel_g8, cmn::image_pt const & img_original, float tolerance );
        cmn::image_pt image_paint_with_hint2( cmn::image_pt const & img_sobel_g8, cmn::image_pt const & img_original, float tolerance );
        
        
        
}
