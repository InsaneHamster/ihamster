#include <adapter/image.hpp>
#include <cmn/log.hpp>
#include <png.h>
#include <stdio.h>

namespace adapter
{
        
cmn::image_pt image_create_from_png( char const * szImgPath )
{
        cmn::image_pt img;        
        char unsigned header[8];    // 8 is the maximum size that can be checked
        
        png_structp png_ptr;        
        png_infop   info_ptr;
        
        int width, height;
        png_byte color_type, bit_depth;
        int number_of_passes;

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

        number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);


        /* read file */
        if (setjmp(png_jmpbuf(png_ptr)))
        {
                cmn::log("image_create_from_png: error during read_image");
                goto unwind_03;
        }

//         row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
//         for (y=0; y<height; y++)
//                 row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

//        png_read_image(png_ptr, row_pointers);

        fclose(fp);
        return img;
        
unwind_03: png_destroy_info_struct(png_ptr, &info_ptr);        
unwind_02: png_destroy_read_struct(&png_ptr, 0, 0);
unwind_01: fclose(fp);
unwind_00: return cmn::image_pt();    
}

}