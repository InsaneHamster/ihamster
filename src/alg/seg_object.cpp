#include <alg/seg_object.hpp>
#include <cmn/image.hpp>
#include <adapter/image.hpp>
#include <limits.h>

namespace alg
{
using namespace cmn;
        
void
seg_create_objects( std::vector< seg_object_t > * objects, uint16_t max_color, cmn::image_pt img_quantized )
{
        if( !max_color )
                return;
        
        struct bounding_box_t
        {
                int l, t, r, b;                 
        };
        
        objects->resize( max_color );
        std::vector<bounding_box_t> bbs;
        bbs.resize( max_color );
        for( int i = 0; i < max_color; ++i )
        {
                bounding_box_t & b = bbs[i];
                b.l = b.t = INT_MAX;
                b.r = b.b = 0;
        }
        //memset( &bbs[0], 0, sizeof(bounding_box_t) );
        
        image_header_t & header = img_quantized->header;
        for( int y = 0; y < header.height; ++y )
        {
                uint16_t * row = img_quantized->row<uint16_t>(y);
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
                seg_object_t & wo = (*objects)[i];
                wo.lt.x = b.l;
                wo.lt.y = b.t;
                wo.img = cmn::image_create( (b.r-b.l)+1, (b.b-b.t)+1, pitch_default, cmn::format_bw );
                wo.wc = point2f_t(0,0);
                int64_t dots = 0;
                
                for( int y = b.t; y <= b.b; ++y )
                {
                        uint16_t * row = img_quantized->row<uint16_t>(y);
                        for( int x = b.l; x <= b.r; ++x )
                        {
                                int x_local = x - b.l;
                                int y_local = y - b.t;
                                
                                uint16_t c = row[x];                
                                bool belong = c == color;
                                cmn::image_bw_writepixel( wo.img.get(), x_local, y_local, belong );
                                if( belong )
                                        (wo.wc.x += x_local), (wo.wc.y += y_local), ++dots;                                 
                        }
                }
                
                wo.wc.x /= dots;
                wo.wc.y /= dots;
                wo.square = dots;
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


void 
seg_color( cmn::image_pt * colored, cmn::image_pt img_quantized )
{
        image_header_t & src_header = img_quantized->header;
        *colored = cmn::image_create( src_header.width, src_header.height, cmn::pitch_default, cmn::format_rgba );
        image_header_t & dst_header = (*colored)->header;
        
        for( int y = 0; y < src_header.height; ++y )
        {
                uint16_t * src_row = (uint16_t *)(img_quantized->bytes + y * src_header.pitch);
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

        
void seg_object_save_to_png( seg_object_t const * wo, char const * szPath )
{
        cmn::image_t * img_src = wo->img.get();
        cmn::image_header_t const & hs = img_src->header;
        cmn::image_pt  img_dst_p = cmn::image_create( hs.width, hs.height, cmn::pitch_default, cmn::format_rgba );
        cmn::image_t * img_dst = img_dst_p.get();
        cmn::image_header_t const & hd = img_dst->header;
        
        cmn::color4b_t white(255,255,255,255);
        cmn::color4b_t black(0,0,0,255);
        
        for( int y = 0; y < hs.height; ++y )
        {
                cmn::color4b_t * const row_dst = img_dst->row<cmn::color4b_t>(y);
                uint8_t const * const row_src = wo->img->row<uint8_t>(y);
                for( int x = 0; x < hs.width; ++x )
                {
                        bool set = cmn::image_bw_readpixel( row_src, x );
                        row_dst[x] = set ? white : black;
                }
        }
        
        adapter::image_save_to_png( img_dst_p, szPath );
}

void seg_objects_save_to_png( std::vector< seg_object_t > const & objects, std::string const & folder )
{
        char buf[256];
        int size = (int)objects.size();
        for( int i = 0; i < size; ++i )
        {
                sprintf(buf, "/%03d.png", i);
                std::string path = folder + buf;
                seg_object_save_to_png( &objects[i], path.c_str() );
        }
}
        
}