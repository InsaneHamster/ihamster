#include <adapter/plotcirc.hpp>
#include <adapter/image.hpp>
#include <cmn/plotcirc.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <memory.h>
#include <math.h>

namespace adapter 
{
        
        
bool plotcirc_save_to_png( cmn::plotcirc_pt const & pc, char const * const szImgPath )
{
        //what size ot chose ? let it be 256x128! (just for great justice)
        //or precisely plotcirc_discr * 4
        int const scale = 4;
        int const width = cmn::plotcirc_discr * scale * 2, height = cmn::plotcirc_discr * scale;
        cmn::image_pt img_p = cmn::image_create( width, height, cmn::pitch_default, cmn::format_rgba );
        cmn::image_t * img = img_p.get();
        //memset( img->bytes, 0, img->header.pitch * height );            //make it very black and transparent
        //upd: transparent looks bad in most viewers...
        
        cmn::color4b_t const white(255,255,255,255);
        cmn::color4b_t const black(0,0,0,255);
        
        {
                cmn::color4b_t * row_dst_begin = (cmn::color4b_t *)img->bytes;
                cmn::color4b_t * const row_dst_end = img->row<cmn::color4b_t>( height-1 ) + img->header.width;
                while( row_dst_begin < row_dst_end ) *row_dst_begin++ = black;
        }
        
        for( int y = 0; y < height; ++y )
        {
                cmn::color4b_t * const row_dst = img->row<cmn::color4b_t>(y);
                int const row_index_src = y / scale;
                std::vector< cmn::point2f_t > const & row_src = pc->rows[row_index_src];                
                int const row_src_size = row_src.size();                                
                
                //make-half transparent drawing...                
                for( int i = 0; i < row_src_size; ++i )
                {
                        cmn::point2f_t seg = row_src[i];
                        seg = seg * width;
                        
                        //now fill it with white
                        //first and last points draw with help of anti-aliasing                        
                        float fint;
                        float frac = modff( seg.x, &fint );
                        float intensity = 1.f - frac;
                        
                        int pos_write = (int)fint;
                        row_dst[pos_write] = white * intensity;
                        ++pos_write;
                        
                        intensity = modff( seg.y, &fint );
                        int pos_stop = ceilf(seg.y);
                        
                        while( pos_write < pos_stop )                        
                                row_dst[pos_write++] = white;
                        if( pos_stop < width && intensity ) //tail, add to value which is there to correctly handle case when length is less than 1 pixel!
                                row_dst[pos_stop] += white * intensity, row_dst[pos_stop].a = 255;  
                }                
        }
        
        bool ok = adapter::image_save_to_png( img_p, szImgPath );        
        
        return ok;
}
        
        
} //adapter