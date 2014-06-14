#include <cmn/image.hpp>
#include <cmn/utils.hpp>
#include <assert.h>

namespace cmn
{

static int format_bits[]
{        
        32, 8, 16, 32, 1, 32, 32*3
};
static_assert( sizeof(format_bits)/sizeof(int) == format_size, "add value(s) to format_bits array" );

int        
format_bits_get(format_et format)
{
        return format_bits[format];
}

void          
image_init( image_plain_t * image, int width, int height, int pitch, format_et format )
{
        image->header.width = width;
        image->header.height = height;
        image->header.pitch = pitch;
        image->header.format = format;
        image->header.flags = 0;
}

image_root_pt 
image_create( int width, int height, int pitch, format_et format )
{
        //align_up
        if( pitch == pitch_default )
                pitch = align_up<int, 8>( width * format_bits_get(format) ) >> 3;
        
        image_root_pt img( new image_root_t );
        image_init(img.get(), width, height, pitch, format);
        img->memory = std::shared_ptr<uint8_t>( new uint8_t[pitch*height], [](uint8_t * mem){ delete[] mem; } );
        img->bytes = img->memory.get();
        return img;
}

}