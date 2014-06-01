#include <cmn/image_utils.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>

namespace cmn
{

image_pt image_rgba_from_g8( image_pt const gray )
{
        image_header_t const & hs = gray->header;
        image_pt img_destp = cmn::image_create( hs.width, hs.height, pitch_default, format_rgba );
        image_t * const img_dest = img_destp.get();
        
        for( int y = 0; y < hs.height; ++y )
        {
                uint8_t const * const row_src = gray->row<uint8_t>(y);
                color4b_t * const row_dst = img_dest->row<color4b_t>(y);
                for( int x = 0; x < hs.width; ++x )
                {
                        uint8_t c = row_src[x];                        
                        row_dst[x] = color4b_t(c,c,c,255);
                }
        }
        
        return img_destp;
}
        
}
