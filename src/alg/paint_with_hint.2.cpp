#include <alg/paint.hpp>
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
        struct flat_t
        {
               std::vector<point2s_t> points;
               uint16_t               color;
               color3f_t              median;               
        };
        
        struct helper1_t
        {
                image_t *               img_dst;
                image_t const *         img_org;
                std::vector<flat_t>     objects;
                std::vector<point2s_t>  backtrace;                
                uint16_t                color = 0;
        };
        
        struct helper_t
        {
                image_t *               img_dst;                
                image_t const *         img_sobel;      //g8
                image_t const *         img_org;        //original: lab or rgb
                std::vector<flat_t>     objects;
                
                std::vector<point2s_t>  rollback;               //return 0 as color on exit to these dots
                std::vector<point2s_t>  backtraces[256];                        
                int                     min_layer;
                float                   tolerance;      //scaled up to 255                                
        };
        
}

static void try_advance1( helper1_t & h, point2s_t const pt )
{
        uint16_t * const row_dst = h.img_dst->row<uint16_t>(pt.y);
        uint16_t & cl = row_dst[pt.x];
        if( !cl )
        {
                cl = h.color;
                h.backtrace.push_back(pt);                
                h.objects.back().points.push_back(pt);
        }
}

static void fill1( helper1_t & h, int const x, int const y, int const width, int const height )
{       
        h.img_dst->row<uint16_t>(y)[x] = h.color;
        
        h.objects.push_back(flat_t());
        flat_t & f = h.objects.back();
        f.color = h.color;
        
        std::vector< point2s_t > & backtrace = h.backtrace;
        h.backtrace.push_back( point2s_t(x,y) );        
                
        color3f_t const anchor = h.img_org->row<color3f_t>(y)[x];
        f.median = anchor;
        
        while( !h.backtrace.empty() )
        {
                point2s_t const pt = backtrace.back();
                backtrace.pop_back();

                color3f_t const * const row_org = h.img_org->row<color3f_t>(pt.y);
                //uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(pt.y);
                                                                
                color3f_t const cl = row_org[pt.x];
                float diff = sqrtf(anchor.distance_sq(cl));
                bool allow = diff <= 3;
                
                if( allow )
                {                        
                        point2s_t const p10(pt.x-1, pt.y);
                        point2s_t const p12(pt.x+1, pt.y);
                        point2s_t const p01(pt.x, pt.y-1);
                        point2s_t const p21(pt.x, pt.y+1);
                                                                        
                        if( p10.x >= 0 ) try_advance1(h,p10);
                        if( p12.x < width ) try_advance1(h,p12);
                        if( p01.y >= 0 ) try_advance1(h,p01);
                        if( p21.y < height ) try_advance1(h,p21);                                                
                }                
        }
        
        if( f.points.size() < 20 )
                h.objects.pop_back();
}

static void pass1( std::vector<flat_t> * objects, image_t * img_dst, cmn::image_t const * img_org )
{
        int const width = img_org->header.width;
        int const height = img_org->header.height;
        helper1_t h;
        h.img_dst = img_dst;
        h.img_org = img_org;        
        
        for( int y = 1; y < height-1; ++y )
        {
                color3f_t const * const row_org = img_org->row<color3f_t>(y);
                //uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(y);  
                uint16_t * const row_dst = img_dst->row<uint16_t>(y);
                for( int x = 1; x < width-1; ++x )
                {
                        if( !row_dst[x] )
                        {
                                ++h.color;
                                if( !h.color ) ++h.color;       //overflow...                                
                                fill1(h, x, y, width, height);                                
                        }
                }
        }          
        
        //now sort all of these objects in a separate index and copy as result
        std::vector<int> ind;
        ind.resize(h.objects.size());
        int n = 0;
        auto gen = [n]() mutable -> int { return n++; };
        std::generate(ind.begin(), ind.end(), gen);
        
        auto s = [&h](int const a, int const b)->bool
        {
                return h.objects[a].points.size() > h.objects[b].points.size();                
        };
        std::sort(ind.begin(), ind.end(), s);
        
        //move all of this to output
        objects->resize(ind.size());
        for(size_t i = 0; i < ind.size(); ++i )
        {
                flat_t & dst = (*objects)[i];
                flat_t & src = h.objects[ ind[i] ];
                dst.color = src.color;
                dst.points.swap(src.points);
        }                
}

