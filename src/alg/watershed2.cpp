#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/utils.hpp>

#include <algorithm>
#include <string.h>

static const int16_t c_grad_tolerance_find = -1;
static const int16_t c_grad_tolerance_flood = 0;
static const int16_t c_grad_sharp_border = 20;          //it's definitely an another object!

namespace alg 
{

using namespace cmn;
        
namespace
{
        
struct lowland_t
{
        point2w_t               center;        
        int16_t                 deepness;       //brightness in center
        uint16_t                color;          //0-based
        std::vector<point2w_t>  border;
        std::vector<point2w_t>  border2;          //back-buffer
};
        
struct helper_t
{
        image_pt                img_src;        //rgba input image
        image_pt                img_color;      //uin16_t image, of format_g16 each pixel - indexed color (see below)
        std::vector<lowland_t>  lowlands;
        std::vector<int>        lowlands_by_deepness;       //sorted, indices in lowlands.
        
        uint16_t                color;          //current        
};

}

static int16_t 
grad( color4b_t const c1, color4b_t const c2 ) 
{
       int16_t v = ((short)c1.r - (short)c2.r)*int16_t(5)  + ((short)c1.g - (short)c2.g)*int16_t(9) + (((short)c1.b - (short)c2.b)<<1);
       return v >> 4;
}

static int16_t
gradt( color4b_t const c1, color4b_t const c2 )
{
        return grad(c1,c2) + c_grad_tolerance_find;
}

static int16_t
gradtl( color4b_t const c1, color4b_t const c2 )
{
        return grad(c1,c2) + c_grad_tolerance_flood;
}


static void 
ll_examine_neighbors(helper_t & h, image_t * img_color, image_t const * img_src, int const x, int const y)
{
        color4b_t range[3][3];
        int16_t   grads[3][3];      
        color4b_t const * row;
        
        row = img_src->row<color4b_t>( y-1 );        
        range[0][0] = row[x-1]; range[0][1] = row[x]; range[0][2] = row[x+1];
        row = img_src->row<color4b_t>( y );
        range[1][0] = row[x-1]; range[1][1] = row[x]; range[1][2] = row[x+1];
        row = img_src->row<color4b_t>( y+1 );
        range[2][0] = row[x-1]; range[2][1] = row[x]; range[2][2] = row[x+1];
        
        color4b_t const middle = range[1][1];
        grads[0][0] = gradt( middle, range[0][0] ); grads[0][1] = gradt( middle, range[0][1] );  grads[0][2] = gradt( middle, range[0][2] );
        grads[1][0] = gradt( middle, range[1][0] ); grads[1][1] = 0;                             grads[1][2] = gradt( middle, range[1][2] );
        grads[2][0] = gradt( middle, range[2][0] ); grads[2][1] = gradt( middle, range[2][1] );  grads[2][2] = gradt( middle, range[2][2] );
        
        uint16_t source_cl = 0;
        
        for( int gy = 0; gy < 3; ++gy )
        {
                int16_t const iy = y+gy-1;
                uint16_t * const row_dst = img_color->row<uint16_t>(iy);
                
                for( int gx = 0; gx < 3; ++gx)
                {
                        int16_t v = grads[gy][gx];
                        if( v < 0 )
                        {       //grads[gy][gx] is higher than grads[x][y] ... abort                                
                                goto exit_loop;
                        }
                        else
                        {
                                //we have to take a look if the neighbor has a color already
                                uint16_t const cl = row_dst[x+gx-1];
                                if(cl)                                 
                                        source_cl = cl;                                                                
                        }
                }
        }
        
        if( source_cl )
        {
                //we should include ourselves in the current lowland
                lowland_t & ll = h.lowlands[source_cl-1];
                ll.border.push_back(point2w_t(x,y));
                img_color->row<uint16_t>(y)[x] = source_cl;            //write it into the current pixel
                goto exit_loop;
        }

        
        //if we are here this means that we are the topmost element in all neihgborhoods!
        {
                uint16_t cl = ++h.color;
                img_color->row<uint16_t>(y)[x] = cl;
                lowland_t ll;
                ll.center = point2w_t(x,y);
                ll.border.push_back(ll.center);
                ll.color = cl;
                ll.deepness = grad( range[1][1], color4b_t(0,0,0,255) );
                h.lowlands.push_back( ll );
        }                
exit_loop:;
        
}


static void 
find_lowlands( helper_t & h )
{
        image_header_t const hdr = h.img_src->header;
        image_t const * img_src = h.img_src.get();
        image_t * img_color = h.img_color.get();
        h.lowlands.reserve(16);         //why not ?
        
        for( int y = 1; y < hdr.height-1; ++y )
        {
                for( int x = 1; x < hdr.width-1; ++x )
                {
                        ll_examine_neighbors( h, img_color, img_src, x, y );
                }
        } 
        
        int n = 0;
        auto inc_range = [&n](){ return n++; };
        h.lowlands_by_deepness.resize( h.lowlands.size() );
        std::generate( h.lowlands_by_deepness.begin(), h.lowlands_by_deepness.end(), inc_range );
                
        std::vector< lowland_t > const & lowlands = h.lowlands;
        auto sort_by_deepness = [&lowlands]( int a, int b )
        {
                return lowlands[a].deepness > lowlands[b].deepness;
        };
                                
        //sort them all
        std::sort( h.lowlands_by_deepness.begin(), h.lowlands_by_deepness.end(), sort_by_deepness );
}

static void
grow_contour_examine_neighbors( helper_t & h, lowland_t & ll, image_t * img_color, image_t const * img_src, point2w_t const pt )
{
        color4b_t               range[3][3];
        int16_t                 grads[3][3];        
        color4b_t const *       row;

        int16_t const lastw = img_src->header.width - 1;
        int16_t const lasth = img_src->header.height - 1;
        
        if( pt.y > 0 )
        {
                row = img_src->row<color4b_t>( pt.y-1 );        
                if(pt.x > 0) range[0][0] = row[pt.x-1]; range[0][1] = row[pt.x]; if(pt.x<lastw) range[0][2] = row[pt.x+1];
        }
        row = img_src->row<color4b_t>( pt.y );
        
        if(pt.x > 0) range[1][0] = row[pt.x-1]; range[1][1] = row[pt.x]; if(pt.x<lastw) range[1][2] = row[pt.x+1];
        
        if( pt.y < lasth )
        {
                row = img_src->row<color4b_t>( pt.y+1 );
                if(pt.x > 0) range[2][0] = row[pt.x-1]; range[2][1] = row[pt.x]; if(pt.x<lastw) range[2][2] = row[pt.x+1];
        }

        color4b_t middle = range[1][1];
        grads[0][0] = gradtl( middle, range[0][0] ); grads[0][1] = gradtl( middle, range[0][1] );  grads[0][2] = gradtl( middle, range[0][2] );
        grads[1][0] = gradtl( middle, range[1][0] ); grads[1][1] = 0;                             grads[1][2] = gradtl( middle, range[1][2] );
        grads[2][0] = gradtl( middle, range[2][0] ); grads[2][1] = gradtl( middle, range[2][1] );  grads[2][2] = gradtl( middle, range[2][2] );
                 
        for( int gy = 0; gy < 3; ++gy )
        {
                int16_t const iy = pt.y + gy - 1;
                if( iy < 0 || iy > lasth )
                        continue;
                
                uint16_t * const row_dst = img_color->row<uint16_t>(iy);
                for( int gx = 0; gx < 3; ++gx)
                {
                        int16_t ix = pt.x + gx - 1;                                                
                        if( ix < 0 || ix > lastw )              //out of scope
                                continue;
                        
                        uint16_t color_dst = row_dst[ix];
                        if( !color_dst )        //free
                        {                        
                                int16_t const val = grads[gy][gx];
                                if(val >= 0 && val < c_grad_sharp_border)            //can grow... 
                                {
                                        ll.border2.emplace_back(ix,iy);     
                                        row_dst[ix] = ll.color;
                                }
                        }
                }
        }
}

static bool
grow_contour( helper_t & h, lowland_t & ll )
{
        std::vector<point2w_t> & border = ll.border;        
        int const border_size = (int)border.size();
        image_t const * img_src = h.img_src.get();
        image_t * img_color = h.img_color.get();
        
        for( int i = 0; i != border_size; ++i )
        {
                point2w_t pt = border[i];
                grow_contour_examine_neighbors( h, ll, img_color, img_src, pt );
        }
                
        border.clear();
        border.swap(ll.border2);        
        return !border.empty();
}

static void 
flood( helper_t & h )
{
        //most heavy part - grow from borders until acquire all image!
        //TODO: just a heaven for parallelization
        bool has_points;
        int lowlands_size = (int)h.lowlands.size();
                        
        do
        {
                has_points = false;
                for( int i = 0; i != lowlands_size; ++i )
                {                
                        lowland_t & ll = h.lowlands[i];                        
                        has_points |= grow_contour(h, ll);
                }                             
        } while( has_points );
}

static void 
flood2( helper_t & h )
{
        bool has_points;
        int lowlands_size = (int)h.lowlands_by_deepness.size();
 
        for( int i = 0; i != lowlands_size; ++i )
        {                
                int index = h.lowlands_by_deepness[i];
                lowland_t & ll = h.lowlands[index];
                do
                {                                
                        has_points = grow_contour(h, ll);
                } while( has_points );
        }
        
}


void watershed2( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img )
{
        helper_t h;
        int width = img->header.width, height = img->header.height;
        
        h.img_src = img;
        h.img_color = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g16 );
        memset( h.img_color->bytes, 0, height * h.img_color->header.pitch );
        h.color = 0;
        
        find_lowlands(h);
        flood2(h);

        if( objects )
                watershed_create_objects( objects, h.color, h.img_color );
        
        if( colored )
                watershed_color( colored, h.img_color );        
}
        
}