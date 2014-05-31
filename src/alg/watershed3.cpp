#include <alg/watershed.hpp>
#include <cmn/image.hpp>
#include <cmn/rect.hpp>
#include <cmn/log.hpp>
#include <cmn/utils.hpp>
#include <string.h>

namespace alg 
{

using namespace cmn;
        
namespace
{
        
struct helper_t
{
        image_pt                img_src;        //rgba input image
        image_pt                img_color;      //uin16_t image, of format_g16 each pixel - indexed color (see below)        
        uint16_t                color;          //current        
        uint8_t                 level;          //of water
};        


}


void watershed3( std::vector< watershed_object_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img )
{
        helper_t h;
        int width = img->header.width, height = img->header.height;
        
        h.img_src = img;
        h.img_color = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g16 );
        memset( h.img_color->bytes, 0, height * h.img_color->header.pitch );
        h.color = 0;
}


} //alg