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


image_pt image_bw_from_g8( image_pt const & gray, uint8_t const border )
{
        image_t * img_src = gray.get();
        image_header_t const & hs = img_src->header;
        int const width = hs.width;
        int const height = hs.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_bw );
        image_t * img_dst = img_dstp.get();
        
        for( int y = 0; y < height; ++y )
        {
                uint8_t const * const row_src= img_src->row<uint8_t>(y);
                uint8_t * const row_dst = img_dst->row<uint8_t>(y);
                
                for( int x = 0; x < width; ++x )
                {
                        bool value = row_src[x] > border;
                        image_bw_writepixel( row_dst, x, border );
                }
        }
        
        return img_dstp;
}

}
