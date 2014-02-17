//this implementation uses libpng and is cross-platform
//but for mac I decided to use native Cocoa API (NSImage based) see src/os/mac/adapter/image.mm
#include <adapter/image.hpp>
#include <cmn/log.hpp>
#include <cmn/image.hpp>

#include <png.h>
#include <vector>
#include <stdio.h>

namespace adapter
{
        
cmn::image_pt image_create_from_png( char const * szImgPath )
{
        cmn::image_root_pt img( new cmn::image_root_t );        
        char unsigned header[8];    // 8 is the maximum size that can be checked
        
        png_structp png_ptr;        
        png_infop   info_ptr;
        
        int width, height;
        png_byte color_type, bit_depth;
        int number_of_passes;
        int pitch, y;        
        uint8_t * row_ptr;                
        std::vector<png_bytep> row_pointers;

        /* open file and test for it being a png */
        FILE *fp = fopen(szImgPath, "rb");
        if (!fp)
        {
                cmn::log("image_create_from_png: file %s could not be opened for reading", szImgPath);
                goto unwind_00;
        }
        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
        {
                cmn::log("image_create_from_png: file %s is not recognized as a PNG file", szImgPath);
                goto unwind_01;
        }

        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
        {
                cmn::log("image_create_from_png: png_create_read_struct failed");
                goto unwind_01;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
                cmn::log("image_create_from_png: png_create_info_struct failed");
                goto unwind_02;
        }

        if (setjmp(png_jmpbuf(png_ptr)))
        {
                cmn::log("image_create_from_png: error during init_io");
                goto unwind_03;
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr, info_ptr);
        bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
                png_set_expand_gray_1_2_4_to_8(png_ptr);
        
        if (bit_depth == 16)
              png_set_scale_16(png_ptr);
        
        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb(png_ptr);
        
        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
                png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        
        number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        /* read file */
        if (setjmp(png_jmpbuf(png_ptr)))
        {
                cmn::log("image_create_from_png: error during read_image");
                goto unwind_03;
        }

        row_pointers.resize(height);
        pitch =  width * 4; //4 - rgba
        img->memory = std::shared_ptr<uint8_t>( new uint8_t[ height * pitch ] );
        row_ptr = img->memory.get();        
        for (y=0; y<height; ++y)
        {
                  row_pointers[y] = row_ptr;
                  row_ptr += pitch;
        }
        png_read_image(png_ptr, &row_pointers[0]);
               
        img->header.width = width;
        img->header.height = height;
        img->header.pitch = pitch;
        img->header.format = cmn::format_rgba;
        img->header.flags = 0;
        img->data.bytes = img->memory.get();
        
unwind_03: png_destroy_info_struct(png_ptr, &info_ptr);        
unwind_02: png_destroy_read_struct(&png_ptr, 0, 0);
unwind_01: fclose(fp);
unwind_00: 
        return img;
}

}