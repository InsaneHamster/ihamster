#pragma once
#include <cmn/fwd.hpp>
#include <stdint.h>

namespace cmn
{
        image_pt image_rgba_from_g8( image_pt const gray );
        
        image_pt image_bw_from_g8( image_pt const & gray, uint8_t const border );
        
}
