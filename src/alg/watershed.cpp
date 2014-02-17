#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <string.h>

namespace alg
{
using namespace cmn;

namespace
{

struct helper_t
{
        std::vector< watershed_object_t > * objects;
        cmn::image_t *                      img;
        
        image_pt                img_qunatized;             
        std::vector<point2i_t>  stack;
        std::vector<point2i_t>  lookup;        
        rect2i_t                bounds;
        uint16_t                color = 1;                
};

} //end of anonymous namespace

static void
watershed_analyze( helper_t & h, point2i_t const & pt, point2i_t const & npt )        //npt - next point
{
        typedef point2i_t point_t;                
}

static void 
watershed_inner( helper_t & h )
{
        typedef point2i_t point_t;
        while( !h.stack.empty() )
        {
                point_t pt = h.stack.back();

                {
                        point_t npt = pt; ++npt.x;
                        watershed_analyze( h, pt, npt );
                }
                {
                        point_t npt = pt; --npt.y;
                        watershed_analyze( h, pt, npt );
                }
                {
                        point_t npt = pt; --npt.x;
                        watershed_analyze( h, pt, npt );
                }
                {
                        point_t npt = pt; ++npt.y;
                        watershed_analyze( h, pt, npt );
                }                                
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
                }
                
                watershed_inner(h);
                ++h.color;
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
        h.objects = objects;
        h.img = img.get();
        h.img_qunatized = image_create( img->header.width, img->header.height, cmn::pitch_default, img->header.format );   
        h.bounds = rect2i_t(0, 0, img->header.width, img->header.height);
                
        memset( h.img_qunatized->data.bytes, 0, h.img_qunatized->header.pitch * h.img_qunatized->header.height );        
        h.lookup.push_back( point_t(0,0) );
                
        watershed_outer( h ); 
        
        //if(colored)
        //        waterched_color();
}
        
}