//returns weight of where to insert the dot for exploring
static bool
try_advance( helper_t & h, point2s_t const pt, uint16_t const color )
{
        //if( pt.x<0 || pt.y<0 || pt.x >= size.x || pt.y >= size.y )
        //        return false;        
                        
        //color3f_t const * const row_org = h.img_org->row<color3f_t>(y);
        uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(pt.y);
        
        uint16_t * const row_dst = h.img_dst->row<uint16_t>(pt.y);
        uint16_t & cl = row_dst[pt.x];
        if( !cl )
        {
                cl = color;
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

enum part_et
{
        part_s0,
        part_s1,
        part_line
};

float
point2segment_distance( part_et * part, color3f_t const p, color3f_t s0, color3f_t s1)
{
        color3f_t const v = s1-s0;
        color3f_t const w = p - s0;
        
        float const c1 = w.dot(v);
        if ( c1 <= 0 )
                return *part=part_s0, sqrtf( p.distance_sq(s0) );

        float const c2 = v.dot(v);
        if ( c2 <= c1 )
                return *part=part_s1, sqrtf(p.distance_sq(s1));

        float const b = c1 / c2;
        color3f_t pb = s0 + v*b;
        return *part=part_line, sqrtf(p.distance_sq(pb));
}

static void
fill( helper_t & h, flat_t & f )
{
        h.min_layer = 0;
        h.backtraces[0].swap(f.points);
        //h.backtraces[0].emplace_back(x,y);        
        //h.img_dst->row<uint16_t>(y)[x] = h.color;        
        
        //try to linear extrapolate...        
        color3f_t median1 = h.img_org->row<color3f_t>(h.backtraces[0].back().y)[h.backtraces[0].back().x];
        color3f_t median2 = median1;
        float       num_dots1 = 1;
        float      num_dots2 = 1;
                        
        point2s_t const size(h.img_org->header.width, h.img_org->header.height);
        while( h.min_layer < 256 )
        {        
                std::vector<point2s_t> * backtrace;
                backtrace = &h.backtraces[h.min_layer];
                                                        
                while( !backtrace->empty() )
                {                        
                        //check if we can accept the point ?                                                
                        point2s_t pt = backtrace->back();
                        backtrace->pop_back();
                        
                        //uint16_t * row_dst = h.img_dst->row<uint16_t>(pt.y);                        
                        //if( row_dst[pt.x] != f.color ) continue;
                        
                        color3f_t const * const row_org = h.img_org->row<color3f_t>(pt.y);
                        uint8_t const * const row_sobel = h.img_sobel->row<uint8_t>(pt.y);
                        
                        bool allow = true;
                                                        
                        if( 0 )//!row_sobel[pt.x] )
                        {
                                median1 += row_org[pt.x], ++num_dots1;
                        }
                        else
                        {
                                //color3f_t const & cl = row_org[pt.x];
                                color3f_t cl = row_org[pt.x];
                                color3f_t exp1 = median1 / num_dots1;
                                color3f_t exp2 = median2 / num_dots2;
                                part_et part;
                                
                                //float diff = sqrtf(expectation.distance_sq(cl)) + row_sobel[pt.x]/60.f*h.tolerance;
                                float diff = point2segment_distance( &part, cl, exp1, exp2 );// + row_sobel[pt.x]/60.f*h.tolerance;
                                if( diff > h.tolerance )
                                        allow = false;
                                else
                                {
                                        float v = (h.tolerance - diff)/h.tolerance;
                                        //median += row_org[x] * v; num_dots += v;

                                        if( part == part_s0 )
                                                median1 += cl*v, num_dots1+=v;
                                        else
                                                median2 += cl*v, num_dots2+=v;                                        
                                }
                        }
                        
                        if( allow )
                        {                        
                                point2s_t const p10(pt.x-1, pt.y);
                                point2s_t const p12(pt.x+1, pt.y);
                                point2s_t const p01(pt.x, pt.y-1);
                                point2s_t const p21(pt.x, pt.y+1);
                                                                                
                                if( p10.x >= 0 ) try_advance(h,p10,f.color);
                                if( p12.x < size.x ) try_advance(h,p12,f.color);
                                if( p01.y >= 0 ) try_advance(h,p01,f.color);
                                if( p21.y < size.y ) try_advance(h,p21,f.color);                                                
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


image_pt image_paint_with_hint2( cmn::image_pt const & img_sobel_g8, cmn::image_pt const & img_original, float tolerance )
{
        int const width = img_original->header.width;
        int const height = img_original->header.height;
        
        image_pt img_dstp = cmn::image_create( width, height, pitch_default, cmn::format_g16 );        
        
        helper_t h;
        h.img_dst = img_dstp.get();        
        h.img_sobel = img_sobel_g8.get();
        h.img_org = img_original.get();        
        h.min_layer = 0;
        h.tolerance = tolerance * 255.f;        
        
        memset( h.img_dst->bytes, 0, h.img_dst->header.height * h.img_dst->header.pitch );
        pass1( &h.objects, h.img_dst, h.img_org );
        memset( h.img_dst->bytes, 0, h.img_dst->header.height * h.img_dst->header.pitch );
        
        for( size_t i = 0; i < h.objects.size(); ++i )
        {
                flat_t & f = h.objects[i];
                fill( h, f );
        }

        
        return img_dstp;
}

}