#pragma once
#include <stdint.h>
#include <memory>

namespace cmn
{

enum format_et : uint16_t
{
        format_rgba,           //byte for each channel, shows how they lay in memory. in hex, little endian 0xAABBGGRR (native for opengl)
        format_g               //g here grey hopefully green also will match for most cases :)
};

struct image_header_t
{
        int             width;
        int             height;
        int             pitch;          //amount of bytes in a row
        format_et       format;                
        uint16_t        flags;
};

enum { pitch_default };

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

typedef std::shared_ptr<image_t> image_pt;

struct image_root_t : public image_t
{
        std::shared_ptr<uint8_t> memory;
};
typedef std::shared_ptr<rootimage_t> image_root_pt;

struct image_sub_t : public image_t       //nested image
{
        image_pt        parent;         //owner of pixel data either another subimage or rootimage
};
typedef std::shared_ptr<image_sub_t> image_sub_pt;

//utility functions

//if pitch == pitch_default it will be taken as width*sizeof(pixel)
void          image_init( plain_image_t * image, int width, int height, int pitch, format_et format );
image_root_pt image_create( int width, int height, int pitch, format_et format );
image_sub_pt  image_sub_create( image_pt const & root, int x, int y, int width, int height );
//image_root_pt image_copy( image_pt const & src );
//image_sub_pt  image_branch( image_pt const & src );          //creates either a sibling-subimage (sharing the same buffer) or if src is rootimage creates subimage equal in size to src


} //of namespace cmn