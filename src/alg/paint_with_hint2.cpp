#include <alg/paint.hpp>
#include <alg/line.hpp>
#include <alg/line2.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <vector>
#include <deque>
#include <string.h>
#include <math.h>
#include <algorithm>

namespace alg
{
      
using namespace cmn;

namespace 
{
        static int const c_tail = 4;            //pixels max    
        //static uint16_t c_busy = 0xffff;
        
        struct point_t
        {
                color3f_t       prev[c_tail];                
                point2s_t       last_pt;               //last point. we do not need entire tail
                int8_t          size = 0;              //actual size                
        };
        
        struct helper_t
        {
                image_t const *                 img_src;
                image_t const *                 img_sobel;
                image_t *                       img_dst;
                float                           tolerance;
                
                std::vector< point_t >          backtrace;
                uint16_t                        color = 0;                
                
                point3f_t                       sum;
                int                             sum_dots;
                
                //std::vector< uint16_t* >        backup;
        };
        
        
}

static void inherit( point_t * opt, point_t const & ipt, color3f_t const & addpt, point2s_t const pt_next  )
{        
        if( ipt.size == c_tail ) 
        {
                for( int i = 1; i < ipt.size; ++i )        
                        opt->prev[i-1] = ipt.prev[i];
                opt->prev[c_tail-1] = addpt;
                opt->size = c_tail;
        }
        else  
        {
                for( int i = 0; i < ipt.size; ++i )        
                        opt->prev[i] = ipt.prev[i];                                
                opt->prev[ipt.size] = addpt;
                opt->size = ipt.size + 1;
        }
                
        opt->last_pt = pt_next;                        
}

static void advance_if_can( helper_t & h, point_t * p_base, point3f_t const & exp, point2s_t pt_next )
{
        uint16_t & dst_cl = h.img_dst->row<uint16_t>( pt_next.y )[pt_next.x];        
        if( !dst_cl )
        {    //we can advance, check if we can by value
                color3f_t const & cl = h.img_src->row<color3f_t>( pt_next.y )[pt_next.x];
                float const dist_sq = exp.distance_sq(cl);   
                float const tolerance_sq = h.tolerance * h.tolerance;      //TODO: move in helper_t

                if( dist_sq < tolerance_sq )
                {
                        dst_cl = h.color;                        
                        h.backtrace.push_back( point_t() );
                        point_t & p_derived = h.backtrace.back();
                        
                        inherit( &p_derived, *p_base, cl, pt_next );
                        
                        h.sum += cl;
                        h.sum_dots++;
                }             
                else
                {
                        //dst_cl = c_busy;
                        //h.backup.push_back(&dst_cl);
                }
        }
}

static void advance( helper_t & h, point_t * p_base, point2s_t anchor, cmn::point3f_t const & exp, int const width, int const height )
{
        point2s_t const pl( anchor.x-1, anchor.y );
        point2s_t const pr( anchor.x+1, anchor.y );
        point2s_t const pu( anchor.x, anchor.y-1 );
        point2s_t const pd( anchor.x, anchor.y+1 );

        if( pl.x > 0 ) advance_if_can( h, p_base, exp, pl );
        if( pr.x < width ) advance_if_can( h, p_base, exp, pr );
        if( pu.y > 0 ) advance_if_can( h, p_base, exp, pu );
        if( pd.y < height ) advance_if_can( h, p_base, exp, pd );
        
}

static void fill( helper_t & h, int const start_x, int const start_y, int const width, int const height )
{
        color3f_t const anchor = h.img_src->row<color3f_t>(start_y)[start_x];        
        
        std::vector< point_t >  & backtrace = h.backtrace;
        backtrace.push_back(point_t());        
        {
                point_t & p_start = backtrace.back();
                ++p_start.size;
                p_start.prev[0] = anchor;
                p_start.last_pt = point2s_t(start_x, start_y);
        }

        h.sum = anchor;
        h.sum_dots = 1;

        
        cmn::point3f_t org, dir;        
        while( !backtrace.empty() )
        {
                point_t p = backtrace.back();
                backtrace.pop_back();
                                
                bool  use_last = !alg::linear_least_squares_parametic( &org, &dir, p.prev, p.size );                                 
                
                if( use_last )
                {
                        cmn::point3f_t const expected = anchor;//h.sum / h.sum_dots;
                        //h.tolerance = 0.03f*255.f;
                        advance( h, &p, p.last_pt, expected, width, height );                        
                }
                else
                {                           
                        //h.tolerance = 0.02f*255.f;//0.02f*255.f;
                        cmn::point3f_t const next = org + dir * c_tail;                        
                        advance( h, &p, p.last_pt, next, width, height );                        
                }                
        }
                
            
#if 0            
        for( size_t i = 0; i < h.backup.size(); ++i )
        {
                *(h.backup[i]) = 0;
        }                
        h.backup.clear();
#endif        
}


image_pt image_paint_with_hint2( cmn::image_pt const & img_sobel_g8, cmn::image_pt const & img_original, float tolerance )
{
        int const width = img_original->header.width;
        int const height = img_original->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, cmn::format_g16 );
        memset( img_dstp->bytes, 0, img_dstp->header.pitch * height );
        
        helper_t h;
        h.img_src = img_original.get();
        h.img_sobel = img_sobel_g8.get();
        h.img_dst = img_dstp.get();
        h.tolerance = tolerance * 255;
        
        for( int y = 0; y < height; ++y )
        {
                uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(y);
                uint16_t * const row_dst = h.img_dst->row<uint16_t>(y);
                for( int x = 0; x < width; ++x )
                {
                        if( !row_sobel[x] && !row_dst[x] )
                        {
                                ++h.color; 
                                if(!h.color) h.color++;         //overflow
                                row_dst[x] = h.color;
                                fill( h, x, y, width, height );
                        }
                }
        }
        
        
        return img_dstp;
}

}