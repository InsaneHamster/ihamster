#include <alg/paint.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <vector>
#include <deque>
#include <string.h>
#include <math.h>

namespace alg
{
      
using namespace cmn;        
        
namespace
{                
        struct helper_t
        {
                image_t *               img_dst;                
                image_t const *         img_sobel;      //g8
                image_t const *         img_org;        //original: lab or rgb
                std::vector<point2s_t>  rollback;               //return 0 as color on exit to these dots
                std::vector<point2s_t>  backtraces[256];                
                int                     backtace_dots;
                int                     min_layer;
                float                   tolerance;      //scaled up to 255
                
                uint16_t                color;
        };
}

//returns weight of where to insert the dot for exploring
static bool
try_advance( helper_t & h, point2s_t const pt )
{
        //if( pt.x<0 || pt.y<0 || pt.x >= size.x || pt.y >= size.y )
        //        return false;        
                        
        //color3f_t const * const row_org = h.img_org->row<color3f_t>(y);
        uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(pt.y);
        
        uint16_t * const row_dst = h.img_dst->row<uint16_t>(pt.y);
        uint16_t & cl = row_dst[pt.x];
        if( !cl )
        {
                cl = h.color;
                uint8_t layer = row_sobel[pt.x];
                h.backtraces[layer].push_back(pt);
                
                if( h.min_layer > layer )
                {
                        h.min_layer = layer;
                        return true;
                }
        }                
        
        return false;
}

static void
fill( helper_t & h, int const x, int const y )
{
        h.min_layer = 0;
        h.backtraces[0].emplace_back(x,y);        
        h.img_dst->row<uint16_t>(y)[x] = h.color;
        
        point2s_t const size(h.img_org->header.width, h.img_org->header.height);                
        
        color3f_t median = color3f_t(0,0,0);
        int num_dots = 0;
        
        
        while( h.min_layer < 256 )
        {        
                std::vector<point2s_t> * backtrace;
                backtrace = &h.backtraces[h.min_layer];
                                                        
                while( !backtrace->empty() )
                {                        
                        //check if we can accept the point ?                                                
                        point2s_t pt = backtrace->back();
                        backtrace->pop_back();

                        color3f_t const * const row_org = h.img_org->row<color3f_t>(pt.y);
                        uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(pt.y);
                        bool allow = true;
                        
                        if( pt.x == 234 && pt.y == 276 )
                                int z=10;
                        
                        if( !row_sobel[pt.x] )
                                median += row_org[x], ++num_dots;
                        else
                        {
                                //color3f_t const & cl = row_org[pt.x];
                                color3f_t cl = row_org[pt.x];
                                color3f_t expectation = median / num_dots;
                                                                
                                float diff = sqrtf(expectation.distance_sq(cl)) + row_sobel[pt.x]/60.f*h.tolerance;
                                if( diff > h.tolerance )
                                        allow = false;
                                //else
                                //        median += row_org[x], ++num_dots;
                        }
                        
                        if( allow )
                        {                        
                                point2s_t const p10(pt.x-1, pt.y);
                                point2s_t const p12(pt.x+1, pt.y);
                                point2s_t const p01(pt.x, pt.y-1);
                                point2s_t const p21(pt.x, pt.y+1);
                                                                                
                                if( p10.x >= 0 ) try_advance(h,p10);
                                if( p12.x < size.x ) try_advance(h,p12);
                                if( p01.y >= 0 ) try_advance(h,p01);
                                if( p21.y < size.y ) try_advance(h,p21);                                                
                        }
                        else                               
                                h.rollback.push_back(pt);
                        
                        backtrace = &h.backtraces[h.min_layer];                        
                }
                
                ++h.min_layer;                                
        }
        
        for( size_t i = 0; i < h.rollback.size(); ++i )
        {
                point2s_t pt = h.rollback[i];
                uint16_t * const row_dst = h.img_dst->row<uint16_t>(pt.y);
                row_dst[pt.x] = 0;
        }
        h.rollback.clear();
}


image_pt image_paint_with_hint( cmn::image_pt const & img_sobel_g8, cmn::image_pt const & img_original, float tolerance )
{
        int const width = img_original->header.width;
        int const height = img_original->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, cmn::format_g16 );        
        
        helper_t h;
        h.img_dst = img_dstp.get();        
        h.img_sobel = img_sobel_g8.get();
        h.img_org = img_original.get();
        h.backtace_dots = 0;
        h.min_layer = 0;
        h.tolerance = tolerance * 255.f;
        h.color = 0;
        
        memset( h.img_dst->bytes, 0, h.img_dst->header.height * h.img_dst->header.pitch );
        
        
        for( int y = 1; y < height-1; ++y )
        {
                color3f_t const * const row_org = h.img_org->row<color3f_t>(y);
                uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(y);
                uint16_t * const row_dst = h.img_dst->row<uint16_t>(y);
                for( int x = 1; x < width-1; ++x )
                {
                        if( !row_dst[x] && !row_sobel[x] )
                        {
                                ++h.color;
                                if( !h.color ) ++h.color;       //overflow...
                                fill(h, x, y);                                
                        }
                }
        }                
        
        return img_dstp;
}

}