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
        format_bw,             //black-white, monochromatic, 1 - white. bitmap. left to right, natural ordering. rows have to be at least 4 bytes aligned
        format_hsva,           //HSV/HSB, last byte - alpha from rgba or unused
        format_lab_f32,        //CIE Lab, each component - 32bit float
        format_size            //has to be last one
};

struct image_header_t
{
        int             width;
        int             height;
        int             pitch;          //number of bytes in a row
        format_et       format;                
        uint16_t        flags;
};

enum { pitch_default = -1};


template< typename T, int D>
struct point_tt;
typedef point_tt<uint8_t, 4> color4b_t;

struct image_plain_t
{
        image_header_t  header;
        uint8_t *       bytes;
        
        template< typename T > T * row( int y ) { return (T*)(header.pitch * y + bytes); }
        template< typename T > T const * row( int y ) const { return (T const*)(header.pitch * y + bytes); }
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

inline bool image_bw_readpixel( cmn::image_plain_t const * const img, int x, int y ) { return !!(img->bytes[ (x>>3) + img->header.pitch * y ] & (1<<(x&7))); }
inline void image_bw_writepixel( cmn::image_plain_t * img, int x, int y, bool value ) 
{
        uint8_t * addr = img->bytes + (x>>3) + img->header.pitch * y;
        if( value ) *addr |= 1 << (x&7); else *addr &= ~(1 << (x&7));        
}
inline bool image_bw_readpixel( uint8_t const * const row, int x ) { return row[x >> 3] & (1 << (x&7)); }
inline void image_bw_writepixel( uint8_t * const row, int x, bool value )
{
        uint8_t * const addr = row + (x >> 3);
        if( value ) *addr |= 1 << (x&7); else *addr &= ~(1 << (x&7));
}


} //of namespace cmn