#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/utils.hpp>
#include <adapter/image.hpp>    //to save to png file

#include <limits.h>
#include <string>
#include <string.h>

static const int16_t c_grad_tolerance = 3;              

namespace alg 
{

using namespace cmn;
        
namespace
{
        
struct lowland_t
{
        point2w_t               center;        
        //uint16_t                deepness;       //brightness in center
        uint16_t                color;          //0-based
        std::vector<point2w_t>  border;
};
        
struct helper_t
{
        image_pt                img_color;      //uin16_t image, of format_g16 each pixel - indexed color (see below)
        std::vector<lowland_t>  lowlands;
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
        return grad(c1,c2) + c_grad_tolerance;
}

static void 
examine_neighbors(helper_t & h, image_t * img_color, image_t const * img_src, int const x, int const y)
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
        
        color4b_t middle = range[1][1];
        grads[0][0] = gradt( middle, range[0][0] ); grads[0][1] = gradt( middle, range[0][1] );  grads[0][2] = gradt( middle, range[0][2] );
        grads[1][0] = gradt( middle, range[1][0] ); grads[1][1] = 0;                             grads[1][2] = gradt( middle, range[1][2] );
        grads[2][0] = gradt( middle, range[2][0] ); grads[2][1] = gradt( middle, range[2][1] );  grads[2][2] = gradt( middle, range[2][2] );
                        
        for( int gy = 0; gy < 3; ++gy )
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
                                uint16_t cl = img_color->row<uint16_t>(y+gy-1)[x+gx-1];
                                if(cl)
                                {
                                        //we should include ourselves in the current lowland
                                        lowland_t & ll = h.lowlands[cl];
                                        ll.border.push_back(point2w_t(x,y));
                                        img_color->row<uint16_t>(y)[x] = cl;            //write it into the current pixel                                        
                                        goto exit_loop;
                                }                                
                        }
                }
        
        //if we are here this means that we are the topmost element in all neihgborhoods!
        {
                uint16_t cl = ++h.color;
                img_color->row<uint16_t>(y)[x] = cl;
                lowland_t ll;
                ll.center = point2w_t(x,y);
                ll.border.push_back(ll.center);
                ll.color = cl;
                //ll.deepness = 
                h.lowlands.push_back( ll );
        }                
exit_loop:;
        
}


static void 
find_lowlands( helper_t & h, image_pt img )
{
        image_header_t hdr = img->header;
        image_t const * img_src = img.get();
        image_t * img_color = h.img_color.get();
        
        for( int y = 1; y < hdr.height-1; ++y )
        {
                for( int x = 1; x < hdr.width-1; ++x )
                {
                        examine_neighbors( h, img_color, img_src, x, y );
                }
        }
        
        
        //most heavy part - grow from borders until acquire all image!
        
        
}

void watershed2( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img )
{
        
}
        
}