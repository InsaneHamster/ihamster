#pragma once
#include <cmn/fwd.hpp>
#include <stdint.h>

namespace cmn
{
        image_pt image_rgba_from_g8( image_pt const gray );
        
        image_pt image_bw_from_g8( image_pt const & gray, uint8_t const border );
        
        image_pt image_hsva_from_rgba( image_pt const & img_srcp );
        image_pt image_rgba_from_hsva( image_pt const & img_srcp );
        
}
