#pragma once
#include <cmn/fwd.hpp>

#include <stdint.h>


namespace cmn
{

enum format_et : uint16_t
{
        format_rgba,           //byte for each channel, shows how they lay in memory. in hex, little endian 0xAABBGGRR (native for opengl)
        format_g,              //g here grey hopefully green also will match for most cases :)
        format_g16,            //two bytes
        format_gf,             //float (32)
        format_bw,             //black-white, monchromatic, 1 - white. bitmap. left to right, natural ordering. rows have to be at least 4 bytes aligned
        format_size            //has to be last one
};

struct image_header_t
{
        int             width;
        int             height;
        int             pitch;          //amount of bytes in a row
        format_et       format;                
        uint16_t        flags;
};

enum { pitch_default = -1};

struct px_rgba_t
{
        union
        {
                struct
                {
                           uint8_t r,g,b,a;
                };
                uint8_t    row[4];                
        };        
};

union image_data_t
{
        union
        {
                uint8_t *      bytes;
                uint8_t *      g;       //alias to bytes
                uint16_t *     g16;
                float *        gf;
                px_rgba_t *    rgba;        
        };
};


struct image_plain_t
{
        image_header_t  header;
        image_data_t    data;
};

struct image_t : public image_plain_t
{                     
        virtual ~image_t(){};
};

struct image_root_t : public image_t
{
        std::shared_ptr<uint8_t> memory;
};

struct image_sub_t : public image_t       //nested image
{
        int             x,y;            //offset in parent
        image_pt        parent;         //owner of pixel data either another subimage or rootimage
};

//utility functions
int           format_bits_get(format_et format);

//if pitch == pitch_default it will be taken as width*sizeof(pixel)
void          image_init( image_plain_t * image, int width, int height, int pitch, format_et format );
image_root_pt image_create( int width, int height, int pitch, format_et format );
image_sub_pt  image_sub_create( image_pt const & root, int x, int y, int width, int height );
//image_root_pt image_copy( image_pt const & src );
//image_sub_pt  image_branch( image_pt const & src );          //creates either a sibling-subimage (sharing the same buffer) or if src is rootimage creates subimage equal in size to src


} //of namespace cmn