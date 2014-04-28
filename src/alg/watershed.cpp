#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/util.hpp>
#include <string.h>


static short    const   c_ws_max_pix_diff = 30;                 //if difference is less, we assume it's a same object (watershed)
static short    const   c_ws_crit_pix_diff = 640;               //if difference is larger its definitely a sharp border
static double   const   c_pl_max_square = 0.001;                //amount of square after which we think it's a separate object (plain filling)
static short    const   c_pl_max_pix_diff = 50; //30;           //same as c_ws_max_pix_diff but during plain filling. also module is taken (only positive values acceptable)

namespace alg
{
using namespace cmn;

namespace
{
        
struct helper_t
{
        //std::vector< watershed_object_t > * objects;
        image_t *               img = 0;
        
        image_pt                img_quantized;             
        rect2i_t                bounds;        
        std::vector<point2i_t>  backtrace;
        std::vector<point2i_t>  lookup;        
        
        uint16_t                color = 0;              //increases, current color to use        
};

} //end of anonymous namespace

static const uint16_t cl_empty = 0;
static const uint16_t cl_handled = -1;

static void fill_backtrace( helper_t & h, uint16_t found_color )
{
        std::vector< point2i_t > & backtrace = h.backtrace;
        size_t size = backtrace.size();
        point2i_t * pts = &backtrace[0];
        point2i_t * pts_end = pts + size;
        
        int const pitch = h.img_quantized->header.pitch;
        uint8_t * bytes = h.img_quantized->bytes;
        
        for( ; pts < pts_end; ++pts  )
        {
                uint16_t * pos = (uint16_t*)( bytes + pts->y * pitch + pts->x * sizeof( uint16_t ) );
                *pos = found_color;                                
        }
        backtrace.clear();
        h.lookup.clear();       //we don't need to lookup anything anymore        
}

static void fill_backtrace( helper_t & h, uint16_t found_color, int x, int y )
{        
        std::vector< point2i_t > & backtrace = h.backtrace;
        backtrace.emplace_back( x, y );
        fill_backtrace( h, found_color );
}

#if 1
//finds object where gradient / luminance is roughly constant
static void mark_flat_objects( helper_t & h )
{
#define grad() (((short)cl_next.r - (short)cl_cur.r)*3  + ((short)cl_next.g - (short)cl_cur.g)*6 + ((short)cl_next.b - (short)cl_cur.b))
//#define grad() (((short)cl_next.g - (short)cl_cur.g))                
        image_header_t const hs = h.img->header;
        image_header_t const hd = h.img_quantized->header;              
        
        int64_t const square = hs.width * hs.height;
        int64_t const critical_square = square * c_pl_max_square;
                
        image_pt img_bitplain = image_create( hs.width, hs.height, pitch_default, format_bw );  
        memset( img_bitplain->bytes, 0, img_bitplain->header.pitch * img_bitplain->header.height );
        
        std::vector<point2i_t>  & backtrace = h.backtrace;
        std::vector<point2i_t>  & lookup = h.lookup;
        
        for( int y = 0; y < hs.height-1; ++y )
        {
                cmn::color4b_t const * const rs = h.img->row<cmn::color4b_t>(y);
                //cmn::color4b_t const * const rs_next = (cmn::color4b_t const *)((size_t)rs + hs.pitch);                
                //uint16_t * const rd = h.img_quantized->row<uint16_t>(y);
                uint8_t * bp = img_bitplain->row<uint8_t>(y);
                                
                for( int x = 0; x < hs.width-1; ++x )
                {                        
                        if( image_bw_readpixel( bp, x ) ) 
                                continue;           //we have handled this case already
                        
                        image_bw_writepixel( bp, x, true );
                        backtrace.emplace_back( x, y );
                        //color4b_t const & cl_cur = rs[x];
                        
                        int ny = y, nx = x;                                                    
                        do
                        {                                
                                cmn::color4b_t const * const nrs = h.img->row<cmn::color4b_t>(ny);
                                cmn::color4b_t const * const nrs_next = (cmn::color4b_t const *)((size_t)nrs + hs.pitch);
                                cmn::color4b_t const * const nrs_prev = (cmn::color4b_t const *)((size_t)nrs - hs.pitch);
                                
                                uint8_t * nbp = img_bitplain->row<uint8_t>(ny);
                                uint8_t * nbp_next = nbp + img_bitplain->header.pitch;
                                uint8_t * nbp_prev = nbp - img_bitplain->header.pitch;

//                                 uint16_t * const nrd = h.img_quantized->row<uint16_t>(ny);
//                                 uint16_t * const nrd_next = (uint16_t*)((size_t)nrd + hd.pitch);
//                                 uint16_t * const nrd_prev = (uint16_t*)((size_t)nrd - hd.pitch);
                                
                                color4b_t const & cl_cur = nrs[nx];
                                                
                                if( (nx+1) < hs.width && !image_bw_readpixel( nbp, nx+1 ) )
                                {
                                        //image_bw_writepixel( nbp, nx+1, true );
                                        color4b_t const & cl_next = nrs[nx+1];                                
                                        short diff = grad();
                                        if( diff < 0 ) diff = -diff;
                                        if( diff <= c_pl_max_pix_diff )
                                        {       //accept it!
                                                image_bw_writepixel( nbp, nx+1, true );
                                                backtrace.emplace_back( nx+1, ny );
                                                lookup.emplace_back( nx+1, ny );                                                
                                        }
                                }
                                
                                if( (ny+1) < hs.height && !image_bw_readpixel( nbp_next, nx ) )
                                {
                                        //image_bw_writepixel( nbp_next, nx, true );
                                        color4b_t const & cl_next = nrs_next[nx];
                                        short diff = grad();
                                        if( diff < 0 ) diff = -diff;
                                        if( diff <= c_pl_max_pix_diff )
                                        {       //accept it!
                                                image_bw_writepixel( nbp_next, nx, true );
                                                backtrace.emplace_back( nx, ny + 1 );
                                                lookup.emplace_back( nx, ny + 1 );                                                
                                        }
                                }
                                
                                if( nx > 0 && !image_bw_readpixel( nbp, nx-1 ) )
                                {
                                        //image_bw_writepixel( nbp, nx+1, true );
                                        color4b_t const & cl_next = nrs[nx-1];                                
                                        short diff = grad();
                                        if( diff < 0 ) diff = -diff;
                                        if( diff <= c_pl_max_pix_diff )
                                        {       //accept it!
                                                image_bw_writepixel( nbp, nx-1, true );
                                                backtrace.emplace_back( nx-1, ny );
                                                lookup.emplace_back( nx-1, ny );                                                
                                        }                                       

                                }
                                
                                if( ny > 0 && !image_bw_readpixel( nbp_prev, nx ) )
                                {
                                        //image_bw_writepixel( nbp_next, nx, true );
                                        color4b_t const & cl_next = nrs_prev[nx];
                                        short diff = grad();
                                        if( diff < 0 ) diff = -diff;
                                        if( diff <= c_pl_max_pix_diff )
                                        {       //accept it!
                                                image_bw_writepixel( nbp_prev, nx, true );
                                                backtrace.emplace_back( nx, ny - 1 );
                                                lookup.emplace_back( nx, ny - 1 );                                                
                                        }

                                }
                                                                                                
                                if( !lookup.empty() )
                                {
                                        point2i_t const & tail = lookup.back();                
                                        nx = tail.x;
                                        ny = tail.y;
                                        lookup.pop_back();
                                }
                                else 
                                {
                                        if( backtrace.size() > critical_square )
                                               fill_backtrace( h, ++h.color );
                                        else
                                               backtrace.clear();
                                        break;
                                }                                                                
                        } while( 1 );                        
                } //for x
        } //for y

#undef grad        
}
#endif


static void 
pool( helper_t & h, int base_x, int base_y )
{
#define grad() (((short)cl_next.r - (short)cl_cur.r)*3  + ((short)cl_next.g - (short)cl_cur.g)*6 + ((short)cl_next.b - (short)cl_cur.b)) // + c_ws_max_pix_diff
//        #define grad() ((short)cl_next.g - (short)cl_cur.g)
        
        cmn::color4b_t cl_next;
        uint16_t       cd;
        
        std::vector<point2i_t>  & backtrace = h.backtrace;
        std::vector<point2i_t>  & lookup = h.lookup;                
        int x = base_x;
        int y = base_y;
        color4b_t cl_cur;
        
        size_t img_dst_pitch = h.img_quantized->header.pitch;
        size_t img_src_pitch = h.img->header.pitch;
                
        while(1)               
        { 
                cmn::color4b_t const * row_src = h.img->row<cmn::color4b_t>(y);
                cmn::color4b_t const * row_src_next = (cmn::color4b_t const *)(((size_t)row_src) + img_src_pitch);
                cmn::color4b_t const * row_src_prev = (cmn::color4b_t const *)(((size_t)row_src) - img_src_pitch);
                
                uint16_t * row_dst = h.img_quantized->row<uint16_t>(y);                                                
                uint16_t * row_dst_next = (uint16_t *)(((size_t)row_dst) + img_dst_pitch);
                uint16_t * row_dst_prev = (uint16_t *)(((size_t)row_dst) - img_dst_pitch);
                
                cl_cur = row_src[x];
                
                bool add_backtrace = false;                
                
                //right
                if( (x+1) < h.img->header.width )
                {
                        cd = row_dst[x+1];
                        if( cd != cl_handled )
                        {
                                cl_next = row_src[x+1];                        
                                short val = grad();
                        
                                if( val >= -c_ws_max_pix_diff && val < c_ws_crit_pix_diff )
                                {
                                        if( !cd )
                                        {
                                                lookup.emplace_back( x+1, y );
                                                add_backtrace = true;
                                        }
                                        else
                                        {
                                                //it says we found a color!
                                                fill_backtrace( h, cd, x, y );
                                                return;
                                        }
                                }
                        }
                }
                
                
                if( (y+1) < h.img->header.height )
                {
                        cd = row_dst_next[x];
                        if( cd != cl_handled )
                        {
                                cl_next = row_src_next[x];                        
                                short val = grad();
                        
                                if( val >= -c_ws_max_pix_diff && val < c_ws_crit_pix_diff )
                                {
                                        if( !cd )
                                        {
                                                lookup.emplace_back( x, y+1 );                                
                                                add_backtrace = true;
                                        }
                                        else
                                        {
                                                //it says we found a color!
                                                fill_backtrace( h, cd, x, y );
                                                return;
                                        }
                                }
                        }
                }

                //left
                if( x > 0 )
                {
                        cd = row_dst[x-1];
                        if( cd != cl_handled )
                        {
                                cl_next = row_src[x-1];
                                short val = grad();
                        
                                if( val >= -c_ws_max_pix_diff && val < c_ws_crit_pix_diff )
                                {
                                        if( !cd )
                                        {
                                                lookup.emplace_back( x-1, y );
                                                add_backtrace = true;
                                        }
                                        else
                                        {
                                                //it says we found a color!
                                                fill_backtrace( h, cd, x, y );
                                                return;
                                        }
                                }
                        }
                }
                
                
                if( y > 0 )
                {
                        cd = row_dst_prev[x];
                        if( cd != cl_handled )
                        {
                                cl_next = row_src_prev[x];                        
                                short val = grad();
                        
                                if( val >= -c_ws_max_pix_diff && val < c_ws_crit_pix_diff )
                                {
                                        if( !cd )
                                        {
                                                lookup.emplace_back( x, y-1 );                                
                                                add_backtrace = true;
                                        }
                                        else
                                        {
                                                //it says we found a color!
                                                fill_backtrace( h, cd, x, y );
                                                return;
                                        }
                                }
                        }
                }
                
                
                if( add_backtrace )
                {
                        row_dst[x] = cl_handled;
                        backtrace.emplace_back( x, y );
                }
                
                if( !lookup.empty() )
                {
                     point2i_t & tail = lookup.back();
                     x = tail.x;
                     y = tail.y;
                     lookup.pop_back();
                }
                else break;
                
        }
        
        fill_backtrace( h, ++h.color, x, y );
#undef grad        
}

static void
watershed_outer( helper_t & h )
{
        typedef point2i_t point_t;
        int width = h.bounds.size.x;
        int height = h.bounds.size.y;
                
        for( int y = 0; y < height; ++y )
        {
                cmn::color4b_t const * row_src = reinterpret_cast<cmn::color4b_t const *>( h.img->bytes + h.img->header.pitch * y );
                uint16_t * row_dst = reinterpret_cast<uint16_t *>( h.img_quantized->bytes + h.img_quantized->header.pitch * y );                                
                for( int x = 0; x < width; ++x )
                {
                        if( !row_dst[x] )        //else we have a color already, skip
                        {                        
                                //cmn::color4b_t px_src = row_src[x];
                                pool( h, x, y );                                
                        }                        
                }                                
        }
}

static void
waterched_create_objects( std::vector< watershed_object_t > * objects, helper_t & h )
{
        if( !h.color )
                return;
        
        struct bounding_box_t
        {
                int l, t, r, b;                 
        };
        
        objects->resize( h.color );
        std::vector<bounding_box_t> bbs;
        bbs.resize( h.color );
        memset( &bbs[0], 0, sizeof(bounding_box_t) );
        
        image_header_t & header = h.img_quantized->header;
        for( int y = 0; y < header.height; ++y )
        {
                uint16_t * row = h.img_quantized->row<uint16_t>(y);
                for( int x = 0; x < header.width; ++x )
                {
                        uint16_t c = row[x]; 
                        if( c )
                        {
                                --c;
                                bounding_box_t & b = bbs[c];
                                if( b.l > x ) b.l = x;
                                if( b.r < x ) b.r = x;
                                if( b.t > y ) b.t = y;
                                if( b.b < y ) b.b = y;                        
                        }
                }
        }
        
        for( size_t i = 0; i < bbs.size(); ++i )
        {
                bounding_box_t & b = bbs[i];
                uint16_t color = (uint16_t)i+1;
                watershed_object_t & wo = (*objects)[i];
                wo.x = b.l;
                wo.y = b.t;
                wo.img = cmn::image_create( (b.r-b.l)+1, (b.b-b.t)+1, pitch_default, cmn::format_bw );
                
                for( int y = b.t; y <= b.b; ++y )
                {
                        uint16_t * row = h.img_quantized->row<uint16_t>(y);
                        for( int x = b.l; x <= b.r; ++x )
                        {
                                uint16_t c = row[x];                                                                
                                cmn::image_bw_writepixel( wo.img.get(), x, y, c == color );                                
                        }
                }
        }        
}

static int const gNumColors = 16;
static cmn::color4b_t gColors[gNumColors] =
{
        {255,   0,      0,      255},
        {0,     255,    0,      255},
        {0,     0,      255,    255},
        {255,   255,    0,      255},
        {0,     255,    255,    255},
        {255,   0,      255,    255},
        {255,   128,    128,    255},
        {128,   255,    128,    255},
        {128,   128,    255,    255},
        {64,    0,      0,      255},
        {0,     64,     0,      255},
        {0,     0,      64,     255},
        {64,    64,     0,      255},
        {0,     64,     64,     255},
        {64,    0,      64,     255},
        {0,     0,      0,      255}
};

static void 
waterched_color( cmn::image_pt * colored, helper_t & h )
{
        image_header_t & src_header = h.img_quantized->header;
        *colored = cmn::image_create( src_header.width, src_header.height, cmn::pitch_default, cmn::format_rgba );
        image_header_t & dst_header = (*colored)->header;
        
        for( int y = 0; y < src_header.height; ++y )
        {
                uint16_t * src_row = (uint16_t *)(h.img_quantized->bytes + y * src_header.pitch);
                cmn::color4b_t * dst_row = (cmn::color4b_t *)((*colored)->bytes + y * dst_header.pitch);
                for( int x = 0; x < src_header.width; ++x )
                {
                        uint16_t src_px = src_row[x];
                        int src_idx = src_px & 15;      //15 == gNumColors-1
                        cmn::color4b_t * dst_px = dst_row+x;
                        *dst_px = gColors[src_idx];                                                                       
                }
        }
}


//TODO: opt: this definitely can work faster in times
void 
watershed( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img )
{                        
        typedef point2i_t point_t;
        
        if( !img || img->header.width == 0 || img->header.height == 0 ) //empty image ?
        {
                cmn::log("alg/watershed.cpp: empty image supplied, skipping");
                return;
        }
                
        helper_t h;
        //h.objects = objects;
        h.img = img.get();
        h.img_quantized = image_create( img->header.width, img->header.height, cmn::pitch_default, cmn::format_g16); // img->header.format );   
        h.bounds = rect2i_t(0, 0, img->header.width, img->header.height);                
        memset( h.img_quantized->bytes, 0, h.img_quantized->header.pitch * h.img_quantized->header.height );                                
        
        mark_flat_objects( h );
        //watershed_outer( h ); 
        
        if( objects )
                waterched_create_objects( objects, h );
        
        if(colored)
                waterched_color( colored, h );
}
        
}
