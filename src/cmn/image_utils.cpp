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
                        image_bw_writepixel( row_dst, x, value );
                }
        }
        
        return img_dstp;
}

image_pt image_hsva_from_rgba( image_pt const & img_srcp )
{
        image_t const * const img_src = img_srcp.get();
        int const width = img_src->header.width;
        int const height = img_src->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_hsva );
        image_t * const img_dst = img_dstp.get();
        
        
        for( int y = 0; y < height; ++y )
        {
                color4b_t const * const row_src = img_src->row<color4b_t>(y);
                color4b_t * const row_dst = img_dst->row<color4b_t>(y);
                
                for( int x = 0; x < width; ++x )
                {
                        color4b_t const cs = row_src[x];
                        color4b_t & cd = row_dst[x];
                        int sel;
                        
                        int const maxc1 = cs.r > cs.g ? (sel=0, cs.r) : (sel=1, cs.g);                        
                        int const maxc = maxc1 > cs.b ? maxc1 : (sel=2, cs.b);
                        
                        int const minc1 = cs.r < cs.g ? cs.r : cs.g;
                        int const minc = minc1 < cs.b ? minc1 : cs.b;
                        
                        int const c = maxc - minc;
                                                
                        if( c )
                        {
                                //42 !!!!
                                switch(sel)
                                {
                                        case 1: cd.r = ((int)cs.b - (int)cs.r)*42 / c + 2*42; break;
                                        case 0: cd.r = ((int)cs.g - (int)cs.b)*42 / c; break;       //auto-clamping for free!
                                        default:cd.r = ((int)cs.r - (int)cs.g)*42 / c + 4*42; break;                                        
                                }                                                                                                
                        }
                        else
                                cd.r = 0;
                        
                        if( maxc )                        
                              cd.g = 255 - (((minc << 8) - minc) / maxc);                        
                        else
                                cd.g = 0;
                        
                        cd.b = maxc;                                                        
                        cd.a = cs.a;
                }
        }
        
        return img_dstp;
};

image_pt image_rgba_from_hsva( image_pt const & img_srcp )
{
        image_t const * const img_src = img_srcp.get();
        int const width = img_src->header.width;
        int const height = img_src->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, format_rgba );
        image_t * const img_dst = img_dstp.get();
        
        
        for( int y = 0; y < height; ++y )
        {
                color4b_t const * const row_src = img_src->row<color4b_t>(y);
                color4b_t * const row_dst = img_dst->row<color4b_t>(y);
                
                for( int x = 0; x < width; ++x )
                {                                  
                        color4b_t const cs = row_src[x];
                        color4b_t & cd = row_dst[x];
                        int const sel = cs.r / 42;
                        int const vmin = (255-cs.g)*cs.b/255;
                        int const a = (cs.b - vmin) * ( cs.a % 42 ) / 42;
                        int const vinc = vmin + a;
                        int const vdec = cs.b - a;
                        
                        switch(sel)
                        {
                                case 0: case 6: cd.r = cs.b; cd.g = vinc; cd.b = vmin; break;
                                case 1: cd.r = vdec; cd.g = cs.b; cd.b = vmin; break;
                                case 2: cd.r = vmin; cd.g = cs.b; cd.b = vinc; break;
                                case 3: cd.r = vmin; cd.g = vdec; cd.b = cs.b; break;
                                case 4: cd.r = vinc; cd.g = vmin; cd.b = cs.b; break;
                                case 5: cd.r = cs.b; cd.g = vmin; cd.b = vdec; break;
                        }                                                
                        cd.a = cs.a;
                }
        }
        
        return img_dstp;
}


}
