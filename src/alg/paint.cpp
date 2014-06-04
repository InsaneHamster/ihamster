#include <alg/paint.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <vector>
#include <string.h>

namespace alg
{
      
using namespace cmn;        
        
namespace
{
        struct helper_t
        {
                image_t *               img_dst;
                image_t const *         img_src;
                std::vector<point2w_t>  backtrace;
                uint16_t                color;
        };
}

static bool 
try_advance( helper_t & h, point2w_t const pt, point2w_t const size )
{
        if( pt.x<0 || pt.y<0 || pt.x >= size.x || pt.y >= size.y )
                return false;        
                
        if( !image_bw_readpixel( h.img_src, pt.x, pt.y ) )
        {
                uint16_t * const row_dst = h.img_dst->row<uint16_t>(pt.y);
                uint16_t & cl = row_dst[pt.x];
                if( !cl )
                {
                        cl = h.color;
                        return true;
                }
        }
        
        return false;
}

static void
fill( helper_t & h, int const x, int const y )
{
        h.backtrace.emplace_back(x,y);
        std::vector<point2w_t> & backtrace = h.backtrace;        
        point2w_t const size(h.img_src->header.width, h.img_src->header.height);
        
        h.img_dst->row<uint16_t>(y)[x] = h.color;
        do
        {
                point2w_t pt = backtrace.back();
                backtrace.pop_back();
                
                point2w_t const p10(pt.x-1, pt.y);
                point2w_t const p12(pt.x+1, pt.y);
                point2w_t const p01(pt.x, pt.y-1);
                point2w_t const p21(pt.x, pt.y+1);
                
                if(try_advance(h,p10,size)) backtrace.push_back(p10);
                if(try_advance(h,p12,size)) backtrace.push_back(p12);
                if(try_advance(h,p01,size)) backtrace.push_back(p01);
                if(try_advance(h,p21,size)) backtrace.push_back(p21);                
        } while( !backtrace.empty() );
}

image_pt image_paint( image_pt const & img_bw )
{
        int const width = img_bw->header.width;
        int const height = img_bw->header.height;
        
        image_pt img_destp = cmn::image_create( width, height, pitch_default, cmn::format_g16 );                
        helper_t h;
        h.img_dst = img_destp.get();
        h.img_src = img_bw.get();
        h.backtrace.reserve(width*height);
        h.color = 0;
        
        memset( h.img_dst->bytes, 0, h.img_dst->header.height * h.img_dst->header.pitch );
        
        for( int y = 0; y < height; ++y )
        {
                uint8_t const * const row_src = h.img_src->row<uint8_t>(y);
                uint16_t * const row_dst = h.img_dst->row<uint16_t>(y);
                for( int x = 0; x < width; ++x )
                {
                        if( !row_dst[x] && !image_bw_readpixel(row_src,x) )
                        {
                                ++h.color;
                                fill(h, x, y);
                        }
                }
        }
        
        return img_destp;
}
        
}