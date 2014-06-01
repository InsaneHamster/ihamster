#pragma once
#include <cmn/fwd.hpp>

namespace alg
{
        //on output g8 image by applying Scharr operator (like Sobel which is famous but more precise)
        //[it finds edges of objects in img_src and is used as part of advanced watershed implementation] 
        //on input - rgba8
        void sobel( cmn::image_pt * img_edge, cmn::image_pt const img_src );
        
        void sobel_test();
}