#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/util.hpp>
#include <string.h>

namespace alg
{
using namespace cmn;

namespace
{
        
struct helper_t
{
        //std::vector< watershed_object_t > * objects;
        cmn::image_t *                      img;
        
        image_pt                img_qunatized;             
        std::vector<point2i_t>  stack;
        std::vector<point2i_t>  lookup;
        std::vector<point2i_t>  backtrack;
        rect2i_t                bounds;
        uint16_t                color = 0;   
        uint16_t                found_color = 0;        
};

} //end of anonymous namespace

static bool
watershed_analyze( helper_t & h, point2i_t const & pt, cmn::point3b_t const & cl, point2i_t const & npt )        //npt - next point
{
        typedef point2i_t point_t;
        bool found_color = false;
        if( h.bounds.is_inside(npt) )
        {        
                cmn::point3b_t ncl = *(cmn::point3b_t*)(h.img->data.rgba + ( npt.x + npt.y * h.img->header.pitch ));
                cmn::point3c_t dir = ncl - cl;
                int sign = dir.x + (int)dir.y + dir.z;        
                if( sign >= 0 )
                {       //increases
                        uint16_t color_q = h.img_qunatized->data.g16[ npt.x + npt.y * h.img_qunatized->header.pitch ];
                        if( color_q )
                        {
                                h.found_color = color_q;
                                found_color = true;
                        }                        
                        else
                        {
                                h.backtrack.push_back(pt);                                
                                h.stack.push_back(npt); //remember for walk
                        }
                }
                else
                {       //decreases
                        h.lookup.push_back(pt);
                }
        }        
        return found_color;
}

static bool 
watershed_find_color( helper_t & h )
{
        typedef point2i_t point_t;
        bool found_color = false;
        while( !h.stack.empty() && !found_color )
        {
                point_t pt = h.stack.back();
                h.stack.pop_back();
                cmn::point3b_t cl = *(cmn::point3b_t*)(h.img->data.rgba + (pt.x + pt.y * h.img->header.pitch));                
                
                {
                        point_t npt = pt; ++npt.x;
                        found_color = watershed_analyze( h, pt, cl, npt );
                }
                if(!found_color)
                {
                        point_t npt = pt; --npt.y;
                        found_color = watershed_analyze( h, pt, cl, npt );
                }
                if(!found_color)
                {
                        point_t npt = pt; --npt.x;
                        found_color = watershed_analyze( h, pt, cl, npt );
                }
                if(!found_color)
                {
                        point_t npt = pt; ++npt.y;
                        found_color = watershed_analyze( h, pt, cl, npt );
                }                                
        }
        
        return found_color;
}

static void 
watershed_backtrack( helper_t & h )
{
        typedef point2i_t point_t;
        while( !h.backtrack.empty() )
        {
                point_t pt = h.backtrack.back();
                h.backtrack.pop_back();
                uint16_t * color_q = h.img_qunatized->data.g16 + ( pt.x + pt.y * h.img_qunatized->header.pitch );
                *color_q = h.found_color;
        }
}

static void
watershed_outer( helper_t & h )
{
        typedef point2i_t point_t;
        while( !h.lookup.empty() )
        {
                {
                        point_t pt = h.lookup.back();
                        h.lookup.pop_back();
                        h.stack.push_back(pt);
                        h.backtrack.push_back(pt);
                }
                
                bool found_color = watershed_find_color(h);                
                if( !found_color )
                {
                        ++h.color;
                        h.found_color = h.color;
                }
                else
                {
                        h.stack.clear();
                }
                
                watershed_backtrack(h);
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
        
        image_header_t & header = h.img_qunatized->header;
        for( int y = 0; y < header.height; ++y )
        {
                uint16_t * row = h.img_qunatized->data.g16 + y * header.pitch;
                for( int x = 0; x < header.width; ++x )
                {
                        uint16_t c = row[x] - 1;
                        bounding_box_t & b = bbs[c];
                        if( b.l > x ) b.l = x;
                        if( b.r < x ) b.r = x;
                        if( b.t > y ) b.t = y;
                        if( b.b < y ) b.b = y;                        
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
                        uint16_t * row = h.img_qunatized->data.g16 + y * header.pitch;
                        for( int x = b.l; x <= b.r; ++x )
                        {
                                uint16_t c = row[x] - 1;                                                                
                                cmn::image_bw_writepixel( wo.img.get(), x, y, c == color );                                
                        }
                }
        }        
}

static int const gNumColors = 16;
static cmn::px_rgba_t gColors[gNumColors] =
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
        image_header_t & src_header = h.img_qunatized->header;
        *colored = cmn::image_create( src_header.width, src_header.height, cmn::pitch_default, cmn::format_rgba );
        image_header_t & dst_header = (*colored)->header;
        
        for( int y = 0; y < src_header.height; ++y )
        {
                uint16_t * src_row = h.img_qunatized->data.g16 + y * src_header.pitch;
                cmn::px_rgba_t * dst_row = (*colored)->data.rgba + y * dst_header.pitch;
                for( int x = 0; x < src_header.width; ++x )
                {
                        uint16_t src_px = src_row[x];
                        int src_idx = src_px & 15;      //15 == gNumColors-1
                        cmn::px_rgba_t * dst_px = dst_row+x;
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
        h.img_qunatized = image_create( img->header.width, img->header.height, cmn::pitch_default, cmn::format_g16); // img->header.format );   
        h.bounds = rect2i_t(0, 0, img->header.width, img->header.height);
                
        memset( h.img_qunatized->data.bytes, 0, h.img_qunatized->header.pitch * h.img_qunatized->header.height );        
        h.lookup.push_back( point_t(0,0) );
                
        watershed_outer( h ); 
        
        if(colored)
                waterched_color( colored, h );
}
        
}